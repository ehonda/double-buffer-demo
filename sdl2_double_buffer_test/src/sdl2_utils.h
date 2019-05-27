#pragma once

#include <functional>
#include <iostream>
#include <string_view>

#include "SDL.h"

namespace sdl2_utils {

// For use with the smart pointers
std::function<void(SDL_Renderer*)> getRendererDeleter();
std::function<void(SDL_Window*)> getWindowDeleter();

// TODO: Implement logging service locator etc
void logError(std::string_view causedBy,
	std::ostream& str = std::cout);

}
