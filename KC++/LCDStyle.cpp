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

KCPP::HitTestResult KCPP::LCDStyle::LCDStyle::hitTest(SDL_Window *window, const SDL_Point *cursorPos) {
	switch (lcdGrabBarAlignment) {
		case ::KCPP::LCDStyle::LCDElementAlignment::LEFT:
			if (cursorPos->x <= (KCPP::LCDStyle::Font::width + 1) * lastContentScale)
				return KCPP::HitTestResult::None;
			else if (cursorPos->x <= (KCPP::LCDStyle::Font::width * 2 + 2) * lastContentScale)
				return KCPP::HitTestResult::Menu;
			break;
		case ::KCPP::LCDStyle::LCDElementAlignment::RIGHT:
			if (cursorPos->x >= (getLcdTextureSize()[0] - (KCPP::LCDStyle::Font::width + 1)) * lastContentScale)
				return KCPP::HitTestResult::None;
			else if (cursorPos->x >= (getLcdTextureSize()[0] - (KCPP::LCDStyle::Font::width + 1) * 2) * lastContentScale)
				return KCPP::HitTestResult::Menu;
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

void KCPP::LCDStyle::LCDStyle::init(SDL_Renderer *renderer) {
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

	return {textureWidth, textureHeight};
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

static inline constexpr ::std::size_t getPixelIndex(SDL_Surface *surface, const ::std::size_t x, const ::std::size_t y) {
	return (x * 4) + (y * (surface->pitch / sizeof(float)));
}

enum class PixelComponent {
	R = 0,
	G = 1,
	B = 2,
	A = 3
};

static inline float &pixel(SDL_Surface *surface, const ::std::size_t x, const ::std::size_t y, const PixelComponent component) {
	return reinterpret_cast < float * >(surface->pixels)[getPixelIndex(surface, x, y) + static_cast < ::std::size_t >(component)];
}

void KCPP::LCDStyle::LCDStyle::renderGlyph(const KCPP::LCDStyle::Font::Glyph &glyph, int xOffset, int yOffset, KCPP::CounterType counter, KCPP::PrestigeType prestige, decltype(activeColour) activeColour, decltype(inactiveColour) inactiveColour) {
	for (std::size_t y = 0; y < KCPP::LCDStyle::Font::height; ++y) {
		for (std::size_t x = 0; x < KCPP::LCDStyle::Font::width; ++x) {
			if (glyph[y][x] || KCPP::randomPrestigeProgressEvent(counter, prestige, 0.1)) {
				std::memcpy(&reinterpret_cast < float * >(lcdTextureSurface->pixels)[getPixelIndex(lcdTextureSurface, x + xOffset, y + yOffset)], activeColour.data(), sizeof(activeColour));
				pixel(lcdTextureSurface, x + xOffset, y + yOffset, PixelComponent::A) = 1;
			} else {
				std::memcpy(&reinterpret_cast < float * >(lcdTextureSurface->pixels)[getPixelIndex(lcdTextureSurface, x + xOffset, y + yOffset)], inactiveColour.data(), sizeof(inactiveColour));
			}
		}
	}
}

void KCPP::LCDStyle::LCDStyle::renderGlyphWithChar(char character, int xOffset, int yOffset, KCPP::CounterType counter, KCPP::PrestigeType prestige, decltype(activeColour) activeColour, decltype(inactiveColour) inactiveColour) {
	renderGlyph(KCPP::LCDStyle::Font::glyphs.contains(character)
				? KCPP::LCDStyle::Font::glyphs[character]
				: KCPP::LCDStyle::Font::unknownCharacter,
				xOffset,
				yOffset,
				counter,
				prestige,
				activeColour,
				inactiveColour);
}

void KCPP::LCDStyle::LCDStyle::renderGlyphs(std::string string, int xOffset, int yOffset, KCPP::CounterType count, KCPP::PrestigeType prestige, decltype(activeColour) activeColour, decltype(inactiveColour) inactiveColour, std::string::size_type wantedRequiredLength, KCPP::LCDStyle::TextAlignment textAlignment) {
	std::string::size_type stringLength = string.size() <= wantedRequiredLength ? wantedRequiredLength : string.size();
	std::string::size_type stringReadOffset = string.size() <= wantedRequiredLength ? wantedRequiredLength - string.size() : 0;

	if (textAlignment == TextAlignment::Left)
		stringReadOffset = 0;

	for (std::string::size_type i = 0; i < stringLength; i++) {
		renderGlyphWithChar(i - stringReadOffset < string.size() ? string[i - stringReadOffset] : ' ',
							(i * KCPP::LCDStyle::Font::width) /* glyph size */
							+ i	                              /* glyph padding */
							+ xOffset,
							yOffset,
							count,
							prestige,
							activeColour,
							inactiveColour
		);
	}
}

void KCPP::LCDStyle::LCDStyle::render(SDL_Renderer *renderer, KCPP::CounterType count, KCPP::PrestigeType prestige) {
	SDL_SetRenderDrawColorFloat(renderer, 0, 0, 0, 0);
	SDL_RenderClear(renderer);

	SDL_LockTextureToSurface(lcdTexture, nullptr, &lcdTextureSurface);

	std::size_t grabBarDetachPoint = 0;
	std::size_t prestigeDetachPoint = 0;
	constexpr std::size_t glyphsNeededForPrestigeCounter = ::KCPP::calculateGlyphsNeededForMaximumPrestigeCounter();

	int grabBarOffset = 1;
	int prestigeOffset = 1;
	int counterOffset = 1;



	if (lcdGrabBarAlignment == ::KCPP::LCDStyle::LCDElementAlignment::LEFT) {
		prestigeOffset += (::KCPP::LCDStyle::Font::width + 1) * 2;
		if (lcdGrabBarAttachment == ::KCPP::LCDStyle::LCDElementAttachment::DETACHED) {
			grabBarDetachPoint = prestigeOffset;
			prestigeOffset += 2;
		}
		counterOffset = prestigeOffset;
	}

	if (lcdPrestigeAlignment == ::KCPP::LCDStyle::LCDElementAlignment::LEFT) {
		constexpr std::size_t offsetFromCounter = (::KCPP::LCDStyle::Font::width + 1) * ::KCPP::calculateGlyphsNeededForMaximumPrestigeCounter();
		counterOffset += offsetFromCounter;
		if (lcdPrestigeAttachment == ::KCPP::LCDStyle::LCDElementAttachment::DETACHED) {
			//if (lcdGrabBarAlignment == ::KCPP::LCDStyle::LCDElementAlignment::LEFT)
			prestigeDetachPoint = counterOffset;
			counterOffset += 2;
		}
	}

	if (lcdPrestigeAlignment == ::KCPP::LCDStyle::LCDElementAlignment::RIGHT) {
		constexpr std::size_t offsetFromCounter = (::KCPP::LCDStyle::Font::width + 1) * ::KCPP::calculateGlyphsNeededForMaximumCounter();
		prestigeOffset += offsetFromCounter;
		if (lcdPrestigeAttachment == ::KCPP::LCDStyle::LCDElementAttachment::DETACHED) {
			prestigeDetachPoint = prestigeOffset;
			prestigeOffset += 2;
		}
	}

	if (lcdGrabBarAlignment == ::KCPP::LCDStyle::LCDElementAlignment::RIGHT) {
		if (counterOffset > prestigeOffset) {
			constexpr std::size_t offsetFromCounter = (::KCPP::LCDStyle::Font::width + 1) * (::KCPP::calculateGlyphsNeededForMaximumCounter() - 1);
			grabBarOffset = counterOffset + offsetFromCounter;
		} else {
			constexpr std::size_t offsetFromCounter = (::KCPP::LCDStyle::Font::width + 1) * (::KCPP::calculateGlyphsNeededForMaximumPrestigeCounter() - 1);
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

	for (std::size_t x = 0; x < getLcdTextureSize()[0]; x++) {
		bool fillColumnWithTransparent = x == grabBarDetachPoint || x == prestigeDetachPoint;

		for (std::size_t y = 0; y < getLcdTextureSize()[1]; y++) {
			std::size_t pixelIndex = (x * 4) + (y * (lcdTextureSurface->pitch / sizeof(float)));

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
	renderGlyph(lcdGrabBarAlignment == ::KCPP::LCDStyle::LCDElementAlignment::LEFT ? KCPP::LCDStyle::Font::menuCharacter : KCPP::LCDStyle::Font::dragCharacter, grabBarOffset + KCPP::LCDStyle::Font::width + 1, 1, count, prestige, activeColour, inactiveColour);

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
			signed long long lcdStringPosition = counterGlyphs - static_cast< signed long long >(phase);

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

			double timeMult = ((touch.timeStamp + touchDuration - currentTouchTick) / static_cast < double >(touchDuration));

			for (std::size_t y = 0; y < getLcdTextureSize()[1]; y++) {
				for (std::size_t x = 0; x < getLcdTextureSize()[0]; x++) {
					double effectivePixelX = x + 0.5;
					double effectivePixelY = y + 0.5;
					double dx = fabs(effectivePixelX - touch.x);
					double dy = fabs(effectivePixelY - touch.y);
					double distance = sqrtf(dx * dx + dy * dy);
					double pixelAmount = (1 - (distance / touchRadius / timeMult));

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
	if (SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND))
		std::terminate();
	if (SDL_SetTextureBlendMode(lcdTexture, SDL_BLENDMODE_BLEND))
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
	return std::array<int, 2> {static_cast < int >(ceil(getLcdTextureSize()[0] * contentScale)), static_cast < int >(ceil(getLcdTextureSize()[1] * contentScale))};
}

void KCPP::LCDStyle::LCDStyle::quit(SDL_Renderer *renderer) {
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

	activeColour[0] = lcdStyleProtoSave.activecolour().r();
	activeColour[1] = lcdStyleProtoSave.activecolour().g();
	activeColour[2] = lcdStyleProtoSave.activecolour().b();

	inactiveColour[0] = lcdStyleProtoSave.inactivecolour().r();
	inactiveColour[1] = lcdStyleProtoSave.inactivecolour().g();
	inactiveColour[2] = lcdStyleProtoSave.inactivecolour().b();
	inactiveColour[3] = lcdStyleProtoSave.inactivecolour().a();

	backgroundColour[0] = lcdStyleProtoSave.bgcolour().r();
	backgroundColour[1] = lcdStyleProtoSave.bgcolour().g();
	backgroundColour[2] = lcdStyleProtoSave.bgcolour().b();
	backgroundColour[3] = lcdStyleProtoSave.bgcolour().a();

	if (lcdStyleProtoSave.has_user_name_text()) {
		userNameConfiguration = UserNameConfiguration::UserChosen;
		userName = lcdStyleProtoSave.user_name_text();
	} else if (lcdStyleProtoSave.has_user_name_policy())
		userNameConfiguration = static_cast < KCPP::LCDStyle::LCDStyle::UserNameConfiguration >(lcdStyleProtoSave.user_name_policy());
}

std::string KCPP::LCDStyle::LCDStyle::generateSettings() {
	KCPP::LCDStyle::LCDStyleSave lcdStyleProtoSave {};

	lcdStyleProtoSave.mutable_activecolour()->set_r(activeColour[0]);
	lcdStyleProtoSave.mutable_activecolour()->set_g(activeColour[1]);
	lcdStyleProtoSave.mutable_activecolour()->set_b(activeColour[2]);

	lcdStyleProtoSave.mutable_prestige_active_colour()->set_r(prestigeActiveColour[0]);
	lcdStyleProtoSave.mutable_prestige_active_colour()->set_g(prestigeActiveColour[1]);
	lcdStyleProtoSave.mutable_prestige_active_colour()->set_b(prestigeActiveColour[2]);

	lcdStyleProtoSave.mutable_inactivecolour()->set_r(inactiveColour[0]);
	lcdStyleProtoSave.mutable_inactivecolour()->set_g(inactiveColour[1]);
	lcdStyleProtoSave.mutable_inactivecolour()->set_b(inactiveColour[2]);
	lcdStyleProtoSave.mutable_inactivecolour()->set_a(inactiveColour[3]);

	lcdStyleProtoSave.mutable_bgcolour()->set_r(backgroundColour[0]);
	lcdStyleProtoSave.mutable_bgcolour()->set_g(backgroundColour[1]);
	lcdStyleProtoSave.mutable_bgcolour()->set_b(backgroundColour[2]);
	lcdStyleProtoSave.mutable_bgcolour()->set_a(backgroundColour[3]);

	if (userNameConfiguration == UserNameConfiguration::UserChosen)
		lcdStyleProtoSave.set_user_name_text(userName);
	else
		lcdStyleProtoSave.set_user_name_policy(static_cast < KCPP::LCDStyle::LCDUserNameType >(userNameConfiguration));

	std::string data {};
	lcdStyleProtoSave.SerializeToString(&data);
	return data;
}
