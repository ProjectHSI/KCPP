// KC++.cpp : Defines the entry point for the application.
//

#include "KC++.h"
#include <SDL3/SDL.h>
#include <numbers>
#include "Style.h"
#include "InputChecker.h"
#include "LCDStyle.h"
#include "Menu.h"

#ifdef _WIN32
#include <Windows.h>
#endif
#include "Styles.h"
#include <KC++Save.pb.h>
#include "Save.h"
#include <KC++.png.h>

#undef max
#undef min

KCPP::Style *KCPP::currentStyle;

SDL_HitTestResult hitTest(SDL_Window *window, const SDL_Point *point, void *data) {
	if (KCPP::currentStyle != nullptr) {
		switch (KCPP::currentStyle->hitTest(window, point)) {
			case KCPP::HitTestResult::Menu:
				return SDL_HITTEST_NORMAL;
			case KCPP::HitTestResult::StylePassthrough:
				return SDL_HITTEST_NORMAL;
			case KCPP::HitTestResult::None:
			default:
				return SDL_HITTEST_DRAGGABLE;
		}
	} else {
		return SDL_HITTEST_DRAGGABLE;
	}
}

SDL_Window *window;
SDL_Surface *kcppIcon;
SDL_Renderer *renderer;

SDL_Tray *tray;
SDL_TrayMenu *trayMenu;
SDL_TrayEntry *trayToggleWindow;
SDL_TrayEntry *trayRecenterWindow;
SDL_TrayEntry *traySeparator;
SDL_TrayEntry *trayQuit;

::std::size_t autoSaveCounter {};
::std::size_t autoSaveInterval = 60 * 1000; // 1 minute

KCPP::CounterType inputCounter = 0;
bool renderNeeded = true;

bool localIsWindowShown = true;

void KCPP::hideWindow() {
	localIsWindowShown = false;
	SDL_HideWindow(window);
	SDL_SetTrayEntryEnabled(trayRecenterWindow, false);
}

bool KCPP::isWindowShown() {
	return localIsWindowShown;
}

std::pair<int, int> KCPP::getWindowPosition(void) {
	int x {};
	int y {};
	SDL_GetWindowPosition(window, &x, &y);
	return {x, y};
}

void KCPP::setWindowPosition(std::pair<int, int> windowPos) {
	SDL_SetWindowPosition(window, windowPos.first, windowPos.second);
}

void KCPP::setCounter(KCPP::CounterType newCounter) {
	inputCounter = newCounter;
	renderNeeded = true;
}

KCPP::CounterType KCPP::getCounter() {
	return inputCounter;
}


KCPP::PrestigeType prestigeCounter = 0;

void KCPP::setPrestige(KCPP::PrestigeType newPrestige) {
	prestigeCounter = newPrestige;
	renderNeeded = true;
}

KCPP::PrestigeType KCPP::getPrestige() {
	return prestigeCounter;
}

void iterate(bool fromMainLoop) {
	//std::cout << "iter" << '\n';

	if (fromMainLoop)
		KCPP::Menu::menuIterate();

	if (autoSaveCounter < SDL_GetTicks() / autoSaveInterval) {
		autoSaveCounter = SDL_GetTicks() / autoSaveInterval;
		KCPP::Save::save();
	}

	//inputCounter += 0.000001;
	//inputCounter += 0.001;					   
	

	if (KCPP::InputChecker::checkInput()) {
		KCPP::CounterType newCount = KCPP::InputChecker::newInputCount();
		KCPP::InputChecker::clear();

		if (inputCounter <= KCPP::calculateMaximumCounterAllowingForPrecision() - newCount) {
			inputCounter += newCount;
		} else {
			inputCounter = KCPP::calculateMaximumCounterAllowingForPrecision();
		}

		if (prestigeCounter != std::numeric_limits < KCPP::PrestigeType >::max() && inputCounter >= KCPP::getNextPrestigePoint(prestigeCounter)) {
			inputCounter -= KCPP::getNextPrestigePoint(prestigeCounter);
			prestigeCounter++;
			if (KCPP::currentStyle != nullptr)
				KCPP::currentStyle->prestige();
		}

		renderNeeded = true;
	}

	//inputCounter = inputCounter >= KCPP::calculateMaximumCounterAllowingForPrecision()
		//? KCPP::calculateMaximumCounterAllowingForPrecision()
		//: inputCounter;

	if (KCPP::currentStyle != nullptr) {
		if (KCPP::currentStyle->sizeChangeNeeded(window)) {
			SDL_SetWindowSize(window,
							  KCPP::currentStyle->getSize(window)[0],
							  KCPP::currentStyle->getSize(window)[1]
			);
			renderNeeded = true;
		}

		if (renderNeeded || KCPP::currentStyle->renderNow()) {
			KCPP::currentStyle->render(renderer, inputCounter, prestigeCounter);
			//std::cout << inputCounter << '\n';
		}

		SDL_RenderPresent(renderer);
		renderNeeded = false;
	}
}

bool menuOpen = false;

static void toggleMenu() {
	if (menuOpen) {
		menuOpen = false;
		KCPP::Menu::closeMenu();
	} else {
		menuOpen = true;
		KCPP::Menu::openMenu();
	}
}

static bool SDLCALL eventWatch(void *userdata, SDL_Event *event) {
	if (event->type == SDL_EVENT_WINDOW_EXPOSED || (menuOpen && event->type == SDL_EVENT_WINDOW_MOVED)) {
		iterate(true);
		//std::cout << "event watch iter" << '\n';
	} else {
		//std::cout << event->type << '\n';
	}

	return true;
}

static void SDLCALL trayToggleWindowFunc(void *userdata, SDL_TrayEntry *entry) {
	if (localIsWindowShown) {
		SDL_HideWindow(window);
		localIsWindowShown = false;
		SDL_SetTrayEntryEnabled(trayRecenterWindow, false);
	} else {
		SDL_ShowWindow(window);
		localIsWindowShown = true;
		SDL_SetTrayEntryEnabled(trayRecenterWindow, true);
	}
}

static void SDLCALL trayRecenterWindowFunc(void *userdata, SDL_TrayEntry *entry) {
	SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
}

bool continueRunning = true;

static void SDLCALL trayQuitFunc(void *userdata, SDL_TrayEntry *entry) {
	continueRunning = false;
}

int main() {
	GOOGLE_PROTOBUF_VERIFY_VERSION;

#if defined(_WIN32) && defined(NDEBUG)
	if (!FreeConsole())
		std::cout << "FREECONSOLE " << GetLastError() << '\n';
#endif

	//std::cout << KCPP::calculateGlyphsNeededForMaximumPrestigeCounter() << '\n';

	KCPP::currentStyle = KCPP::Styles::availableStyles->begin()->second.get();

	KCPP::UsernameFinder::refreshCachedUserNames();

	SDL_Init(SDL_INIT_VIDEO);

	SDL_SetHint(SDL_HINT_VIDEO_ALLOW_SCREENSAVER, "1");

	KCPP::InputChecker::init();

	window = SDL_CreateWindow("KC++", 1, 1, SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_TRANSPARENT | SDL_WINDOW_UTILITY | SDL_WINDOW_BORDERLESS);

	if (!window) {
		std::cout << "STOP! window not init; " << SDL_GetError() << '\n';
		std::terminate();
	}

	kcppIcon = SDL_LoadPNG_IO(SDL_IOFromConstMem(kcppPng.data(), kcppPng.size()), true);

	SDL_SetWindowIcon(window, kcppIcon);

	renderer = SDL_CreateRenderer(window, nullptr);

	if (!renderer) {
		std::cout << "STOP! renderer not init; " << SDL_GetError() << '\n';
		std::terminate();
	}



	tray = SDL_CreateTray(kcppIcon, "KC++");
	trayMenu = SDL_CreateTrayMenu(tray);
	trayToggleWindow = SDL_InsertTrayEntryAt(trayMenu, -1, "Toggle Window", SDL_TRAYENTRY_BUTTON);
	SDL_SetTrayEntryCallback(trayToggleWindow, trayToggleWindowFunc, nullptr);
	trayRecenterWindow = SDL_InsertTrayEntryAt(trayMenu, -1, "Recenter Window", SDL_TRAYENTRY_BUTTON);
	SDL_SetTrayEntryCallback(trayRecenterWindow, trayRecenterWindowFunc, nullptr);
	traySeparator = SDL_InsertTrayEntryAt(trayMenu, -1, nullptr, SDL_TRAYENTRY_DISABLED);
	trayQuit = SDL_InsertTrayEntryAt(trayMenu, -1, "Quit", SDL_TRAYENTRY_BUTTON);
	SDL_SetTrayEntryCallback(trayQuit, trayQuitFunc, nullptr);



	if (!SDL_SetRenderVSync(renderer, 2)) {
		std::cout << "No adaptive vsync support\n";
		if (!SDL_SetRenderVSync(renderer, 1)) {
			std::cout << "No vsync support\n";
		}
	}

	KCPP::Menu::menuInit();

	KCPP::Save::load();

	//inputCounter = KCPP::getNextPrestigePoint(prestigeCounter) - 10000000;

	KCPP::currentStyle->init(renderer);
	KCPP::currentStyle->resetRenderer(renderer);

	SDL_SetWindowHitTest(window, hitTest, nullptr);
	SDL_AddEventWatch(eventWatch, nullptr);

	while (continueRunning) {
		SDL_Event event {};
		while (SDL_PollEvent(&event)) {
			if (!KCPP::Menu::menuEvent(event)) {
				continue;
			}

			if (event.type == SDL_EVENT_QUIT) {
				continueRunning = false;
			} else if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
				if (event.window.windowID == SDL_GetWindowID(window)) {
					continueRunning = false;
				} else {
					toggleMenu();
				}
			} else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
				if (KCPP::currentStyle != nullptr) {
					SDL_Point eventCoordinatesAsPoint {event.button.x, event.button.y};

					if (KCPP::currentStyle->hitTest(window, &eventCoordinatesAsPoint) == KCPP::HitTestResult::Menu) {
						toggleMenu();
					} else if (KCPP::currentStyle->hitTest(window, &eventCoordinatesAsPoint) == KCPP::HitTestResult::StylePassthrough) {
						KCPP::currentStyle->processEvent(event);
					}
				}
			} else {
				//std::cout << event.type << '\n';
			}
		}

		iterate(true);
	}

	//SDL_Renderer *renderer = SDL_CreateRenderer();

	KCPP::InputChecker::quit();

	KCPP::Save::save();

	KCPP::Menu::menuQuit();

	SDL_DestroyTray(tray);

	SDL_DestroyRenderer(renderer);
	SDL_DestroySurface(kcppIcon);
	SDL_DestroyWindow(window);

	SDL_Quit();

	google::protobuf::ShutdownProtobufLibrary();

	return 0;
}
