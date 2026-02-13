#include "WarningManagement.h"
#include "LCDStyle.h"
#include "LCDFont.h"
#include <cmath>
#include <string>
#include <format>
#include "KCCommon.h"
#include <iostream>
#include <cassert>
#include <optional>
#include <filesystem>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include "Styles.h"
#pragma warning( push, 1 )
#pragma warning(disable : 4371 4365 4626 5027 4100 4946 4371 5267 5243)
#include <LCDStyleSave.pb.h>
#pragma warning( pop )
#include "KC++.h"

constexpr std::size_t tickerTimer = 50; // ms per character
constexpr std::size_t touchDuration = 2000; // ms

constexpr ::std::size_t PixelSize = 3;

const char *regName = "lcdStyle";

static bool lcdStyleReg = KCPP::Styles::addStyle(regName, dynamic_cast < KCPP::Style * >(new KCPP::LCDStyle::LCDStyle()));

std::string_view KCPP::LCDStyle::LCDStyle::getInternalName() {
	return regName;
}

std::string_view KCPP::LCDStyle::LCDStyle::getDisplayName() {
	return "LCD-Style";
}


void KCPP::LCDStyle::LCDStyle::prestige() {
	queuedTickerAnimations.push_back({std::format("PRESTIGE!!! PRESTIGE LEVEL {}, KEEP GOING!", KCPP::calculatePrestigeString(KCPP::getPrestige())), std::numeric_limits < decltype (TickerAnimation::startTime) >::max()});
}




KCPP::LCDStyle::LCDStyle::LCDStyle() : KCPP::Style() {
	save.mutable_active_colour()->set_r(0.0f);
	save.mutable_active_colour()->set_g(1.0f);
	save.mutable_active_colour()->set_b(0.0f);
	
	save.mutable_prestige_active_colour()->set_r(1.0f);
	save.mutable_prestige_active_colour()->set_g(1.0f);
	save.mutable_prestige_active_colour()->set_b(0.0f);

	save.mutable_inactive_colour()->set_r(0.0f);
	save.mutable_inactive_colour()->set_g(0.0f);
	save.mutable_inactive_colour()->set_b(0.0f);
	save.mutable_inactive_colour()->set_a(0.2f);
	
	save.mutable_bg_colour()->set_r(0.0f);
	save.mutable_bg_colour()->set_g(0.0f);
	save.mutable_bg_colour()->set_b(0.0f);
	save.mutable_bg_colour()->set_a(0.1f);

	save.set_user_name_policy(LCDUserNameType::UserName);
}

KCPP::HitTestResult KCPP::LCDStyle::LCDStyle::hitTest(SDL_Window *hitTestWindow, const SDL_FPoint *cursorPos) {
	const auto windowSize = getSize(hitTestWindow);
	switch (save.lcd_grab_bar_alignment()) {
		case ::KCPP::LCDStyle::LCDElementAlignment::LEFT:
			if (cursorPos->x <= (KCPP::LCDStyle::Font::width + 1) * lastContentScale)
				return KCPP::HitTestResult::None;
			else if (cursorPos->x <= (KCPP::LCDStyle::Font::width * 2 + 2) * lastContentScale)
				return KCPP::HitTestResult::Menu;
			break;
		case ::KCPP::LCDStyle::LCDElementAlignment::RIGHT:
			if (cursorPos->x >= static_cast < float >(getLcdTextureSize()[0] - (KCPP::LCDStyle::Font::width + 1)) * lastContentScale)
				return KCPP::HitTestResult::None;
			else if (cursorPos->x >= static_cast < float >(getLcdTextureSize()[0] - (KCPP::LCDStyle::Font::width + 1) * 2) * lastContentScale)
				return KCPP::HitTestResult::Menu;
			break;
		case ::KCPP::LCDStyle::LCDElementAlignment::LCDElementAlignment_INT_MIN_SENTINEL_DO_NOT_USE_:
			[[fallthrough]];
		case ::KCPP::LCDStyle::LCDElementAlignment::LCDElementAlignment_INT_MAX_SENTINEL_DO_NOT_USE_:
			std::terminate();
			break;
	}
	if (save.fast_quit() && cursorPos->x >= static_cast < const float >(windowSize[0] - PixelSize) && cursorPos->y <= static_cast < const float >(PixelSize))
		return KCPP::HitTestResult::Close;
	return KCPP::HitTestResult::StylePassthrough;
}

bool KCPP::LCDStyle::LCDStyle::processEvent(SDL_Event &event) {
	switch (event.type) {
		case SDL_EVENT_MOUSE_BUTTON_DOWN: 
			{
				currentTouches.push_back(Touch {
					event.button.x / lastContentScale,
					event.button.y / lastContentScale,
					SDL_GetTicks()
										});
				break;
			}
		default:
			break;
	}

	return true;
}

bool KCPP::LCDStyle::LCDStyle::renderNow() {
	bool editJustPerformedLocal = editJustPerformed;
	editJustPerformed = false;

	bool renderingAnimations = !queuedTickerAnimations.empty() || !currentTouches.empty();
	if (!renderingAnimations && renderedAnimationLastFrame) {
		renderedAnimationLastFrame = false;
		return true;
	} else if (renderingAnimations && !renderedAnimationLastFrame) {
		renderedAnimationLastFrame = true;
		return true;
	}
	return renderingAnimations || editJustPerformedLocal || (KCPP::getCounter() > KCPP::getNextPrestigePoint(KCPP::getPrestige()) / 2);
}

void KCPP::LCDStyle::LCDStyle::init([[maybe_unused]] SDL_Renderer *renderer) {
	/*switch (userNameConfiguration) {
		case UserNameConfiguration::DisplayName:
			if (KCPP::UsernameFinder::displayName.has_value()) {
				userName = KCPP::UsernameFinder::displayName.value();
				break;
			}
		case UserNameConfiguration::TechnicalName:
			if (KCPP::UsernameFinder::technicalName.has_value()) {
				userName = KCPP::UsernameFinder::technicalName.value();
				break;
			}
		case UserNameConfiguration::UserChosen:
			break;
	}*/
	addIntroTickerAnimation();
}

void KCPP::LCDStyle::LCDStyle::addIntroTickerAnimation(void) {
	std::string userName {};
	if (save.has_user_name_policy()) {
		switch (save.user_name_policy()) {
			case ::KCPP::LCDStyle::LCDUserNameType::DisplayName:
				if (::KCPP::UsernameFinder::displayName.has_value()) {
					userName = ::KCPP::UsernameFinder::displayName.value();
					break;
				}
			case ::KCPP::LCDStyle::LCDUserNameType::UserName:
			default:
				if (::KCPP::UsernameFinder::technicalName) {
					userName = ::KCPP::UsernameFinder::technicalName.value();
					break;
				}
		}
	} else if (save.has_user_name_text()) {
		userName = save.user_name_text();
	}
	if (!userName.empty()) {
		queuedTickerAnimations.push_back(TickerAnimation {std::format("WELCOME, {}!      ENJOY YOUR DAY!", userName), std::numeric_limits < decltype (TickerAnimation::startTime) >::max()});
	} else {
		queuedTickerAnimations.push_back(TickerAnimation {"WELCOME!      ENJOY YOUR DAY!", std::numeric_limits < decltype (TickerAnimation::startTime) >::max()});
	}
}

static constexpr std::size_t getGlyphsNeeded() {
	//return 50;
	return KCPP::calculateGlyphsNeededForMaximumCounter() + KCPP::calculateGlyphsNeededForMaximumPrestigeCounter() + 2 - 1/* for menu */;
}

static constexpr std::size_t glyphsNeeded = getGlyphsNeeded();

static constexpr std::array < int, 2 > getLcdTextureSizeBase() {
	constexpr int textureHeight = KCPP::LCDStyle::Font::height + 2;
	constexpr int textureWidth =
		KCPP::LCDStyle::Font::width * (glyphsNeeded + 1) /* glyph space */
		+ (1 * ((glyphsNeeded + 1) - 1)) /* glyph padding */
		+ 2 /* whole area padding */;

	return {{textureWidth, textureHeight}};
}

std::array<int, 2> KCPP::LCDStyle::LCDStyle::getLcdTextureSize() const {
	constexpr std::array < int, 2 > textureSize = getLcdTextureSizeBase();

	std::array < int, 2 > variableTextureSize = textureSize;

	if (save.lcd_grab_bar_attachment() == LCDElementAttachment::DETACHED)
		variableTextureSize[0] += 2;
	if (save.lcd_prestige_counter_attachment() == LCDElementAttachment::DETACHED)
		variableTextureSize[0] += 2;

	return variableTextureSize;
}

void KCPP::LCDStyle::LCDStyle::resetRenderer(SDL_Renderer *renderer) {
	if (renderer == nullptr) {
		if (lcdTexture != nullptr)
			renderer = SDL_GetRendererFromTexture(lcdTexture);
		else
			// No LCD Texture yet recreating textures from edit
			throw std::exception();
	}
	if (lcdTexture != nullptr) {
		SDL_DestroyTexture(lcdTexture);
		lcdTexture = nullptr;
	}
	lcdTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA128_FLOAT, SDL_TEXTUREACCESS_STREAMING, getLcdTextureSize()[0], getLcdTextureSize()[1]);
	SDL_SetTextureScaleMode(lcdTexture, SDL_SCALEMODE_PIXELART);
}

static inline constexpr ::std::size_t getPixelIndex(SDL_Surface *surface, const int x, const int y) {
	return (x * 4) + (y * (surface->pitch / sizeof(float)));
}

enum class PixelComponent {
	R = 0,
	G = 1,
	B = 2,
	A = 3
};

static inline float &pixel(SDL_Surface *surface, const int x, const int y, const PixelComponent component) {
	return reinterpret_cast < float * >(surface->pixels)[getPixelIndex(surface, x, y) + static_cast < ::std::size_t >(component)];
}

void KCPP::LCDStyle::LCDStyle::renderGlyph(const KCPP::LCDStyle::Font::Glyph &glyph, int xOffset, int yOffset, KCPP::CounterType counter, KCPP::PrestigeType prestige, std::array < float, 3 > currentActiveColour, std::array < float, 4 > currentInactiveColour ) {
	for (int y = 0; y < static_cast < int >(KCPP::LCDStyle::Font::height); ++y) {
		for (int x = 0; x < static_cast < int >(KCPP::LCDStyle::Font::width); ++x) {
			if (glyph[static_cast < ::std::size_t >(y)][static_cast < ::std::size_t >(x)] || KCPP::randomPrestigeProgressEvent(counter, prestige, 0.1)) {
				std::memcpy(&reinterpret_cast < float * >(lcdTextureSurface->pixels)[getPixelIndex(lcdTextureSurface, x + xOffset, y + yOffset)], currentActiveColour.data(), sizeof(currentActiveColour));
				pixel(lcdTextureSurface, x + xOffset, y + yOffset, PixelComponent::A) = 1;
			} else {
				std::memcpy(&reinterpret_cast < float * >(lcdTextureSurface->pixels)[getPixelIndex(lcdTextureSurface, x + xOffset, y + yOffset)], currentInactiveColour.data(), sizeof(currentInactiveColour));
			}
		}
	}
}

void KCPP::LCDStyle::LCDStyle::renderGlyphWithChar(char character, int xOffset, int yOffset, KCPP::CounterType counter, KCPP::PrestigeType prestige, std::array < float, 3 > currentActiveColour, std::array < float, 4 > currentInactiveColour) {
	renderGlyph(KCPP::LCDStyle::Font::glyphs.contains(character)
				? KCPP::LCDStyle::Font::glyphs[character]
				: KCPP::LCDStyle::Font::unknownCharacter,
				xOffset,
				yOffset,
				counter,
				prestige,
				currentActiveColour,
				currentInactiveColour);
}

void KCPP::LCDStyle::LCDStyle::renderGlyphs(std::string string, int xOffset, int yOffset, KCPP::CounterType count, KCPP::PrestigeType prestige, std::array < float, 3 > currentActiveColour, std::array < float, 4 > currentInactiveColour, std::string::size_type wantedRequiredLength, KCPP::LCDStyle::TextAlignment textAlignment) {
	std::string::size_type stringLength = string.size() <= wantedRequiredLength ? wantedRequiredLength : string.size();
	std::string::size_type stringReadOffset = string.size() <= wantedRequiredLength ? wantedRequiredLength - string.size() : 0;

	if (textAlignment == TextAlignment::Left)
		stringReadOffset = 0;

	for (std::string::size_type i = 0; i < stringLength; i++) {
		renderGlyphWithChar(i - stringReadOffset < string.size() ? string[i - stringReadOffset] : ' ',
							static_cast < int >(i * KCPP::LCDStyle::Font::width) /* glyph size */
							+ static_cast < int >(i)                             /* glyph padding */
							+ xOffset,
							yOffset,
							count,
							prestige,
							currentActiveColour,
							currentInactiveColour
		);
	}
}

std::array < float, 3 > getContigousLcdColour3(const ::KCPP::LCDStyle::LCDColour3 & colour) {
	return {{colour.r(), colour.g(), colour.b()}};
}

std::array < float, 4 > getContigousLcdColour4(const ::KCPP::LCDStyle::LCDColour4 &colour) {
	return {{colour.r(), colour.g(), colour.b(), colour.a()}};
}

void KCPP::LCDStyle::LCDStyle::render(SDL_Renderer *renderer, KCPP::CounterType count, KCPP::PrestigeType prestige) {
	SDL_SetRenderDrawColorFloat(renderer, 0, 0, 0, 0);
	SDL_RenderClear(renderer);

	SDL_LockTextureToSurface(lcdTexture, nullptr, &lcdTextureSurface);

	int grabBarDetachPoint = 0;
	int prestigeDetachPoint = 0;
	//constexpr std::size_t glyphsNeededForPrestigeCounter = ::KCPP::calculateGlyphsNeededForMaximumPrestigeCounter();

	int grabBarOffset = 1;
	int prestigeOffset = 1;
	int counterOffset = 1;



	if (save.lcd_grab_bar_alignment() == ::KCPP::LCDStyle::LCDElementAlignment::LEFT) {
		prestigeOffset += static_cast < int >((::KCPP::LCDStyle::Font::width + 1) * 2);
		if (save.lcd_grab_bar_attachment() == ::KCPP::LCDStyle::LCDElementAttachment::DETACHED) {
			grabBarDetachPoint = prestigeOffset;
			prestigeOffset += 2;
		}
		counterOffset = prestigeOffset;
	}

	if (save.lcd_prestige_counter_alignment() == ::KCPP::LCDStyle::LCDElementAlignment::LEFT) {
		constexpr int offsetFromCounter = static_cast < int >((::KCPP::LCDStyle::Font::width + 1) * ::KCPP::calculateGlyphsNeededForMaximumPrestigeCounter());
		counterOffset += offsetFromCounter;
		if (save.lcd_prestige_counter_attachment() == ::KCPP::LCDStyle::LCDElementAttachment::DETACHED) {
			//if (lcdGrabBarAlignment == ::KCPP::LCDStyle::LCDElementAlignment::LEFT)
			prestigeDetachPoint = counterOffset;
			counterOffset += 2;
		}
	}

	if (save.lcd_prestige_counter_alignment() == ::KCPP::LCDStyle::LCDElementAlignment::RIGHT) {
		constexpr int offsetFromCounter = static_cast < int >((::KCPP::LCDStyle::Font::width + 1) * ::KCPP::calculateGlyphsNeededForMaximumCounter());
		prestigeOffset += offsetFromCounter;
		if (save.lcd_prestige_counter_attachment() == ::KCPP::LCDStyle::LCDElementAttachment::DETACHED) {
			prestigeDetachPoint = prestigeOffset;
			prestigeOffset += 2;
		}
	}

	if (save.lcd_grab_bar_alignment() == ::KCPP::LCDStyle::LCDElementAlignment::RIGHT) {
		if (counterOffset > prestigeOffset) {
			constexpr int offsetFromCounter = static_cast < int >((::KCPP::LCDStyle::Font::width + 1) * (::KCPP::calculateGlyphsNeededForMaximumCounter() - 1));
			grabBarOffset = counterOffset + offsetFromCounter;
		} else {
			constexpr int offsetFromCounter = static_cast < int >((::KCPP::LCDStyle::Font::width + 1) * (::KCPP::calculateGlyphsNeededForMaximumPrestigeCounter() - 1));
			grabBarOffset = prestigeOffset + offsetFromCounter;
		}
		grabBarOffset += (::KCPP::LCDStyle::Font::width + 1);
		if (save.lcd_grab_bar_attachment() == ::KCPP::LCDStyle::LCDElementAttachment::DETACHED) {
			grabBarDetachPoint = grabBarOffset;
			grabBarOffset += 2;
		}
	}

	grabBarDetachPoint = grabBarDetachPoint ? grabBarDetachPoint : -1;
	prestigeDetachPoint = prestigeDetachPoint ? prestigeDetachPoint : -1;

	const auto backgroundColour = getContigousLcdColour4(save.bg_colour());
	const auto inactiveColour = getContigousLcdColour4(save.inactive_colour());
	const auto activeColour = getContigousLcdColour3(save.active_colour());
	const auto prestigeActiveColour = getContigousLcdColour3(save.prestige_active_colour());

	for (int x = 0; x < getLcdTextureSize()[0]; x++) {
		bool fillColumnWithTransparent = x == grabBarDetachPoint || x == prestigeDetachPoint;

		for (int y = 0; y < getLcdTextureSize()[1]; y++) {
			if (!fillColumnWithTransparent)
				std::memcpy(&reinterpret_cast < float * >(lcdTextureSurface->pixels)[getPixelIndex(lcdTextureSurface, x, y)], backgroundColour.data(), sizeof(backgroundColour));
			else {
				pixel(lcdTextureSurface, x, y, PixelComponent::R) = 0;
				pixel(lcdTextureSurface, x, y, PixelComponent::G) = 0;
				pixel(lcdTextureSurface, x, y, PixelComponent::B) = 0;
				pixel(lcdTextureSurface, x, y, PixelComponent::A) = 0;
			}
		}
	}

	renderGlyph(save.lcd_grab_bar_alignment() == ::KCPP::LCDStyle::LCDElementAlignment::LEFT ? KCPP::LCDStyle::Font::dragCharacter : KCPP::LCDStyle::Font::menuCharacter, grabBarOffset, 1, count, prestige, activeColour, inactiveColour);
	renderGlyph(save.lcd_grab_bar_alignment() == ::KCPP::LCDStyle::LCDElementAlignment::LEFT ? KCPP::LCDStyle::Font::menuCharacter : KCPP::LCDStyle::Font::dragCharacter, grabBarOffset + static_cast < int >(KCPP::LCDStyle::Font::width) + 1, 1, count, prestige, activeColour, inactiveColour);

	std::string counterString = KCPP::calculateCounterString(count);
	std::string prestigeString = KCPP::calculatePrestigeString(prestige);

	if (!queuedTickerAnimations.empty()) {
		if (queuedTickerAnimations.front().startTime == std::numeric_limits < decltype (TickerAnimation::startTime) >::max()) {
			queuedTickerAnimations.front().startTime = SDL_GetTicks();
		}

		std::size_t phase = (SDL_GetTicks() - queuedTickerAnimations.front().startTime) / tickerTimer;

		prestigeString = "";
		counterString = "";

		if (phase >= queuedTickerAnimations.front().text.length() + ::KCPP::calculateGlyphsNeededForMaximumCounter() + 1) {
			queuedTickerAnimations.pop_front();
		} else {
			constexpr ::std::size_t counterGlyphs = ::KCPP::calculateGlyphsNeededForMaximumCounter();
			signed long long lcdStringPosition = static_cast< signed long long >(counterGlyphs) - static_cast< signed long long >(phase);

			std::string_view viewableString = queuedTickerAnimations.front().text;

			counterString.resize(::KCPP::calculateGlyphsNeededForMaximumCounter() + viewableString.size(), ' ');

			auto outputIterator = counterString.begin();

			if (lcdStringPosition < 0)
			#pragma warning(push)
			#pragma warning(disable : 4365, justification : "\"-\" will never actually result in a negative number, so we can safely use it in an unsigned integer.")
				viewableString = viewableString.substr(-lcdStringPosition, counterGlyphs);
			#pragma warning(pop)
			else {
				outputIterator += lcdStringPosition;
			}

			std::copy(viewableString.cbegin(), viewableString.cend(), outputIterator);
			
			counterString = counterString.substr(0, counterGlyphs);
		}
	}

	constexpr auto counterMaxLength = calculateGlyphsNeededForMaximumCounter();
	constexpr auto prestigeCounterMaxLength = calculateGlyphsNeededForMaximumPrestigeCounter();

	::KCPP::LCDStyle::TextAlignment counterTextAlignment = save.lcd_prestige_counter_alignment() == LCDElementAlignment::LEFT ? TextAlignment::Right : TextAlignment::Left;
	::KCPP::LCDStyle::TextAlignment prestigeTextAlignment = save.lcd_prestige_counter_alignment() == LCDElementAlignment::LEFT ? TextAlignment::Left : TextAlignment::Right;

	renderGlyphs(counterString, counterOffset, 1, count, prestige, activeColour, inactiveColour, counterMaxLength, counterTextAlignment);
	renderGlyphs(prestigeString, prestigeOffset, 1, count, prestige, prestigeActiveColour, inactiveColour, prestigeCounterMaxLength, prestigeTextAlignment);

	const ::std::size_t currentTouchTick = SDL_GetTicks();

	while (!currentTouches.empty()) {
		if (currentTouches.front().timeStamp + touchDuration < currentTouchTick) {
			currentTouches.pop_front();
		} else {
			break;
		}
	}

	if (!currentTouches.empty()) {
		for (auto touchI = currentTouches.crbegin(); touchI != currentTouches.crend(); ++touchI) {
			const auto &touch = *touchI;
			constexpr size_t touchRadius = 16;

			double timeMult = (static_cast < double >(touch.timeStamp + touchDuration - currentTouchTick) / static_cast < double >(touchDuration));

			for (int y = 0; y < getLcdTextureSize()[1]; y++) {
				for (int x = 0; x < getLcdTextureSize()[0]; x++) {
					double effectivePixelX = static_cast < double >(x) + 0.5;
					double effectivePixelY = static_cast < double >(y) + 0.5;
					double dx = fabs(effectivePixelX - touch.x);
					double dy = fabs(effectivePixelY - touch.y);
					double distance = sqrt(dx * dx + dy * dy);
					float pixelAmount = static_cast < float >(1 - (distance / touchRadius / timeMult));

					if (distance <= touchRadius && pixelAmount > 0) {
						pixel(lcdTextureSurface, x, y, PixelComponent::R) *= (1 - pixelAmount);
						pixel(lcdTextureSurface, x, y, PixelComponent::R) += activeColour[static_cast < ::std::size_t >(PixelComponent::R)] * pixelAmount;

						pixel(lcdTextureSurface, x, y, PixelComponent::G) *= (1 - pixelAmount);
						pixel(lcdTextureSurface, x, y, PixelComponent::G) += activeColour[static_cast < ::std::size_t >(PixelComponent::G)] * pixelAmount;

						pixel(lcdTextureSurface, x, y, PixelComponent::B) *= (1 - pixelAmount);
						pixel(lcdTextureSurface, x, y, PixelComponent::B) += activeColour[static_cast < ::std::size_t >(PixelComponent::B)] * pixelAmount;

						pixel(lcdTextureSurface, x, y, PixelComponent::A) *= (1 - pixelAmount);
						pixel(lcdTextureSurface, x, y, PixelComponent::A) += pixelAmount;
					}
				}
			}
		}

		/*		for (const auto &touch : currentTouches) {
					constexpr int touchRadius = 3;
					for (std::size_t y = 0; y < getLcdTextureSize()[1]; y++) {
						std::size_t yRowStart = y * pitch / sizeof(float);
						for (std::size_t x = 0; x < getLcdTextureSize()[0]; x++) {
							std::size_t pixelIndex = (yRowStart + x * 4);
							float effectivePixelX = x + 0.5;
							float effectivePixelY = y + 0.5;
							int dx = effectivePixelX - touch.x;
							int dy = effectivePixelY - touch.y;
							double distance = dx * dx + dy * dy;
							if (distance <= touchRadius) {
								pixels[pixelIndex + 0] = 0.0f; // R
								pixels[pixelIndex + 1] = 0.0f; // G
								pixels[pixelIndex + 2] = 0.0f; // B
								pixels[pixelIndex + 3] = 1.0f; // A
							} else {
								//pixels[pixelIndex + 0] = 1.0f; // R
								//pixels[pixelIndex + 1] = 1.0f; // G
								//pixels[pixelIndex + 2] = 1.0f; // B
								//pixels[pixelIndex + 3] = 1.0f; // A
							}
						}
					}
				}*/
	}

	SDL_UnlockTexture(lcdTexture);

	SDL_SetRenderTarget(renderer, nullptr);
	if (!SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND))
		std::terminate();
	if (!SDL_SetTextureBlendMode(lcdTexture, SDL_BLENDMODE_BLEND))
		std::terminate();
	SDL_RenderTexture(renderer, lcdTexture, nullptr, nullptr);

	if (save.fast_quit()) {
		SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);

		const auto windowSize = getSize(SDL_GetRenderWindow(renderer));
		const auto windowScale = SDL_GetWindowDisplayScale(SDL_GetRenderWindow(renderer));

		std::array < SDL_FPoint, 4 > closePoints {{
			{ static_cast < float >(windowSize[0] - 1),                                                             0 },
			{ static_cast < float >(windowSize[0] - 1 - (PixelSize - 1)) * windowScale, (PixelSize - 1) * windowScale },
			{ static_cast < float >(windowSize[0] - 1 - (PixelSize - 1)) * windowScale, 0                             },
			{ static_cast < float >(windowSize[0] - 1),                                 (PixelSize - 1) * windowScale }
		}};

		SDL_RenderLine(renderer, closePoints[0].x, closePoints[0].y, closePoints[1].x, closePoints[1].y);
		SDL_RenderLine(renderer, closePoints[2].x, closePoints[2].y, closePoints[3].x, closePoints[3].y);
	}

	//SDL_RenderLines(renderer, closePoints.data(), 4);
}



bool KCPP::LCDStyle::LCDStyle::sizeChangeNeeded(SDL_Window *window) {
	if (lastContentScale != SDL_GetWindowDisplayScale(window) * PixelSize || editJustPerformed) {
		lastContentScale = SDL_GetWindowDisplayScale(window) * PixelSize;
		return true;
	} else {
		return false;
	}
}

std::array<int, 2> KCPP::LCDStyle::LCDStyle::getSize(SDL_Window *window) const {
	float contentScale = SDL_GetWindowDisplayScale(window) * PixelSize;
	return std::array<int, 2> {{static_cast < int >(ceil(static_cast < float >(getLcdTextureSize()[0]) *contentScale)), static_cast < int >(ceil(static_cast < float >(getLcdTextureSize()[1]) *contentScale))}};
}

void KCPP::LCDStyle::LCDStyle::quit([[maybe_unused]] SDL_Renderer *renderer) {
	SDL_DestroyTexture(lcdTexture);
}

const std::map < KCPP::LCDStyle::LCDElementAlignment, std::string_view > lcdElementAlignmentItems {
	{KCPP::LCDStyle::LCDElementAlignment::LEFT, "Left"},
	{KCPP::LCDStyle::LCDElementAlignment::RIGHT, "Right"}
};

const std::map < KCPP::LCDStyle::LCDElementAttachment, std::string_view > lcdElementAttachmentItems {
	{KCPP::LCDStyle::LCDElementAttachment::ATTACHED, "Attached"},
	{KCPP::LCDStyle::LCDElementAttachment::DETACHED, "Detached"}
};

static bool lcdElementAlignmentOption(const char *label, KCPP::LCDStyle::LCDElementAlignment &option) {
	bool edited = false;
	if (ImGui::BeginCombo(label, lcdElementAlignmentItems.at(option).data())) {
		for (const auto &i : lcdElementAlignmentItems) {
			bool selectedI = i.first == option;
			if (ImGui::Selectable(i.second.data(), &selectedI)) {
				option = i.first;
				edited = true;
			}
		}
		ImGui::EndCombo();
	}
	return edited;
}

static bool lcdElementAttachmentOption(const char *label, KCPP::LCDStyle::LCDElementAttachment &option) {
	bool edited = false;
	if (ImGui::BeginCombo(label, lcdElementAttachmentItems.at(option).data())) {
		for (const auto &i : lcdElementAttachmentItems) {
			bool selectedI = i.first == option;
			if (ImGui::Selectable(i.second.data(), &selectedI)) {
				option = i.first;
				edited = true;
			}
		}
		ImGui::EndCombo();
	}
	return edited;
}

void KCPP::LCDStyle::LCDStyle::imGuiSettingsMenu() {
	bool edited = false;

	bool bgColourEdited = false;
	bool inactiveColourEdited = false;
	bool activeColourEdited = false;
	bool prestigeColourEdited = false;
	// FIXME: feels inefficient, optimise?
	auto backgroundColour = getContigousLcdColour4(save.bg_colour());
	auto inactiveColour = getContigousLcdColour4(save.inactive_colour());
	auto activeColour = getContigousLcdColour3(save.active_colour());
	auto prestigeActiveColour = getContigousLcdColour3(save.prestige_active_colour());
	edited |= bgColourEdited = ImGui::ColorEdit4("LCD Background Colour", backgroundColour.data());
	edited |= inactiveColourEdited = ImGui::ColorEdit4("Inactive Segment Colour", inactiveColour.data());
	edited |= activeColourEdited = ImGui::ColorEdit3("Active Segment Colour", activeColour.data());
	edited |= prestigeColourEdited = ImGui::ColorEdit3("Prestige Segment Colour", prestigeActiveColour.data());
	if (bgColourEdited) {
		save.mutable_bg_colour()->set_r(backgroundColour[static_cast < ::std::size_t >(PixelComponent::R)]);
		save.mutable_bg_colour()->set_g(backgroundColour[static_cast < ::std::size_t >(PixelComponent::G)]);
		save.mutable_bg_colour()->set_b(backgroundColour[static_cast < ::std::size_t >(PixelComponent::B)]);
		save.mutable_bg_colour()->set_a(backgroundColour[static_cast < ::std::size_t >(PixelComponent::A)]);
	}
	if (inactiveColourEdited) {
		save.mutable_inactive_colour()->set_r(inactiveColour[static_cast < ::std::size_t >(PixelComponent::R)]);
		save.mutable_inactive_colour()->set_g(inactiveColour[static_cast < ::std::size_t >(PixelComponent::G)]);
		save.mutable_inactive_colour()->set_b(inactiveColour[static_cast < ::std::size_t >(PixelComponent::B)]);
		save.mutable_inactive_colour()->set_a(inactiveColour[static_cast < ::std::size_t >(PixelComponent::A)]);
	}
	if (activeColourEdited) {
		save.mutable_active_colour()->set_r(activeColour[static_cast < ::std::size_t >(PixelComponent::R)]);
		save.mutable_active_colour()->set_g(activeColour[static_cast < ::std::size_t >(PixelComponent::G)]);
		save.mutable_active_colour()->set_b(activeColour[static_cast < ::std::size_t >(PixelComponent::B)]);
	}
	if (prestigeColourEdited) {
		save.mutable_prestige_active_colour()->set_r(prestigeActiveColour[static_cast < ::std::size_t >(PixelComponent::R)]);
		save.mutable_prestige_active_colour()->set_g(prestigeActiveColour[static_cast < ::std::size_t >(PixelComponent::G)]);
		save.mutable_prestige_active_colour()->set_b(prestigeActiveColour[static_cast < ::std::size_t >(PixelComponent::B)]);
	}

	ImGui::SeparatorText("Fast Quit");

	bool fastQuitEdited;
	bool fastQuit = save.fast_quit();
	edited |= fastQuitEdited = ImGui::Checkbox("Fast Quit", &fastQuit);
	if (fastQuitEdited)
		save.set_fast_quit(fastQuit);

	ImGui::SeparatorText("Introduction");

	std::string userName = "";
	if (save.has_user_name_policy()) {
		switch (save.user_name_policy()) {
			case ::KCPP::LCDStyle::LCDUserNameType::DisplayName:
				if (::KCPP::UsernameFinder::displayName.has_value()) {
					userName = ::KCPP::UsernameFinder::displayName.value();
					break;
				}
			case ::KCPP::LCDStyle::LCDUserNameType::UserName:
			default:
				if (::KCPP::UsernameFinder::technicalName) {
					userName = ::KCPP::UsernameFinder::technicalName.value();
					break;
				}
		}
	} else if (save.has_user_name_text()) {
		userName = save.user_name_text();
	}

	ImGui::BeginDisabled(!save.has_user_name_text());
	{
		bool userNameEdited = false;
		edited |= userNameEdited = ImGui::InputText("User Name", &userName);
		if (userNameEdited)
			save.set_user_name_text(userName);
	}
	ImGui::EndDisabled();

	for (const auto character : userName) {
		if (!KCPP::LCDStyle::Font::glyphs.contains(character)) {
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Some characters will be displayed as a stylised cross.");
			break;
		}
	}

	ImGui::BeginDisabled(!KCPP::UsernameFinder::technicalName.has_value());
	if (ImGui::Button("Set to Username")) {
		save.set_user_name_policy(KCPP::LCDStyle::LCDUserNameType::UserName);
	}
	ImGui::EndDisabled();

	ImGui::SameLine();

	ImGui::BeginDisabled(!KCPP::UsernameFinder::displayName.has_value());
	if (ImGui::Button("Set to Display Name")) {
		save.set_user_name_policy(KCPP::LCDStyle::LCDUserNameType::DisplayName);
	}
	ImGui::EndDisabled();

	ImGui::SameLine();

	if (ImGui::Button("Set to Custom Name")) {
		save.set_user_name_text(userName);
	}

	ImGui::BeginDisabled(!queuedTickerAnimations.empty());
	if (ImGui::Button("Preview Intro Ticker Animation")) {
		addIntroTickerAnimation();
	}
	ImGui::EndDisabled();


	ImGui::SeparatorText("Elements");

	bool grabBarAlignmentEdited = false;
	bool grabBarAttachmentEdited = false;
	bool prestigeCounterAlignmentEdited = false;
	bool prestigeCounterAttachmentEdited = false;
	LCDElementAlignment grabBarAlignment = save.lcd_grab_bar_alignment();
	LCDElementAttachment grabBarAttachment = save.lcd_grab_bar_attachment();
	LCDElementAlignment prestigeCounterAlignment = save.lcd_prestige_counter_alignment();
	LCDElementAttachment prestigeCounterAttachment = save.lcd_prestige_counter_attachment();
	edited |= grabBarAlignmentEdited = lcdElementAlignmentOption("Grab Bar Alignment", grabBarAlignment);
	edited |= grabBarAttachmentEdited = lcdElementAttachmentOption("Grab Bar Attachment", grabBarAttachment);
	edited |= prestigeCounterAlignmentEdited = lcdElementAlignmentOption("Prestige Counter Alignment", prestigeCounterAlignment);
	edited |= prestigeCounterAttachmentEdited = lcdElementAttachmentOption("Prestige Counter Attachment", prestigeCounterAttachment);
	if (grabBarAlignmentEdited)
		save.set_lcd_grab_bar_alignment(grabBarAlignment);
	if (grabBarAttachmentEdited)
		save.set_lcd_grab_bar_attachment(grabBarAttachment);
	if (prestigeCounterAlignmentEdited)
		save.set_lcd_prestige_counter_alignment(prestigeCounterAlignment);
	if (prestigeCounterAttachmentEdited)
		save.set_lcd_prestige_counter_attachment(prestigeCounterAttachment);

	if (edited)
		resetRenderer();

	editJustPerformed |= edited;
}

void KCPP::LCDStyle::LCDStyle::parseSettings(const std::string &data) {
	save.ParseFromString(data);

	if (save.lcd_save_version() < 1) {
		if (save.has_active_colour_double()) {
			save.mutable_active_colour()->set_r(static_cast < float >(save.active_colour_double().r()));
			save.mutable_active_colour()->set_g(static_cast < float >(save.active_colour_double().g()));
			save.mutable_active_colour()->set_b(static_cast < float >(save.active_colour_double().b()));
		}
		if (save.has_prestige_active_colour_double()) {
			save.mutable_prestige_active_colour()->set_r(static_cast < float >(save.prestige_active_colour_double().r()));
			save.mutable_prestige_active_colour()->set_g(static_cast < float >(save.prestige_active_colour_double().g()));
			save.mutable_prestige_active_colour()->set_b(static_cast < float >(save.prestige_active_colour_double().b()));
		}
		if (save.has_inactive_colour_double()) {
			save.mutable_inactive_colour()->set_r(static_cast < float >(save.inactive_colour_double().r()));
			save.mutable_inactive_colour()->set_g( static_cast < float >(save.inactive_colour_double().g()));
			save.mutable_inactive_colour()->set_b(static_cast < float >(save.inactive_colour_double().b()));
			save.mutable_inactive_colour()->set_a(static_cast < float >(save.inactive_colour_double().a()));
		}
		if (save.has_inactive_colour()) {
			save.mutable_bg_colour()->set_r(static_cast < float >(save.bg_colour_double().r()));
			save.mutable_bg_colour()->set_g(static_cast < float >(save.bg_colour_double().g()));
			save.mutable_bg_colour()->set_b(static_cast < float >(save.bg_colour_double().b()));
			save.mutable_bg_colour()->set_a(static_cast < float >(save.bg_colour_double().a()));
		}
	}

	save.set_lcd_save_version(1);

/*	if (save.has_user_name_text()) {
		userNameConfiguration = UserNameConfiguration::UserChosen;
		userName = lcdStyleProtoSave.user_name_text();
	} else if (save.has_user_name_policy())
		userNameConfiguration = static_cast < KCPP::LCDStyle::LCDStyle::UserNameConfiguration >(save.user_name_policy());*/
}

std::string KCPP::LCDStyle::LCDStyle::generateSettings() {
	save.mutable_active_colour_double()->set_r(save.active_colour().r());
	save.mutable_active_colour_double()->set_g(save.active_colour().g());
	save.mutable_active_colour_double()->set_b(save.active_colour().b());

	save.mutable_prestige_active_colour_double()->set_r(save.prestige_active_colour().r());
	save.mutable_prestige_active_colour_double()->set_g(save.prestige_active_colour().g());
	save.mutable_prestige_active_colour_double()->set_b(save.prestige_active_colour().b());

	save.mutable_inactive_colour_double()->set_r(save.inactive_colour().r());
	save.mutable_inactive_colour_double()->set_g(save.inactive_colour().g());
	save.mutable_inactive_colour_double()->set_b(save.inactive_colour().b());
	save.mutable_inactive_colour_double()->set_a(save.inactive_colour().a());

	save.mutable_bg_colour_double()->set_r(save.bg_colour().r());
	save.mutable_bg_colour_double()->set_g(save.bg_colour().g());
	save.mutable_bg_colour_double()->set_b(save.bg_colour().b());
	save.mutable_bg_colour_double()->set_a(save.bg_colour().a());

	std::string data {};
	save.SerializeToString(&data);
	return data;
}
