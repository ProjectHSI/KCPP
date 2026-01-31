#include "Menu.h"

#include <SDL3/SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>
#include "InputChecker.h"
#include "KC++.h"
#include <KC++.png.h>

SDL_Window *menuWindow = nullptr;
SDL_Renderer *menuRenderer = nullptr;

void KCPP::Menu::menuInit() {
	SDL_Surface *kcppPngSurface = SDL_LoadPNG_IO(SDL_IOFromConstMem(kcppPng.data(), kcppPng.size()), true);

	menuWindow = SDL_CreateWindow("KC++ Menu", 400, 300, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN);
	SDL_SetWindowIcon(menuWindow, kcppPngSurface);

	SDL_DestroySurface(kcppPngSurface);

	menuRenderer = SDL_CreateRenderer(menuWindow, nullptr);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

	ImGui::StyleColorsDark();

	ImGui_ImplSDL3_InitForSDLRenderer(menuWindow, menuRenderer);
	ImGui_ImplSDLRenderer3_Init(menuRenderer);
}

bool menuWindowOpen = false;

void KCPP::Menu::openMenu() {
	SDL_ShowWindow(menuWindow);
	SDL_FlashWindow(menuWindow, SDL_FLASH_UNTIL_FOCUSED);
	menuWindowOpen = true;
}

bool KCPP::Menu::menuEvent(SDL_Event &event) {
	return !ImGui_ImplSDL3_ProcessEvent(&event);
}

void KCPP::Menu::menuIterate() {
	ImGuiIO &io = ImGui::GetIO();

	ImGui_ImplSDLRenderer3_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();

	bool openPointsReset = false;
	bool trayIconShowNotif = false;

	ImGui::SetNextWindowSize(io.DisplaySize, ImGuiCond_Always);
	ImGui::SetNextWindowPos({0, 0}, ImGuiCond_Always);
	if (ImGui::Begin("KC++ Configuration", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove)) {
		ImGui::BeginDisabled(!KCPP::isWindowShown());
		if (ImGui::Button("Hide KC++")) {
			trayIconShowNotif = true;
			KCPP::hideWindow();
		}
		ImGui::EndDisabled();
		bool kbEnabled = KCPP::InputChecker::getInputEnabled(KCPP::InputChecker::InputType::Keyboard);
		bool mouseEnabled = KCPP::InputChecker::getInputEnabled(KCPP::InputChecker::InputType::Mouse);
		bool controllerEnabled = KCPP::InputChecker::getInputEnabled(KCPP::InputChecker::InputType::Controller);
		if (ImGui::Checkbox("Enable Keyboard Input", &kbEnabled)) {
			KCPP::InputChecker::setInputEnabled(KCPP::InputChecker::InputType::Keyboard, kbEnabled);
		}
		if (ImGui::Checkbox("Enable Mouse Input", &mouseEnabled)) {
			KCPP::InputChecker::setInputEnabled(KCPP::InputChecker::InputType::Mouse, mouseEnabled);
		}
		ImGui::SetItemTooltip("Enables mouse rewards.\n"
						  "You may want to disable this option if you are getting irregular mouse behaviour in some applications.\n"
						  "Note that on Win32, this will also disable mouse button input.");

		ImGui::BeginDisabled();
		if (ImGui::Checkbox("Enable Controller Input", &controllerEnabled)) {
			KCPP::InputChecker::setInputEnabled(KCPP::InputChecker::InputType::Controller, controllerEnabled);
		}
		ImGui::EndDisabled();

		ImGui::Separator();

		openPointsReset = ImGui::Button("Reset points to 0");

		if (KCPP::currentStyle) {
			ImGui::SeparatorText("Style");
			KCPP::currentStyle->imGuiSettingsMenu();
		}
	}
	ImGui::End();

	if (openPointsReset)
		ImGui::OpenPopup("Reset Points");

	if (ImGui::BeginPopupModal("Reset Points", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::TextUnformatted("Are you sure?");

		if (ImGui::Button("Yes")) {
			KCPP::setCounter(0);
			KCPP::setPrestige(0);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("No")) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	if (trayIconShowNotif)
		ImGui::OpenPopup("Tray Icon");

	if (ImGui::BeginPopupModal("Tray Icon", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::TextUnformatted("You can use the tray icon to show KC++ again.");
		if (ImGui::Button("OK"))
			ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}

	//ImGui::ShowDemoWindow();

	ImGui::Render();

	SDL_SetRenderScale(menuRenderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
	SDL_SetRenderDrawColorFloat(menuRenderer, 0, 0, 0, 1);
	SDL_RenderClear(menuRenderer);

	ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), menuRenderer);

	SDL_RenderPresent(menuRenderer);
}

void KCPP::Menu::closeMenu() {
	SDL_FlashWindow(menuWindow, SDL_FLASH_CANCEL);
	SDL_HideWindow(menuWindow);

	menuWindowOpen = false;
}

void KCPP::Menu::menuQuit() {
	SDL_DestroyWindow(menuWindow);
}
