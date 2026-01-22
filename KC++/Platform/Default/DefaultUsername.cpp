#include "../../UsernameFinder.h"

#define DEFAULT_USERNAME_FINDER_STRATEGY_SDL_FOLDER 0
#define DEFAULT_USERNAME_FINDER_STRATEGY_EMPTY 1
#define DEFAULT_USERNAME_FINDER_STRATEGY DEFAULT_USERNAME_FINDER_STRATEGY_EMPTY

std::optional<std::string> KCPP::UsernameFinder::determineUserName(KCPP::UsernameFinder::UsernameType usernameType) {
#if DEFAULT_USERNAME_FINDER_STRATEGY == DEFAULT_USERNAME_FINDER_STRATEGY_EMPTY
	return std::optional<std::string>();
#elif DEFAULT_USERNAME_FINDER_STRATEGY == DEFAULT_USERNAME_FINDER_STRATEGY_SDL_FOLDER
	if (usernameType == KCPP::UsernameFinder::UsernameType::Display) {
		return std::optional<std::string>();
	}

	const char *sdlUserFolder = SDL_GetUserFolder(SDL_Folder::SDL_FOLDER_HOME);

	if (sdlUserFolder) {
		std::string sdlUserFolderString {sdlUserFolder};
		sdlUserFolderString.erase(sdlUserFolderString.size() - 1);
		return sdlUserFolderString.substr(sdlUserFolderString.find_last_of(std::filesystem::path::preferred_separator) + 1);
	} else {
		return {};
	}
#endif
}