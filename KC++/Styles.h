#pragma once

#include <map>
#include "Style.h"
#include <memory>

namespace KCPP {
	namespace Styles {
		extern std::unique_ptr < std::map < std::string, std::unique_ptr < KCPP::Style > > > availableStyles;
		bool addStyle(const std::string &name, KCPP::Style *style);
	}
}