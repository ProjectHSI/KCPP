#pragma once

#include <optional>
#include <string>
#include <SDL3/SDL_filesystem.h>
#include <filesystem>

namespace KCPP {
	namespace UsernameFinder {
		enum class UsernameType {
			Display,
			Technical
		};

		using PossibleUserNameType = std::optional < std::string >;
		using PossibleAmbigousUserNameType = std::optional < std::pair < std::string, UsernameType > >;

		PossibleUserNameType determineUserName(UsernameType usernameType, bool forceExactMatch = false);

		inline std::string determineUserNameWithDefault(UsernameType usernameType, const std::string &defaultName, bool forceExactMatch = false) {
			if (std::optional < std::string > userName = determineUserName(usernameType, forceExactMatch); userName.has_value()) {
				return userName.value();
			} else {
				return defaultName;
			}
		}

		extern PossibleUserNameType displayName;
		extern PossibleUserNameType technicalName;
		extern PossibleAmbigousUserNameType defaultName;

		void refreshCachedUserNames();
	}
}