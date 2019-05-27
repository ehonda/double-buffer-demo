#pragma once

#include <memory>

#include "SDL.h"

#include "scene.h"

namespace app {

class Application {
public:
	void run();

private:
	void initialize();

	void handleEvents();
	void update();
	void render();

	void cleanup();

	std::shared_ptr<SDL_Window> window_;
	std::shared_ptr<SDL_Renderer> renderer_;

	scene::Scene scene_;

	bool running_ = false;
};

}
