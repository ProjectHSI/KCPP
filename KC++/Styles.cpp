#include "Styles.h"

std::unique_ptr < std::map < std::string, std::unique_ptr < KCPP::Style > > > KCPP::Styles::availableStyles;

bool KCPP::Styles::addStyle(const std::string &name, KCPP::Style *style) {
	static bool mapInitalised = false;

	if (!mapInitalised) {
		mapInitalised = true;
		availableStyles = std::make_unique < std::map < std::string, std::unique_ptr < KCPP::Style > > >();
	}

	availableStyles->operator[](name) = std::unique_ptr < KCPP::Style >(style);

	return false;
}
