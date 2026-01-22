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

constexpr std::size_t tickerTimer = 50; // ms per character
constexpr std::size_t touchDuration = 2000; // ms


const char *regName = "lcdStyle";

static bool lcdStyleReg = KCPP::Styles::addStyle(regName, dynamic_cast < KCPP::Style * >(new KCPP::LCDStyle::LCDStyle()));

std::string_view KCPP::LCDStyle::LCDStyle::getInternalName() {
	return regName;
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
	return renderingAnimations || editJustPerformedLocal;
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
	return KCPP::calculateGlyphsNeededForMaximumCounter() + 2 /* for menu */;
}

static constexpr std::array < int, 2 > getLcdTextureSize() {
	constexpr int textureHeight = KCPP::LCDStyle::Font::height + 2;
	constexpr int textureWidth =
		KCPP::LCDStyle::Font::width * (getGlyphsNeeded() + 1) /* glyph space */
		+ (1 * ((getGlyphsNeeded() + 1) - 1)) /* glyph padding */
		+ 2 /* whole area padding */;

	return {textureWidth, textureHeight};
}

void KCPP::LCDStyle::LCDStyle::resetRenderer(SDL_Renderer *renderer) {
	if (lcdTexture != nullptr) {
		SDL_DestroyTexture(lcdTexture);
		lcdTexture = nullptr;
	}
	if (lcdTextureInvert != nullptr) {
		SDL_DestroyTexture(lcdTextureInvert);
		lcdTextureInvert = nullptr;
	}
	lcdTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA128_FLOAT, SDL_TEXTUREACCESS_TARGET, getLcdTextureSize()[0], getLcdTextureSize()[1]);
	lcdTextureInvert = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA128_FLOAT, SDL_TEXTUREACCESS_STREAMING, getLcdTextureSize()[0], getLcdTextureSize()[1]);
	SDL_SetTextureScaleMode(lcdTexture, SDL_SCALEMODE_PIXELART);
	SDL_SetTextureScaleMode(lcdTextureInvert, SDL_SCALEMODE_PIXELART);
}

void KCPP::LCDStyle::LCDStyle::renderGlyph(SDL_Renderer *renderer, SDL_Texture *texture, const KCPP::LCDStyle::Font::Glyph &glyph, int xOffset, int yOffset) {
	SDL_SetRenderTarget(renderer, texture);
	for (std::size_t y = 0; y < KCPP::LCDStyle::Font::height; ++y) {
		for (std::size_t x = 0; x < KCPP::LCDStyle::Font::width; ++x) {
			SDL_FRect pixelRect {
				static_cast< int >(xOffset + x),
				static_cast< int >(yOffset + y),
				1,
				1
			};
			if (glyph[y][x]) {
				SDL_SetRenderDrawColorFloat(renderer,
											activeColour[0],
											activeColour[1],
											KCPP::LCDStyle::LCDStyle::activeColour[2],
											1
				);
			} else {
				SDL_SetRenderDrawColorFloat(renderer,
											inactiveColour[0],
											inactiveColour[1],
											KCPP::LCDStyle::LCDStyle::inactiveColour[2],
											inactiveColour[3]
				);
			}
			SDL_RenderFillRect(renderer, &pixelRect);
		}
	}
	SDL_SetRenderTarget(renderer, nullptr);
}

void KCPP::LCDStyle::LCDStyle::renderGlyphWithChar(SDL_Renderer *renderer, SDL_Texture *texture, char character, int xOffset, int yOffset) {
	renderGlyph(renderer, texture,
				KCPP::LCDStyle::Font::glyphs.contains(character)
					? KCPP::LCDStyle::Font::glyphs[character]
					: KCPP::LCDStyle::Font::unknownCharacter,
				xOffset,
				yOffset);
}

void KCPP::LCDStyle::LCDStyle::render(SDL_Renderer *renderer, KCPP::CounterType count) {
	SDL_SetRenderTarget(renderer, lcdTexture);

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
	SDL_SetTextureBlendMode(lcdTexture, SDL_BLENDMODE_NONE);

	SDL_SetRenderDrawColorFloat(renderer,
								KCPP::LCDStyle::LCDStyle::backgroundColour[0],
								KCPP::LCDStyle::LCDStyle::backgroundColour[1],
								KCPP::LCDStyle::LCDStyle::backgroundColour[2],
								KCPP::LCDStyle::LCDStyle::backgroundColour[3]
	);
	SDL_RenderClear(renderer);

	renderGlyph(renderer, lcdTexture, KCPP::LCDStyle::Font::dragCharacter, 1, 1);
	renderGlyph(renderer, lcdTexture, KCPP::LCDStyle::Font::menuCharacter, 1 + KCPP::LCDStyle::Font::width + 1, 1);

	if (queuedTickerAnimations.empty()) {
		std::string counterString = std::format(std::numeric_limits < KCPP::CounterType >::is_exact ? "{:{}}" : "{:{}.{}f}", count, KCPP::calculateGlyphsNeededForMaximumCounter() + 1, 6);

		if (KCPP::fixedPoint) {
			constexpr std::size_t dotPlace = KCPP::calculateGlyphsNeededForMaximumCounter() - KCPP::fixedPoint;

			for (size_t i = 0; i < dotPlace; i++) {
				counterString[i] = counterString[i + 1];
			}

			if (count < KCPP::constexprExp(10, KCPP::fixedPoint)) {
				for (size_t i = dotPlace - 1; i < KCPP::calculateGlyphsNeededForMaximumCounter(); i++) {
					if (
						i != dotPlace
						&& counterString[i] == ' '
						)
						counterString[i] = '0';
				}
			}

			counterString[dotPlace] = '.';
		}

		if (counterString.length() > getGlyphsNeeded()) {
			counterString = counterString.substr(0, getGlyphsNeeded());
			//counterString += "...";
		}

		for (size_t i = 0; i < getGlyphsNeeded(); i++) {
			size_t offsetI = i + 2;

			renderGlyphWithChar(renderer, lcdTexture, i < counterString.length() ? counterString[i] : ' ',
								(offsetI * KCPP::LCDStyle::Font::width) /* glyph size */
								+ offsetI	                            /* glyph padding */
								+ 1,
								1
			);
		}
	} else {
		if (queuedTickerAnimations.front().startTime == std::numeric_limits < decltype (TickerAnimation::startTime) >::max()) {
			queuedTickerAnimations.front().startTime = SDL_GetTicks();
		}

		std::size_t phase = (SDL_GetTicks() - queuedTickerAnimations.front().startTime) / tickerTimer;
		
		if (phase >= queuedTickerAnimations.front().text.length() + getGlyphsNeeded() + 1) {
			queuedTickerAnimations.pop_front();

			for (size_t i = 0; i < getGlyphsNeeded(); i++) {
				size_t offsetI = i + 2;

				renderGlyphWithChar(renderer, lcdTexture, ' ',
									(offsetI * KCPP::LCDStyle::Font::width) /* glyph size */
									+ offsetI	                            /* glyph padding */
									+ 1,
									1
				);
			}
		} else {
			signed long long lcdStringPosition = getGlyphsNeeded() - static_cast< signed long long >(phase);
			for (size_t i = 0; i < getGlyphsNeeded(); i++) {
				size_t offsetI = i + 2;
				size_t lcdStringIndex = i - lcdStringPosition;
				if (lcdStringIndex >= queuedTickerAnimations.front().text.size()) {
					renderGlyphWithChar(renderer, lcdTexture, ' ',
										(offsetI * KCPP::LCDStyle::Font::width) /* glyph size */
										+ offsetI	                            /* glyph padding */
										+ 1,
										1
					);
					continue;
				} else {
					renderGlyphWithChar(renderer, lcdTexture, queuedTickerAnimations.front().text.at(i - lcdStringPosition),
										(offsetI * KCPP::LCDStyle::Font::width) /* glyph size */
										+ offsetI	                            /* glyph padding */
										+ 1,
										1
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
		float *pixels {};
		int pitch {};

		SDL_LockTexture(lcdTextureInvert, nullptr, reinterpret_cast < void ** >(& pixels), &pitch);

		for (std::size_t y = 0; y < getLcdTextureSize()[1]; y++) {
			for (std::size_t x = 0; x < getLcdTextureSize()[0]; x++) {
				std::size_t pixelIndex = (x * 4) + (y * (pitch / sizeof(float)));
				pixels[pixelIndex + 0] = 0.0f; // R
				pixels[pixelIndex + 1] = 0.0f; // G
				pixels[pixelIndex + 2] = 0.0f; // B
				pixels[pixelIndex + 3] = 0.0f; // A
			}
		}

		for (auto touchI = currentTouches.crbegin(); touchI != currentTouches.crend(); ++touchI) {
			const auto &touch = *touchI;
			constexpr size_t touchRadius = 16;
			for (std::size_t y = 0; y < getLcdTextureSize()[1]; y++) {
				for (std::size_t x = 0; x < getLcdTextureSize()[0]; x++) {
					std::size_t pixelIndex = (x * 4) + (y * (pitch / sizeof(float)));
					double effectivePixelX = x + 0.5;
					double effectivePixelY = y + 0.5;
					double dx = fabs(effectivePixelX - touch.x);
					double dy = fabs(effectivePixelY - touch.y);
					double distance = sqrtf(dx * dx + dy * dy);
					double timeMult = ((touch.timeStamp + touchDuration - SDL_GetTicks()) / static_cast < double >(touchDuration));
					double pixelAmount = (1 - (distance / touchRadius / timeMult));
					if (distance <= touchRadius && pixelAmount > 0) {
						pixels[pixelIndex + 0] += pixelAmount * activeColour[0]; // R
						pixels[pixelIndex + 1] += pixelAmount * activeColour[1]; // G
						pixels[pixelIndex + 2] += pixelAmount * activeColour[2]; // B
						pixels[pixelIndex + 3] = 1.0f; // A
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

		SDL_UnlockTexture(lcdTextureInvert);

		SDL_SetRenderTarget(renderer, lcdTexture);
		SDL_SetTextureBlendMode(lcdTexture, SDL_BLENDMODE_NONE);
		SDL_SetTextureBlendMode(lcdTextureInvert, SDL_BLENDMODE_ADD);
		//SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
		SDL_RenderTexture(renderer, lcdTextureInvert, nullptr, nullptr);
		SDL_SetTextureBlendMode(lcdTexture, SDL_BLENDMODE_NONE);
	}

	SDL_SetRenderTarget(renderer, nullptr);
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
	SDL_DestroyTexture(lcdTextureInvert);
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
