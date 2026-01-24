// KC++.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <iostream>
#include "Style.h"
#include "KCCommon.h"

namespace KCPP {
	extern KCPP::Style *currentStyle;
	void setCounter(KCPP::CounterType newInputCounter);
	KCPP::CounterType getCounter(void);
	void setPrestige(KCPP::PrestigeType newPrestige);
	KCPP::PrestigeType getPrestige(void);

	bool isWindowShown(void);
	void hideWindow();
}

// TODO: Reference additional headers your program requires here.
