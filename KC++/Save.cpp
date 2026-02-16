#include "Save.h"
#include "WarningManagement.h"

#include "InputChecker.h"
#pragma warning( push, 1 )
#pragma warning(disable : 4371 4365 4626 5027 4100 4946 4371 5267 5243)
#include <KC++Save.pb.h>
#pragma warning( pop )
#include "KC++.h"
#include "Styles.h"
#include <fstream>

KCPP::Save::Save kcppProtoSave {};

void KCPP::Save::save() {
	kcppProtoSave.set_counter(KCPP::getCounter());
	kcppProtoSave.set_prestige(KCPP::getPrestige());

	kcppProtoSave.set_keyboardinput(KCPP::InputChecker::getInputEnabled(KCPP::InputChecker::InputType::Keyboard));
	kcppProtoSave.set_mouseinput(KCPP::InputChecker::getInputEnabled(KCPP::InputChecker::InputType::Mouse));
	kcppProtoSave.set_controllerinput(KCPP::InputChecker::getInputEnabled(KCPP::InputChecker::InputType::Controller));

	if (KCPP::currentStyle)
		kcppProtoSave.set_current_style(KCPP::currentStyle->getInternalName());

	google::protobuf::Map<std::string, std::string> *styleSettings = kcppProtoSave.mutable_style_settings();
	
	for (auto &i : *(KCPP::Styles::availableStyles.get())) {
		styleSettings->operator[](i.first) = i.second->generateSettings();
	}

	const auto windowPos = KCPP::getWindowPosition();

	kcppProtoSave.set_window_x(windowPos.first);
	kcppProtoSave.set_window_y(windowPos.second);
	
	{
		std::ofstream saveStream("kc++save.pbbin", std::ios::trunc | std::ios::binary);
		kcppProtoSave.SerializeToOstream(&saveStream);
	}
}

void KCPP::Save::load() {
	{
		std::ifstream saveStream("kc++save.pbbin", std::ios::binary);
		if (!kcppProtoSave.ParseFromIstream(&saveStream))
			return;
	}

	KCPP::setCounter(static_cast < KCPP::CounterType >(kcppProtoSave.counter()));
	KCPP::setPrestige(static_cast < KCPP::PrestigeType >(kcppProtoSave.prestige()));

	KCPP::InputChecker::setInputEnabled(KCPP::InputChecker::InputType::Keyboard, kcppProtoSave.keyboardinput());
	KCPP::InputChecker::setInputEnabled(KCPP::InputChecker::InputType::Mouse, kcppProtoSave.mouseinput());
	KCPP::InputChecker::setInputEnabled(KCPP::InputChecker::InputType::Controller, kcppProtoSave.controllerinput());

	if (KCPP::Styles::availableStyles->contains(kcppProtoSave.current_style()))
		KCPP::currentStyle = KCPP::Styles::availableStyles->operator[](kcppProtoSave.current_style()).get();

	for (const auto &i : kcppProtoSave.style_settings()) {
		if (KCPP::Styles::availableStyles->contains(i.first)) {
			KCPP::Styles::availableStyles->operator[](i.first)->parseSettings(i.second);
		}
	}

	if (kcppProtoSave.has_window_x() && kcppProtoSave.has_window_y())
	KCPP::setWindowPosition({kcppProtoSave.window_x(), kcppProtoSave.window_y()});
}
