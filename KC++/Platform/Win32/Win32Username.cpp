#include "../../UsernameFinder.h"

#define WIN32_LEAN_AND_MEAN
#define SECURITY_WIN32
#include <Windows.h>
#include <Security.h>
#include <secext.h>

#pragma comment( lib, "user32.lib") 
#pragma comment( lib, "gdi32.lib")
#pragma comment( lib, "Secur32.lib")

std::optional<std::string> KCPP::UsernameFinder::determineUserName(KCPP::UsernameFinder::UsernameType usernameType, bool forceExactMatch) {
	EXTENDED_NAME_FORMAT nameFormat = usernameType == KCPP::UsernameFinder::UsernameType::Display ? EXTENDED_NAME_FORMAT::NameDisplay : EXTENDED_NAME_FORMAT::NameUnknown;

	std::string userName;

	ULONG userNameSize = 0;
	if (nameFormat != EXTENDED_NAME_FORMAT::NameUnknown)
		GetUserNameExA(nameFormat, nullptr, &userNameSize);
	else
		GetUserNameA(nullptr, &userNameSize);

	bool canMoveToAlternates = (nameFormat != EXTENDED_NAME_FORMAT::NameUnknown && !forceExactMatch);

	if (GetLastError() != ERROR_MORE_DATA && GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
		if (canMoveToAlternates && nameFormat != EXTENDED_NAME_FORMAT::NameUnknown) {
			nameFormat = EXTENDED_NAME_FORMAT::NameUnknown;
			GetUserNameA(nullptr, &userNameSize);

			if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
				return std::optional<std::string>();
			}
		} else {
			return std::optional<std::string>();
		}
	}

	userName.resize(userNameSize);
	if (nameFormat != EXTENDED_NAME_FORMAT::NameUnknown)
		GetUserNameExA(nameFormat, userName.data(), &userNameSize);
	else
		GetUserNameA(userName.data(), &userNameSize);

	if (nameFormat == EXTENDED_NAME_FORMAT::NameUnknown)
		userName.resize(userNameSize - 1); // Remove null terminator
	else
		userName.resize(userNameSize); // Remove spurious null terminators

	return userName;
}
