#include "UsernameFinder.h"

KCPP::UsernameFinder::PossibleUserNameType KCPP::UsernameFinder::technicalName {};
KCPP::UsernameFinder::PossibleUserNameType KCPP::UsernameFinder::displayName {};
KCPP::UsernameFinder::PossibleAmbigousUserNameType KCPP::UsernameFinder::defaultName {};

void KCPP::UsernameFinder::refreshCachedUserNames() {
	technicalName = determineUserName(UsernameType::Technical);
	displayName = determineUserName(UsernameType::Display);
	if (technicalName.has_value()) {
		defaultName = std::make_pair(technicalName.value(), UsernameType::Technical);
	} else if (displayName.has_value()) {
		defaultName = std::make_pair(displayName.value(), UsernameType::Display);
	} else {
		defaultName = std::nullopt;
	}
}