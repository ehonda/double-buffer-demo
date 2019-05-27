#include "application.h"

#include "constants.h"
#include "sdl2_utils.h"

namespace app {

void Application::run() {
	initialize();
	while (running_) {
		handleEvents();
		update();
		render();
		// Speed control
		SDL_Delay(10);
	}
	cleanup();
}

void Application::initialize() {
	running_ = true;

	const auto error = [this](const std::string_view msg) {
		sdl2_utils::logError(msg);
		running_ = false;
	};

	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		error("SDL_Init");
		return;
	}

	window_ = std::shared_ptr<SDL_Window>(
		SDL_CreateWindow(
			"App",
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			constants::DEFAULT_WINDOW_WIDTH,
			constants::DEFAULT_WINDOW_HEIGHT,
			SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE),
		sdl2_utils::getWindowDeleter());
	if (!window_) {
		error("SDL_CreateWindow");
		return;
	}

	renderer_ = std::shared_ptr<SDL_Renderer>(
		SDL_CreateRenderer(
			window_.get(),
			-1,
			SDL_RENDERER_ACCELERATED),
		sdl2_utils::getRendererDeleter());
	if (!renderer_) {
		error("SDL_CreateRenderer");
		return;
	}

	scene_.setRenderer(renderer_);
}

void Application::handleEvents() {
	SDL_Event e;
	while (SDL_PollEvent(&e)) {
		switch (e.type) {
		case SDL_QUIT:
			running_ = false;
			break;

		default:
			break;
		}
	}
}

void Application::update() {
	scene_.update();
}

void Application::render() {
	// Clear
	SDL_SetRenderDrawColor(renderer_.get(), 255, 255, 255, 0);
	SDL_RenderClear(renderer_.get());

	scene_.render();

	SDL_RenderPresent(renderer_.get());
}

void Application::cleanup() {
	renderer_.reset();
	window_.reset();
}

}
