// KC++.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <iostream>
#include "Style.h"

namespace KCPP {
	extern KCPP::Style *currentStyle;
	void setCounter(KCPP::CounterType newInputCounter);
	KCPP::CounterType getCounter(void);
}

// TODO: Reference additional headers your program requires here.
