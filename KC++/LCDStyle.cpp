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
#include <LCDStyleSave.pb.h>
#include "KC++.h"

constexpr std::size_t tickerTimer = 50; // ms per character
constexpr std::size_t touchDuration = 2000; // ms


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

}

KCPP::HitTestResult KCPP::LCDStyle::LCDStyle::hitTest([[maybe_unused]] SDL_Window *hitTestWindow, const SDL_FPoint *cursorPos) {
	switch (lcdGrabBarAlignment) {
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
	switch (userNameConfiguration) {
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
	}
	addIntroTickerAnimation();
}

void KCPP::LCDStyle::LCDStyle::addIntroTickerAnimation(void) {
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

	if (lcdGrabBarAttachment == LCDElementAttachment::DETACHED)
		variableTextureSize[0] += 2;
	if (lcdPrestigeAttachment == LCDElementAttachment::DETACHED)
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

void KCPP::LCDStyle::LCDStyle::renderGlyph(const KCPP::LCDStyle::Font::Glyph &glyph, int xOffset, int yOffset, KCPP::CounterType counter, KCPP::PrestigeType prestige, decltype(activeColour) currentActiveColour, decltype(inactiveColour) currentInactiveColour ) {
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

void KCPP::LCDStyle::LCDStyle::renderGlyphWithChar(char character, int xOffset, int yOffset, KCPP::CounterType counter, KCPP::PrestigeType prestige, decltype(activeColour) currentActiveColour, decltype(inactiveColour) currentInactiveColour) {
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

void KCPP::LCDStyle::LCDStyle::renderGlyphs(std::string string, int xOffset, int yOffset, KCPP::CounterType count, KCPP::PrestigeType prestige, decltype(activeColour) currentActiveColour, decltype(inactiveColour) currentInactiveColour, std::string::size_type wantedRequiredLength, KCPP::LCDStyle::TextAlignment textAlignment) {
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



	if (lcdGrabBarAlignment == ::KCPP::LCDStyle::LCDElementAlignment::LEFT) {
		prestigeOffset += static_cast < int >((::KCPP::LCDStyle::Font::width + 1) * 2);
		if (lcdGrabBarAttachment == ::KCPP::LCDStyle::LCDElementAttachment::DETACHED) {
			grabBarDetachPoint = prestigeOffset;
			prestigeOffset += 2;
		}
		counterOffset = prestigeOffset;
	}

	if (lcdPrestigeAlignment == ::KCPP::LCDStyle::LCDElementAlignment::LEFT) {
		constexpr int offsetFromCounter = static_cast < int >((::KCPP::LCDStyle::Font::width + 1) * ::KCPP::calculateGlyphsNeededForMaximumPrestigeCounter());
		counterOffset += offsetFromCounter;
		if (lcdPrestigeAttachment == ::KCPP::LCDStyle::LCDElementAttachment::DETACHED) {
			//if (lcdGrabBarAlignment == ::KCPP::LCDStyle::LCDElementAlignment::LEFT)
			prestigeDetachPoint = counterOffset;
			counterOffset += 2;
		}
	}

	if (lcdPrestigeAlignment == ::KCPP::LCDStyle::LCDElementAlignment::RIGHT) {
		constexpr int offsetFromCounter = static_cast < int >((::KCPP::LCDStyle::Font::width + 1) * ::KCPP::calculateGlyphsNeededForMaximumCounter());
		prestigeOffset += offsetFromCounter;
		if (lcdPrestigeAttachment == ::KCPP::LCDStyle::LCDElementAttachment::DETACHED) {
			prestigeDetachPoint = prestigeOffset;
			prestigeOffset += 2;
		}
	}

	if (lcdGrabBarAlignment == ::KCPP::LCDStyle::LCDElementAlignment::RIGHT) {
		if (counterOffset > prestigeOffset) {
			constexpr int offsetFromCounter = static_cast < int >((::KCPP::LCDStyle::Font::width + 1) * (::KCPP::calculateGlyphsNeededForMaximumCounter() - 1));
			grabBarOffset = counterOffset + offsetFromCounter;
		} else {
			constexpr int offsetFromCounter = static_cast < int >((::KCPP::LCDStyle::Font::width + 1) * (::KCPP::calculateGlyphsNeededForMaximumPrestigeCounter() - 1));
			grabBarOffset = prestigeOffset + offsetFromCounter;
		}
		grabBarOffset += (::KCPP::LCDStyle::Font::width + 1);
		if (lcdGrabBarAttachment == ::KCPP::LCDStyle::LCDElementAttachment::DETACHED) {
			grabBarDetachPoint = grabBarOffset;
			grabBarOffset += 2;
		}
	}

	grabBarDetachPoint = grabBarDetachPoint ? grabBarDetachPoint : -1;
	prestigeDetachPoint = prestigeDetachPoint ? prestigeDetachPoint : -1;

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

	renderGlyph(lcdGrabBarAlignment == ::KCPP::LCDStyle::LCDElementAlignment::LEFT ? KCPP::LCDStyle::Font::dragCharacter : KCPP::LCDStyle::Font::menuCharacter, grabBarOffset, 1, count, prestige, activeColour, inactiveColour);
	renderGlyph(lcdGrabBarAlignment == ::KCPP::LCDStyle::LCDElementAlignment::LEFT ? KCPP::LCDStyle::Font::menuCharacter : KCPP::LCDStyle::Font::dragCharacter, grabBarOffset + static_cast < int >(KCPP::LCDStyle::Font::width) + 1, 1, count, prestige, activeColour, inactiveColour);

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
				viewableString = viewableString.substr(-lcdStringPosition, counterGlyphs);
			else {
				outputIterator += lcdStringPosition;
			}

			std::copy(viewableString.cbegin(), viewableString.cend(), outputIterator);
			
			counterString = counterString.substr(0, counterGlyphs);
		}
	}

	constexpr auto counterMaxLength = calculateGlyphsNeededForMaximumCounter();
	constexpr auto prestigeCounterMaxLength = calculateGlyphsNeededForMaximumPrestigeCounter();

	::KCPP::LCDStyle::TextAlignment counterTextAlignment = lcdPrestigeAlignment == LCDElementAlignment::LEFT ? TextAlignment::Right : TextAlignment::Left;
	::KCPP::LCDStyle::TextAlignment prestigeTextAlignment = lcdPrestigeAlignment == LCDElementAlignment::LEFT ? TextAlignment::Left : TextAlignment::Right;

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

					float dR = activeColour[0] - pixel(lcdTextureSurface, x, y, PixelComponent::R);
					float dG = activeColour[1] - pixel(lcdTextureSurface, x, y, PixelComponent::G);
					float dB = activeColour[2] - pixel(lcdTextureSurface, x, y, PixelComponent::B);
					float dA = 1 - pixel(lcdTextureSurface, x, y, PixelComponent::A);

					if (distance <= touchRadius && pixelAmount > 0) {
						pixel(lcdTextureSurface, x, y, PixelComponent::R) += pixelAmount * dR; // R
						pixel(lcdTextureSurface, x, y, PixelComponent::G) += pixelAmount * dG; // G
						pixel(lcdTextureSurface, x, y, PixelComponent::B) += pixelAmount * dB; // B
						pixel(lcdTextureSurface, x, y, PixelComponent::A) += pixelAmount * dA; // A
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
}



bool KCPP::LCDStyle::LCDStyle::sizeChangeNeeded(SDL_Window *window) {
	if (lastContentScale != SDL_GetWindowDisplayScale(window) * 3 || editJustPerformed) {
		lastContentScale = SDL_GetWindowDisplayScale(window) * 3;
		return true;
	} else {
		return false;
	}
}

std::array<int, 2> KCPP::LCDStyle::LCDStyle::getSize(SDL_Window *window) const {
	float contentScale = SDL_GetWindowDisplayScale(window) * 3;
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
		for (const auto i : lcdElementAlignmentItems) {
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
		for (const auto i : lcdElementAttachmentItems) {
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

	edited |= ImGui::ColorEdit4("LCD Background Colour", backgroundColour.data());
	edited |= ImGui::ColorEdit4("Inactive Segment Colour", inactiveColour.data());
	edited |= ImGui::ColorEdit3("Active Segment Colour", activeColour.data());
	edited |= ImGui::ColorEdit3("Prestige Segment Colour", prestigeActiveColour.data());


	ImGui::Separator();


	ImGui::BeginDisabled(userNameConfiguration != UserNameConfiguration::UserChosen);
	edited |= ImGui::InputText("User Name", &userName);
	ImGui::EndDisabled();

	for (const auto character : userName) {
		if (!KCPP::LCDStyle::Font::glyphs.contains(character)) {
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Some characters will be displayed as a stylised cross.");
			break;
		}
	}

	ImGui::BeginDisabled(!KCPP::UsernameFinder::technicalName.has_value());
	if (ImGui::Button("Set to Username")) {
		userName = KCPP::UsernameFinder::technicalName.value();
		userNameConfiguration = UserNameConfiguration::TechnicalName;
	}
	ImGui::EndDisabled();

	ImGui::SameLine();

	ImGui::BeginDisabled(!KCPP::UsernameFinder::displayName.has_value());
	if (ImGui::Button("Set to Display Name")) {
		userName = KCPP::UsernameFinder::displayName.value();
		userNameConfiguration = UserNameConfiguration::DisplayName;
	}
	ImGui::EndDisabled();

	ImGui::SameLine();

	if (ImGui::Button("Set to Custom Name")) {
		userNameConfiguration = UserNameConfiguration::UserChosen;
	}

	ImGui::BeginDisabled(!queuedTickerAnimations.empty());
	if (ImGui::Button("Preview Intro Ticker Animation")) {
		addIntroTickerAnimation();
	}
	ImGui::EndDisabled();


	ImGui::Separator();


	edited |= lcdElementAlignmentOption("Grab Bar Alignment", lcdGrabBarAlignment);
	edited |= lcdElementAttachmentOption("Grab Bar Attachment", lcdGrabBarAttachment);
	edited |= lcdElementAlignmentOption("Prestige Counter Alignment", lcdPrestigeAlignment);
	edited |= lcdElementAttachmentOption("Prestige Counter Attachment", lcdPrestigeAttachment);

	if (edited)
		resetRenderer();

	editJustPerformed |= edited;
}

void KCPP::LCDStyle::LCDStyle::parseSettings(const std::string &data) {
	KCPP::LCDStyle::LCDStyleSave lcdStyleProtoSave {};

	lcdStyleProtoSave.ParseFromString(data);

	if (lcdStyleProtoSave.has_activecolour_double()) {
		activeColour[0] = static_cast < float >(lcdStyleProtoSave.activecolour_double().r());
		activeColour[1] = static_cast < float >(lcdStyleProtoSave.activecolour_double().g());
		activeColour[2] = static_cast < float >(lcdStyleProtoSave.activecolour_double().b());
	} else {
		activeColour[0] = lcdStyleProtoSave.active_colour().r();
		activeColour[1] = lcdStyleProtoSave.active_colour().g();
		activeColour[2] = lcdStyleProtoSave.active_colour().b();
	}

	if (lcdStyleProtoSave.has_prestige_active_colour_double()) {
		prestigeActiveColour[0] = static_cast < float >(lcdStyleProtoSave.prestige_active_colour_double().r());
		prestigeActiveColour[1] = static_cast < float >(lcdStyleProtoSave.prestige_active_colour_double().g());
		prestigeActiveColour[2] = static_cast < float >(lcdStyleProtoSave.prestige_active_colour_double().b());
	} else {
		prestigeActiveColour[0] = lcdStyleProtoSave.prestige_active_colour().r();
		prestigeActiveColour[1] = lcdStyleProtoSave.prestige_active_colour().g();
		prestigeActiveColour[2] = lcdStyleProtoSave.prestige_active_colour().b();
	}

	if (lcdStyleProtoSave.has_inactivecolour_double()) {
		inactiveColour[0] = static_cast < float >(lcdStyleProtoSave.inactivecolour_double().r());
		inactiveColour[1] = static_cast < float >(lcdStyleProtoSave.inactivecolour_double().g());
		inactiveColour[2] = static_cast < float >(lcdStyleProtoSave.inactivecolour_double().b());
		inactiveColour[3] = static_cast < float >(lcdStyleProtoSave.inactivecolour_double().a());
	} else {
		inactiveColour[0] = lcdStyleProtoSave.inactive_colour().r();
		inactiveColour[1] = lcdStyleProtoSave.inactive_colour().g();
		inactiveColour[2] = lcdStyleProtoSave.inactive_colour().b();
		inactiveColour[3] = lcdStyleProtoSave.inactive_colour().a();
	}

	if (lcdStyleProtoSave.has_bgcolour_double()) {
		backgroundColour[0] = static_cast < float >(lcdStyleProtoSave.bgcolour_double().r());
		backgroundColour[1] = static_cast < float >(lcdStyleProtoSave.bgcolour_double().g());
		backgroundColour[2] = static_cast < float >(lcdStyleProtoSave.bgcolour_double().b());
		backgroundColour[3] = static_cast < float >(lcdStyleProtoSave.bgcolour_double().a());
	} else {
		backgroundColour[0] = lcdStyleProtoSave.bg_colour().r();
		backgroundColour[1] = lcdStyleProtoSave.bg_colour().g();
		backgroundColour[2] = lcdStyleProtoSave.bg_colour().b();
		backgroundColour[3] = lcdStyleProtoSave.bg_colour().a();
	}

	if (lcdStyleProtoSave.has_user_name_text()) {
		userNameConfiguration = UserNameConfiguration::UserChosen;
		userName = lcdStyleProtoSave.user_name_text();
	} else if (lcdStyleProtoSave.has_user_name_policy())
		userNameConfiguration = static_cast < KCPP::LCDStyle::LCDStyle::UserNameConfiguration >(lcdStyleProtoSave.user_name_policy());
}

std::string KCPP::LCDStyle::LCDStyle::generateSettings() {
	KCPP::LCDStyle::LCDStyleSave lcdStyleProtoSave {};

	lcdStyleProtoSave.mutable_active_colour()->set_r(activeColour[0]);
	lcdStyleProtoSave.mutable_active_colour()->set_g(activeColour[1]);
	lcdStyleProtoSave.mutable_active_colour()->set_b(activeColour[2]);

	lcdStyleProtoSave.mutable_prestige_active_colour()->set_r(prestigeActiveColour[0]);
	lcdStyleProtoSave.mutable_prestige_active_colour()->set_g(prestigeActiveColour[1]);
	lcdStyleProtoSave.mutable_prestige_active_colour()->set_b(prestigeActiveColour[2]);

	lcdStyleProtoSave.mutable_inactive_colour()->set_r(inactiveColour[0]);
	lcdStyleProtoSave.mutable_inactive_colour()->set_g(inactiveColour[1]);
	lcdStyleProtoSave.mutable_inactive_colour()->set_b(inactiveColour[2]);
	lcdStyleProtoSave.mutable_inactive_colour()->set_a(inactiveColour[3]);

	lcdStyleProtoSave.mutable_bg_colour()->set_r(backgroundColour[0]);
	lcdStyleProtoSave.mutable_bg_colour()->set_g(backgroundColour[1]);
	lcdStyleProtoSave.mutable_bg_colour()->set_b(backgroundColour[2]);
	lcdStyleProtoSave.mutable_bg_colour()->set_a(backgroundColour[3]);

	if (userNameConfiguration == UserNameConfiguration::UserChosen)
		lcdStyleProtoSave.set_user_name_text(userName);
	else
		lcdStyleProtoSave.set_user_name_policy(static_cast < KCPP::LCDStyle::LCDUserNameType >(userNameConfiguration));

	std::string data {};
	lcdStyleProtoSave.SerializeToString(&data);
	return data;
}
