#include "Save.h"

#include "InputChecker.h"
#include <KC++Save.pb.h>
#include "KC++.h"
#include "Styles.h"
#include <fstream>

void KCPP::Save::save() {
	KCPP::Save::Save kcppProtoSave {};

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
	
	{
		std::ofstream saveStream("kc++save.pbbin", std::ios::trunc | std::ios::binary);
		kcppProtoSave.SerializeToOstream(&saveStream);
	}
}

void KCPP::Save::load() {
	KCPP::Save::Save kcppProtoSave {};

	{
		std::ifstream saveStream("kc++save.pbbin", std::ios::binary);
		if (!kcppProtoSave.ParseFromIstream(&saveStream))
			return;
	}

	KCPP::setCounter(kcppProtoSave.counter());
	KCPP::setPrestige(kcppProtoSave.prestige());

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
}
