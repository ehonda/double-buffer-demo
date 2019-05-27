#pragma once

#include <list>
#include <memory>

#include "boost/geometry/geometries/point_xy.hpp"
#include "SDL.h"

#include "constants.h"

namespace scene {

using Velocity = boost::geometry::model::d2::point_xy<double>;

const Velocity NORTH = { 0, -1 };
const Velocity NORTH_EAST = { 1, -1 };
const Velocity EAST = { 1, 0 };
const Velocity SOUTH_EAST = { 1, 1 };
const Velocity SOUTH = { 0, 1 };
const Velocity SOUTH_WEST = { -1, 1 };
const Velocity WEST = { -1, 0 };
const Velocity NORTH_WEST = { -1, -1 };
const Velocity STILL = { 0, 0 };

struct MovingRectangle {
	double mass;
	boost::geometry::model::d2::point_xy<double> center;
	int width, height;
	Velocity v;
};

MovingRectangle createRect(const SDL_Rect& rect, const Velocity& v);
bool rectanglesCollide(const MovingRectangle& a, const MovingRectangle& b);
SDL_Rect convertToSDLRect(const MovingRectangle& r);
MovingRectangle move(MovingRectangle r);

void log(const MovingRectangle& r);

bool operator==(const MovingRectangle& lhs, 
	const MovingRectangle& rhs);
bool operator!=(const MovingRectangle& lhs,
	const MovingRectangle& rhs);

Velocity getVelocityAfterCollision(const MovingRectangle& a, const MovingRectangle& b);


// Scene is controller and view combined
class Scene {
public:
	Scene();

	void setRenderer(const std::shared_ptr<SDL_Renderer>& renderer) noexcept;

	void update();
	void render();
	
private:
	int width_ = constants::DEFAULT_WINDOW_WIDTH;
	int height_ = constants::DEFAULT_WINDOW_HEIGHT;

	void swapRectBuffers() noexcept;

	// Size of border rects is:
	// w: 2 * B_X + width_
	// h: 2 * B_X + height_
	const int B_X = (2 << 28);
	const SDL_Rect LEFT_BORDER = {
		.x = -(2 * B_X + width_),
		.y = -B_X,
		.w = 2 * B_X + width_,
		.h = 2 * B_X + height_ };
	const SDL_Rect UPPER_BORDER = { 
		.x = -B_X,
		.y = -(2 * B_X + height_), 
		.w = 2 * B_X + width_,
		.h = 2 * B_X + height_ };
	const SDL_Rect RIGHT_BORDER = {
		.x = width_,
		.y = -B_X,
		.w = 2 * B_X + width_,
		.h = 2 * B_X + height_ };
	const SDL_Rect LOWER_BORDER = {
		.x = -B_X,
		.y = height_,
		.w = 2 * B_X + width_,
		.h = 2 * B_X + height_ };
	std::list<MovingRectangle> borderRectangles_
		= {
			createRect(LEFT_BORDER, STILL),
			createRect(UPPER_BORDER, STILL),
			createRect(RIGHT_BORDER, STILL),
			createRect(LOWER_BORDER, STILL)
	};

	using RectList = std::list<MovingRectangle>;
	RectList rectBuffers_[2];
	RectList *current_, *next_;
	
	std::shared_ptr<SDL_Renderer> renderer_;
};

}
