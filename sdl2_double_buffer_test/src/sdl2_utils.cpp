#include "sdl2_utils.h"

namespace sdl2_utils {

std::function<void(SDL_Renderer*)> getRendererDeleter() {
	return [](auto* r) { SDL_DestroyRenderer(r); };
}

std::function<void(SDL_Window*)> getWindowDeleter() {
	return [](auto* w) { SDL_DestroyWindow(w); };
}

void logError(std::string_view causedBy, std::ostream& str) {
	str << "Error in " << causedBy << ": "
		<< SDL_GetError() << "\n";
}

}
