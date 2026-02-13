#pragma once

#include <cstddef>
#include <SDL3/SDL.h>
#include <array>
#include <string>
#include "KCCommon.h"

namespace KCPP {
	enum class HitTestResult {
		Menu,
		StylePassthrough,
		//StylePassthroughAndDraggable,
		Close,
		None
	};
	class Style {
	public:
		virtual std::string_view getInternalName() = 0;
		virtual std::string_view getDisplayName() = 0;


		// Events
		virtual void prestige() = 0;



		virtual HitTestResult hitTest(SDL_Window *window, const SDL_FPoint *cursorPos) = 0;
		virtual bool processEvent([[maybe_unused]] SDL_Event &event) { return true; };

		virtual bool renderNow() = 0;

		virtual void init(SDL_Renderer *renderer) = 0;
		virtual void resetRenderer(SDL_Renderer *renderer) = 0;

		virtual bool sizeChangeNeeded(SDL_Window *window) = 0;
		virtual std::array < int, 2 > getSize(SDL_Window *window) const = 0;

		virtual void render(SDL_Renderer *renderer, KCPP::CounterType count, KCPP::PrestigeType prestige) = 0;

		virtual void quit(SDL_Renderer *renderer) = 0;


		// Saves
		virtual void imGuiSettingsMenu() = 0;
		virtual void parseSettings(const std::string &data) = 0;
		virtual std::string generateSettings() = 0;


		virtual ~Style() = default;
	};
}