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
	if (cursorPos->x <= (KCPP::LCDStyle::Font::width + 1) * lastContentScale)
		return KCPP::HitTestResult::None;
	else if (cursorPos->x <= (KCPP::LCDStyle::Font::width * 2 + 2) * lastContentScale)
		return KCPP::HitTestResult::Menu;
	else
		return KCPP::HitTestResult::StylePassthrough;
}

bool KCPP::LCDStyle::LCDStyle::processEvent(SDL_Event &event) {
	switch (event.type) {
		case SDL_EVENT_MOUSE_BUTTON_DOWN: {
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
	return KCPP::calculateGlyphsNeededForMaximumCombinedCounter() + 2 /* for menu */;
}

static constexpr std::size_t glyphsNeeded = getGlyphsNeeded();

static constexpr std::array < int, 2 > getLcdTextureSize() {
	constexpr int textureHeight = KCPP::LCDStyle::Font::height + 2;
	constexpr int textureWidth =
		KCPP::LCDStyle::Font::width * (glyphsNeeded + 1) /* glyph space */
		+ (1 * ((glyphsNeeded + 1) - 1)) /* glyph padding */
		+ 2 /* whole area padding */;

	return {textureWidth, textureHeight};
}

void KCPP::LCDStyle::LCDStyle::resetRenderer(SDL_Renderer *renderer) {
	if (lcdTexture != nullptr) {
		SDL_DestroyTexture(lcdTexture);
		lcdTexture = nullptr;
	}
	lcdTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA128_FLOAT, SDL_TEXTUREACCESS_STREAMING, getLcdTextureSize()[0], getLcdTextureSize()[1]);
	SDL_SetTextureScaleMode(lcdTexture, SDL_SCALEMODE_PIXELART);
}

static constexpr ::std::size_t getPixelIndex(SDL_Surface *surface, const ::std::size_t x, const ::std::size_t y) {
	return (x * 4) + (y * (surface->pitch / sizeof(float)));
}

enum class PixelComponent {
	R = 0,
	G = 1,
	B = 2,
	A = 3
};

static float &pixel(SDL_Surface *surface, const ::std::size_t x, const ::std::size_t y, const PixelComponent component) {
	return reinterpret_cast < float * >(surface->pixels)[getPixelIndex(surface, x, y) + static_cast < ::std::size_t >(component)];
}

void KCPP::LCDStyle::LCDStyle::renderGlyph(SDL_Renderer *renderer, const KCPP::LCDStyle::Font::Glyph &glyph, int xOffset, int yOffset, KCPP::CounterType counter, KCPP::PrestigeType prestige) {
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

void KCPP::LCDStyle::LCDStyle::renderGlyphWithChar(SDL_Renderer *renderer, char character, int xOffset, int yOffset, KCPP::CounterType counter, KCPP::PrestigeType prestige) {
	renderGlyph(renderer,
				KCPP::LCDStyle::Font::glyphs.contains(character)
					? KCPP::LCDStyle::Font::glyphs[character]
					: KCPP::LCDStyle::Font::unknownCharacter,
				xOffset,
				yOffset,
				counter,
				prestige);
}

void KCPP::LCDStyle::LCDStyle::render(SDL_Renderer *renderer, KCPP::CounterType count, KCPP::PrestigeType prestige) {
	SDL_SetRenderDrawColorFloat(renderer, 0, 0, 0, 0);
	SDL_RenderClear(renderer);

	SDL_LockTextureToSurface(lcdTexture, nullptr, &lcdTextureSurface);

	for (std::size_t y = 0; y < getLcdTextureSize()[1]; y++) {
		for (std::size_t x = 0; x < getLcdTextureSize()[0]; x++) {
			std::size_t pixelIndex = (x * 4) + (y * (lcdTextureSurface->pitch / sizeof(float)));
			std::memcpy(&reinterpret_cast < float * >(lcdTextureSurface->pixels)[getPixelIndex(lcdTextureSurface, x, y)], backgroundColour.data(), sizeof(backgroundColour));
		}
	}

	renderGlyph(renderer, KCPP::LCDStyle::Font::dragCharacter, 1, 1, count, prestige);
	renderGlyph(renderer, KCPP::LCDStyle::Font::menuCharacter, 1 + KCPP::LCDStyle::Font::width + 1, 1, count, prestige);

	if (queuedTickerAnimations.empty()) {
		std::string counterString = calculateCombinedString(count, prestige);

		for (size_t i = 0; i < glyphsNeeded - 1; i++) {
			size_t offsetI = i + 2;

			renderGlyphWithChar(renderer, i < counterString.length() ? counterString[i] : ' ',
								(offsetI * KCPP::LCDStyle::Font::width) /* glyph size */
								+ offsetI	                            /* glyph padding */
								+ 1,
								1,
								count,
								prestige
			);
		}
	} else {
		if (queuedTickerAnimations.front().startTime == std::numeric_limits < decltype (TickerAnimation::startTime) >::max()) {
			queuedTickerAnimations.front().startTime = SDL_GetTicks();
		}

		std::size_t phase = (SDL_GetTicks() - queuedTickerAnimations.front().startTime) / tickerTimer;
		
		if (phase >= queuedTickerAnimations.front().text.length() + glyphsNeeded + 1) {
			queuedTickerAnimations.pop_front();

			for (size_t i = 0; i < glyphsNeeded - 1; i++) {
				size_t offsetI = i + 2;

				renderGlyphWithChar(renderer, ' ',
									(offsetI * KCPP::LCDStyle::Font::width) /* glyph size */
									+ offsetI	                            /* glyph padding */
									+ 1,
									1,
									count,
									prestige
				);
			}
		} else {
			signed long long lcdStringPosition = glyphsNeeded - static_cast< signed long long >(phase);
			for (size_t i = 0; i < glyphsNeeded - 1; i++) {
				size_t offsetI = i + 2;
				size_t lcdStringIndex = i - lcdStringPosition;
				if (lcdStringIndex >= queuedTickerAnimations.front().text.size()) {
					renderGlyphWithChar(renderer, ' ',
										(offsetI * KCPP::LCDStyle::Font::width) /* glyph size */
										+ offsetI	                            /* glyph padding */
										+ 1,
										1,
										count,
										prestige
					);
					continue;
				} else {
					renderGlyphWithChar(renderer, queuedTickerAnimations.front().text.at(i - lcdStringPosition),
										(offsetI * KCPP::LCDStyle::Font::width) /* glyph size */
										+ offsetI	                            /* glyph padding */
										+ 1,
										1,
										count,
										prestige
					);
				}
			}
		}
	}

	while (!currentTouches.empty()) {
		if (currentTouches.front().timeStamp + touchDuration < SDL_GetTicks()) {
			currentTouches.pop_front();
		} else {
			break;
		}
	}

	if (!currentTouches.empty()) {
		for (auto touchI = currentTouches.crbegin(); touchI != currentTouches.crend(); ++touchI) {
			const auto &touch = *touchI;
			constexpr size_t touchRadius = 16;
			for (std::size_t y = 0; y < getLcdTextureSize()[1]; y++) {
				for (std::size_t x = 0; x < getLcdTextureSize()[0]; x++) {
					double effectivePixelX = x + 0.5;
					double effectivePixelY = y + 0.5;
					double dx = fabs(effectivePixelX - touch.x);
					double dy = fabs(effectivePixelY - touch.y);
					double distance = sqrtf(dx * dx + dy * dy);
					double timeMult = ((touch.timeStamp + touchDuration - SDL_GetTicks()) / static_cast < double >(touchDuration));
					double pixelAmount = (1 - (distance / touchRadius / timeMult));

					float dR = activeColour[0] - pixel(lcdTextureSurface, x, y, PixelComponent::R);
					float dG = activeColour[1] - pixel(lcdTextureSurface, x, y, PixelComponent::G);
					float dB = activeColour[2] - pixel(lcdTextureSurface, x, y, PixelComponent::B);
					float dA = 1               - pixel(lcdTextureSurface, x, y, PixelComponent::A);

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

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
	SDL_SetTextureBlendMode(lcdTexture, SDL_BLENDMODE_NONE);
	SDL_RenderTexture(renderer, lcdTexture, nullptr, nullptr);
}



bool KCPP::LCDStyle::LCDStyle::sizeChangeNeeded(SDL_Window *window) {
	if (lastContentScale != SDL_GetWindowDisplayScale(window) * 3) {
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



void KCPP::LCDStyle::LCDStyle::imGuiSettingsMenu() {
	bool edited = false;

	edited |= ImGui::ColorEdit4("Background Colour", backgroundColour.data());
	edited |= ImGui::ColorEdit4("Inactive Segment Colour", inactiveColour.data());
	edited |= ImGui::ColorEdit3("Active Segment Colour", activeColour.data());

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
	}
	else if (lcdStyleProtoSave.has_user_name_policy()) 
		userNameConfiguration = static_cast < KCPP::LCDStyle::LCDStyle::UserNameConfiguration >(lcdStyleProtoSave.user_name_policy());
}

std::string KCPP::LCDStyle::LCDStyle::generateSettings() {
	KCPP::LCDStyle::LCDStyleSave lcdStyleProtoSave {};

	lcdStyleProtoSave.mutable_activecolour()->set_r(activeColour[0]);
	lcdStyleProtoSave.mutable_activecolour()->set_g(activeColour[1]);
	lcdStyleProtoSave.mutable_activecolour()->set_b(activeColour[2]);

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
