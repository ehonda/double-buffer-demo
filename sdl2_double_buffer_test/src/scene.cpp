#include "scene.h"

#include <cmath>

#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>

#include "boost/geometry/arithmetic/arithmetic.hpp"
#include "boost/geometry/arithmetic/dot_product.hpp"

namespace scene {

Velocity operator*(int scalar, const Velocity& v) {
	return { scalar * v.x(), scalar * v.y() };
}

MovingRectangle createRect(const SDL_Rect& rect, const Velocity& v) {
	MovingRectangle r;
	r.mass = static_cast<double>(rect.w) * rect.h;
	r.center.x(rect.x + static_cast<double>(rect.w) / 2);
	r.center.y(rect.y + static_cast<double>(rect.h) / 2);
	r.width = rect.w;
	r.height = rect.h;
	r.v = v;
	return r;
}

bool rectanglesCollide(const MovingRectangle& a, const MovingRectangle& b) {
	SDL_Rect result;
	return SDL_IntersectRect(&convertToSDLRect(a), 
		&convertToSDLRect(b), &result);
}

SDL_Rect convertToSDLRect(const MovingRectangle& r) {
	SDL_Rect s;
	const auto half_w = r.width / 2.0;
	const auto half_h = r.height / 2.0;
	s.x = static_cast<int>(r.center.x() - half_w);
	s.y = static_cast<int>(r.center.y() - half_h);
	s.w = r.width;
	s.h = r.height;
	return s;
}

MovingRectangle move(MovingRectangle r) {
	r.center.x(r.center.x() + r.v.x());
	r.center.y(r.center.y() + r.v.y());
	return r;
}

void log(const MovingRectangle& r) {
	const auto rect = convertToSDLRect(r);
	std::cout << "Pos = { " << rect.x
		<< ", " << rect.y << " },\t\tVel = "
		<< "{ " << r.v.x() << ", " << r.v.y() << " }\n";
}

bool operator==(const MovingRectangle& lhs, 
	const MovingRectangle& rhs)
{
	return SDL_RectEquals(&convertToSDLRect(lhs), &convertToSDLRect(rhs));
}

bool operator!=(const MovingRectangle& lhs, 
	const MovingRectangle& rhs)
{
	return !operator==(lhs, rhs);
}

Velocity getVelocityAfterCollision(const MovingRectangle& a, const MovingRectangle& b) {
	// See https://en.wikipedia.org/wiki/Elastic_collision#Two-dimensional_collision_with_two_moving_objects
	using namespace boost::geometry;
	
	auto v = a.v;
	subtract_point(v, b.v);
	auto x = a.center;
	subtract_point(x, b.center);

	const auto dot = dot_product(v, x);
	const auto sqNorm = x.x() * x.x() + x.y() * x.y();
	const auto m = 2 * b.mass / (a.mass + b.mass);
	const auto factor = m * dot / sqNorm;

	multiply_value(x, factor);

	auto ret = a.v;
	subtract_point(ret, x);

	return ret;
}


namespace {

std::list<MovingRectangle> makeRandomRectangles(std::size_t n,
	int width, int height)
{
	const auto seed = static_cast<unsigned>(
		std::chrono::system_clock::now().time_since_epoch().count());
	std::mt19937 gen(seed);
	std::uniform_int_distribution x_dist(0, width - 100);
	std::uniform_int_distribution y_dist(0, height - 100);
	std::uniform_int_distribution size_dist(10, 100);
	std::uniform_int_distribution v_dist(-5, 5);

	std::list<MovingRectangle> rects;
	while (rects.size() < n) {
		// Generate new random rect
		SDL_Rect rect;
		Velocity v;
		rect.x = x_dist(gen);
		rect.y = y_dist(gen);
		rect.w = size_dist(gen);
		rect.h = size_dist(gen);
		v.x(v_dist(gen));
		v.y(v_dist(gen));

		const auto r = createRect(rect, v);

		// Insert if it doesn't intersect with any in the list
		SDL_Rect result;
		if (std::none_of(std::cbegin(rects), std::cend(rects),
			[&r, &result](const auto& rect) {
				return SDL_IntersectRect(&convertToSDLRect(r), &convertToSDLRect(rect), 
					&result); }))
		{
			rects.push_back(r);
		}
	}
	return rects;
}

}

Scene::Scene()
	: current_{&rectBuffers_[0]}, next_{&rectBuffers_[1]}
{
	*current_ = makeRandomRectangles(30, width_, height_);
	/**current_ = {
		createRect({100, 350, 100, 100}, EAST),
		createRect({900, 350, 100, 100}, WEST)
	};*/
	*next_ = *current_;
}

void Scene::setRenderer(
	const std::shared_ptr<SDL_Renderer>& renderer) noexcept 
{
	renderer_ = renderer;
}

void Scene::update() {
	using namespace std;
	// ----------------------------------------------------------------
	// DOUBLE BUFFER UPDATES
	// ----------------------------------------------------------------

	transform(cbegin(*current_), cend(*current_), begin(*next_),
		[this](const auto& rect) {
			const auto oneStep = move(rect);
			auto next = rect;
			// Find collision with other rect
			for (const auto& other : *current_)
				if (rect != other && rectanglesCollide(oneStep, other)) 
					next.v = getVelocityAfterCollision(rect, other);
			// Find collision with border
			for (const auto& border : borderRectangles_)
				if (rectanglesCollide(oneStep, border))
					next.v = getVelocityAfterCollision(rect, border);
			//log(rect);
			return move(next);
		});
	//std::cout << "\n";
	swapRectBuffers();

	// ----------------------------------------------------------------
	// SEQUENTIAL UPDATES
	// ----------------------------------------------------------------
	
	//for (auto& rect : *current_) {
	//	const auto oneStep = move(rect);
	//	// Find collision with other rect
	//	for (const auto& other : *current_) {
	//		if (rect != other && rectanglesCollide(oneStep, other))
	//			rect.v = getVelocityAfterCollision(rect, other);
	//	}
	//	// Find collision with border
	//	for (const auto& border : borderRectangles_) {
	//		if (rectanglesCollide(oneStep, border))
	//			rect.v = getVelocityAfterCollision(rect, border);
	//	}
	//	rect = move(rect);
	//	//log(rect);
	//}
	////std::cout << "\n";
}

void Scene::render() {
	// BORDERS
	SDL_SetRenderDrawColor(renderer_.get(), 255, 0, 0, 0);
	for (const auto& rect : borderRectangles_)
		SDL_RenderFillRect(renderer_.get(), &convertToSDLRect(rect));

	// RECTANGLES
	SDL_SetRenderDrawColor(renderer_.get(), 0, 0, 0, 0);
	for (const auto& rect : *current_)
		SDL_RenderFillRect(renderer_.get(), &convertToSDLRect(rect));
}

void Scene::swapRectBuffers() noexcept {
	auto tmp = current_;
	current_ = next_;
	next_ = tmp;
}

}
