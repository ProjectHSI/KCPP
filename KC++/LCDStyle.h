#pragma once

#include "Style.h"
#include "KCCommon.h"
#include <string>

#include <deque>
#include "LCDFont.h"
#include "UsernameFinder.h"

namespace KCPP {
	namespace LCDStyle {
		struct Touch {
			float x {};
			float y {};
			std::size_t timeStamp {};
		};

		class LCDStyle : public KCPP::Style {
		private:
			SDL_Texture *lcdTexture = nullptr;
			SDL_Texture *lcdTextureInvert = nullptr;

			void renderGlyph(SDL_Renderer *renderer, SDL_Texture *texture, const KCPP::LCDStyle::Font::Glyph &glyph, int xOffset, int yOffset);
			void renderGlyphWithChar(SDL_Renderer *renderer, SDL_Texture *texture, char character, int xOffset, int yOffset);

			struct TickerAnimation {
				std::string text {};
				std::size_t startTime {};
			};

			std::deque < TickerAnimation > queuedTickerAnimations {};
			std::deque < Touch > currentTouches {};

			std::array < float, 4 > backgroundColour {
				0.0f, 0.0f, 0.0f, 0.1f
			};

			std::array < float, 4 > inactiveColour {
				0.0f, 0.0f, 0.0f, 0.2f
			};
																		  
			std::array < float, 3 > activeColour {
				0.0f, 1.0f, 0.0f
			};


			enum class UserNameConfiguration {
				TechnicalName,
				DisplayName,
				UserChosen
			};
			UserNameConfiguration userNameConfiguration = UserNameConfiguration::TechnicalName;
			std::string userName {};

			bool editJustPerformed = false;

			bool renderedAnimationLastFrame = false;
			float lastContentScale = 0.0f;

			void addIntroTickerAnimation();

		public:
			virtual std::string_view getInternalName() override;



			LCDStyle();

			virtual KCPP::HitTestResult hitTest(SDL_Window *window, const SDL_Point *cursorPos);
			virtual bool processEvent(SDL_Event &event);

			virtual bool renderNow();

			virtual void init(SDL_Renderer *renderer) override;
			virtual void resetRenderer(SDL_Renderer *renderer) override;

			virtual bool sizeChangeNeeded(SDL_Window *window) override;
			virtual std::array < int, 2 > getSize(SDL_Window *window) const override;

			virtual void render(SDL_Renderer *renderer, KCPP::CounterType count) override;

			virtual void quit(SDL_Renderer *renderer) override;


			// Saves
			virtual void imGuiSettingsMenu();
			virtual void parseSettings(const std::string &data);
			virtual std::string generateSettings();


			virtual ~LCDStyle() = default;
		};
	}
}