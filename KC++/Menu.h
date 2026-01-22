#include <SDL3/SDL.h>

namespace KCPP {
	namespace Menu {
		void menuInit();
		void openMenu();
		bool menuEvent(SDL_Event &event);
		void menuIterate();
		void closeMenu();
		void menuQuit();
	}
}