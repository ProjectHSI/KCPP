#pragma once

#include "WarningManagement.h"
#include "Style.h"
#include "KCCommon.h"
#include <string>

#include <deque>
#include "LCDFont.h"
#include "UsernameFinder.h"
#pragma warning( push, 1 )
#pragma warning(disable : 4371 4365 4626 5027 4100 4946 4371 5267 5243)
#include <LCDStyleSave.pb.h>
#pragma warning( pop )

namespace KCPP {
	namespace LCDStyle {
		struct Touch {
			float x {};
			float y {};
			std::size_t timeStamp {};
		};

		enum class TextAlignment {
			Left,
			Right
		};

		class LCDStyle : public KCPP::Style {
		private:
			/*std::array < float, 4 > backgroundColour {{
				0.0f, 0.0f, 0.0f, 0.1f
			}};

			std::array < float, 4 > inactiveColour {{
				0.0f, 0.0f, 0.0f, 0.2f
			}};

			std::array < float, 3 > activeColour {{
				0.0f, 1.0f, 0.0f
			}};

			std::array < float, 3 > prestigeActiveColour {{
				1.0f, 1.0f, 0.0f
			}};*/

			SDL_Surface *lcdTextureSurface = nullptr;
			SDL_Texture *lcdTexture = nullptr;
			//SDL_Texture *lcdTextureInvert = nullptr;

			void renderGlyph(const KCPP::LCDStyle::Font::Glyph &glyph, int xOffset, int yOffset, KCPP::CounterType counter, KCPP::PrestigeType prestige, std::array < float, 3 > activeColour, std::array < float, 4 > inactiveColour);
			void renderGlyphWithChar(char character, int xOffset, int yOffset, KCPP::CounterType counter, KCPP::PrestigeType prestige, std::array < float, 3 > activeColour, std::array < float, 4 > inactiveColour);

			void renderGlyphs(std::string string, int xOffset, int yOffset, KCPP::CounterType counter, KCPP::PrestigeType prestige, std::array < float, 3 > activeColour, std::array < float, 4 > inactiveColour, std::string::size_type wantedRequiredLength = 0, TextAlignment textAlignment = TextAlignment::Right);

			struct TickerAnimation {
				std::string text {};
				std::size_t startTime {};
			};

			std::deque < TickerAnimation > queuedTickerAnimations {};
			std::deque < Touch > currentTouches {};

			bool wasSettingsInitalised = false;
			KCPP::LCDStyle::LCDStyleSave save {};

			/*enum class UserNameConfiguration {
				TechnicalName,
				DisplayName,
				UserChosen
			};
			UserNameConfiguration userNameConfiguration = UserNameConfiguration::TechnicalName;
			std::string userName {};

			::KCPP::LCDStyle::LCDElementAlignment  lcdGrabBarAlignment {};
			::KCPP::LCDStyle::LCDElementAttachment lcdGrabBarAttachment {};
			::KCPP::LCDStyle::LCDElementAlignment  lcdPrestigeAlignment {};
			::KCPP::LCDStyle::LCDElementAttachment lcdPrestigeAttachment {};*/

			std::array < int, 2 > getLcdTextureSize() const;

			bool editJustPerformed = false;

			bool renderedAnimationLastFrame = false;
			float lastContentScale = 0.0f;

			void addIntroTickerAnimation();

		public:
			virtual std::string_view getInternalName() override;
			virtual std::string_view getDisplayName() override;


			// Events
			virtual void prestige() override;



			LCDStyle();

			virtual KCPP::HitTestResult hitTest(SDL_Window *window, const SDL_FPoint *cursorPos);
			virtual bool processEvent(SDL_Event &event);

			virtual bool renderNow();

			virtual void init(SDL_Renderer *renderer) override;
			virtual void resetRenderer(SDL_Renderer *renderer = nullptr) override;

			virtual bool sizeChangeNeeded(SDL_Window *window) override;
			virtual std::array < int, 2 > getSize(SDL_Window *window) const override;

			virtual void render(SDL_Renderer *renderer, KCPP::CounterType count, KCPP::PrestigeType prestige) override;

			virtual void quit(SDL_Renderer *renderer) override;


			// Saves
			virtual void imGuiSettingsMenu();
			virtual void parseSettings(const std::string &data);
			virtual std::string generateSettings();


			virtual ~LCDStyle() = default;
		};
	}
}