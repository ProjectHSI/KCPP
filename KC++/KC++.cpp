// KC++.cpp : Defines the entry point for the application.
//

#include "WarningManagement.h"
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
#pragma warning( push, 1 )
#pragma warning(disable : 4371 4365 4626 5027 4100 4946 4371 5267 5243)
#include <KC++Save.pb.h>
#pragma warning( pop )
#include "Save.h"
#include <KC++.png.h>

#include <SDL3/SDL_main.h>

#undef max
#undef min

KCPP::Style *KCPP::currentStyle;

SDL_HitTestResult hitTest(SDL_Window *hitTestWindow, const SDL_Point *point, [[maybe_unused]] void *data) {
	SDL_FPoint fPoint {};
	fPoint.x = static_cast < float >(point->x);
	fPoint.y = static_cast < float >(point->y);

	if (KCPP::currentStyle != nullptr) {
		switch (KCPP::currentStyle->hitTest(hitTestWindow, &fPoint)) {
			case KCPP::HitTestResult::Menu:
				[[fallthrough]];
			case KCPP::HitTestResult::StylePassthrough:
				[[fallthrough]];
			case KCPP::HitTestResult::Close:
				return SDL_HITTEST_NORMAL;
			case KCPP::HitTestResult::None:
				[[fallthrough]];
			default:
				return SDL_HITTEST_DRAGGABLE;
		}
	} else {
		return SDL_HITTEST_DRAGGABLE;
	}
}

SDL_Window *mainWindow;
SDL_Surface *kcppIcon;
SDL_Renderer *mainRenderer;

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
	SDL_HideWindow(mainWindow);
	SDL_SetTrayEntryEnabled(trayRecenterWindow, false);
}

bool KCPP::isWindowShown() {
	return localIsWindowShown;
}

std::pair<int, int> KCPP::getWindowPosition(void) {
	int x {};
	int y {};
	SDL_GetWindowPosition(mainWindow, &x, &y);
	return {x, y};
}

void KCPP::setWindowPosition(std::pair<int, int> windowPos) {
	SDL_SetWindowPosition(mainWindow, windowPos.first, windowPos.second);
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

static void iterate() {
	if (autoSaveCounter < SDL_GetTicks() / autoSaveInterval) {
		autoSaveCounter = SDL_GetTicks() / autoSaveInterval;
		KCPP::Save::save();
	}			   

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

	if (KCPP::currentStyle != nullptr) {
		if (KCPP::currentStyle->sizeChangeNeeded(mainWindow)) {
			SDL_SetWindowSize(mainWindow,
							  KCPP::currentStyle->getSize(mainWindow)[0],
							  KCPP::currentStyle->getSize(mainWindow)[1]
			);
			renderNeeded = true;
		}

		if (renderNeeded || KCPP::currentStyle->renderNow()) {
			KCPP::currentStyle->render(mainRenderer, inputCounter, prestigeCounter);
		}

		SDL_RenderPresent(mainRenderer);
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

static bool SDLCALL eventWatch([[maybe_unused]] void *userdata, SDL_Event *event) {
	if (event->type == SDL_EVENT_WINDOW_EXPOSED) {
		iterate();
		::KCPP::Menu::menuIterate();
	}

	return true;
}

static void SDLCALL trayToggleWindowFunc([[maybe_unused]] void *userdata, [[maybe_unused]] SDL_TrayEntry *entry) {
	if (localIsWindowShown) {
		SDL_HideWindow(mainWindow);
		localIsWindowShown = false;
		SDL_SetTrayEntryEnabled(trayRecenterWindow, false);
	} else {
		SDL_ShowWindow(mainWindow);
		localIsWindowShown = true;
		SDL_SetTrayEntryEnabled(trayRecenterWindow, true);
	}
}

static void SDLCALL trayRecenterWindowFunc([[maybe_unused]] void *userdata, [[maybe_unused]] SDL_TrayEntry *entry) {
	SDL_SetWindowPosition(mainWindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
}

bool continueRunning = true;

static void SDLCALL trayQuitFunc([[maybe_unused]] void *userdata, [[maybe_unused]] SDL_TrayEntry *entry) {
	continueRunning = false;
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
	GOOGLE_PROTOBUF_VERIFY_VERSION;

#if defined(_WIN32) && defined(NDEBUG)
	if (!FreeConsole())
		std::cout << "FREECONSOLE " << GetLastError() << '\n';
#endif

	KCPP::currentStyle = KCPP::Styles::availableStyles->begin()->second.get();
	KCPP::UsernameFinder::refreshCachedUserNames();

	SDL_SetHint(SDL_HINT_VIDEO_ALLOW_SCREENSAVER, "1");

	SDL_Init(SDL_INIT_VIDEO);

	KCPP::InputChecker::init();

	// window creation
	mainWindow = SDL_CreateWindow("KC++", 1, 1, SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_TRANSPARENT | SDL_WINDOW_UTILITY | SDL_WINDOW_BORDERLESS);

	if (!mainWindow) {
		std::cout << "STOP! window not init; " << SDL_GetError() << '\n';
		std::terminate();
	}

	kcppIcon = SDL_LoadPNG_IO(SDL_IOFromConstMem(kcppPng.data(), kcppPng.size()), true);
	SDL_SetWindowIcon(mainWindow, kcppIcon);

	// renderer creation
	mainRenderer = SDL_CreateRenderer(mainWindow, nullptr);

	if (!mainRenderer) {
		std::cout << "STOP! renderer not init; " << SDL_GetError() << '\n';
		std::terminate();
	}

	// tray icon creation
	/*
		KC++
		| - Toggle Window
		| - Recenter Window
		|
		| - Quit
	*/
	tray = SDL_CreateTray(kcppIcon, "KC++");
	trayMenu = SDL_CreateTrayMenu(tray);
	trayToggleWindow = SDL_InsertTrayEntryAt(trayMenu, -1, "Toggle Window", SDL_TRAYENTRY_BUTTON);
	SDL_SetTrayEntryCallback(trayToggleWindow, trayToggleWindowFunc, nullptr);
	trayRecenterWindow = SDL_InsertTrayEntryAt(trayMenu, -1, "Recenter Window", SDL_TRAYENTRY_BUTTON);
	SDL_SetTrayEntryCallback(trayRecenterWindow, trayRecenterWindowFunc, nullptr);
	traySeparator = SDL_InsertTrayEntryAt(trayMenu, -1, nullptr, SDL_TRAYENTRY_DISABLED);
	trayQuit = SDL_InsertTrayEntryAt(trayMenu, -1, "Quit", SDL_TRAYENTRY_BUTTON);
	SDL_SetTrayEntryCallback(trayQuit, trayQuitFunc, nullptr);

	// vsync
	if (KCPP::EnableVSync) {
		if (!SDL_SetRenderVSync(mainRenderer, SDL_RENDERER_VSYNC_ADAPTIVE)) {
			std::cout << "No adaptive vsync support\n";
			if (!SDL_SetRenderVSync(mainRenderer, 1)) {
				std::cout << "No vsync support\n";
			}
		}
	} else {
		SDL_SetRenderVSync(mainRenderer, SDL_RENDERER_VSYNC_DISABLED);
	}

	KCPP::Menu::menuInit();

	// sensible default if not set by load
	SDL_SetWindowPosition(mainWindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

	KCPP::Save::load();

	KCPP::currentStyle->init(mainRenderer);
	KCPP::currentStyle->resetRenderer(mainRenderer);

	// window exposed event watch
	SDL_SetWindowHitTest(mainWindow, hitTest, nullptr);
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
				if (event.window.windowID == SDL_GetWindowID(mainWindow)) {
					continueRunning = false;
				} else {
					toggleMenu();
				}
			} else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
				if (KCPP::currentStyle != nullptr) {
					SDL_FPoint eventCoordinatesAsPoint {event.button.x, event.button.y};

					switch (KCPP::currentStyle->hitTest(mainWindow, &eventCoordinatesAsPoint)) {
					case KCPP::HitTestResult::Menu:
						toggleMenu();
						break;
					case KCPP::HitTestResult::StylePassthrough:
						KCPP::currentStyle->processEvent(event);
						break;
					case KCPP::HitTestResult::Close:
						continueRunning = false;
						[[fallthrough]];
					case KCPP::HitTestResult::None:
						[[fallthrough]];
					default:
						break;
					}
				}
			}
		}

		if (continueRunning) {
			KCPP::Menu::menuIterate();
			iterate();
		}
	}

	KCPP::InputChecker::quit();

	KCPP::Save::save();

	KCPP::Menu::menuQuit();

	SDL_DestroyTray(tray);

	SDL_DestroyRenderer(mainRenderer);
	SDL_DestroySurface(kcppIcon);
	SDL_DestroyWindow(mainWindow);

	SDL_Quit();

	google::protobuf::ShutdownProtobufLibrary();

	return 0;
}
