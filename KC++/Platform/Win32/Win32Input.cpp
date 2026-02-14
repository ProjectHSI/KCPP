#include "../../InputChecker.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <WinUser.h>
#pragma comment( lib, "user32.lib") 
#pragma comment( lib, "gdi32.lib")

#include <atomic>
#include <iostream>
#include <thread>
#include <unordered_set>
#include <bitset>
#include <cmath>

std::atomic_flag receivedInput = ATOMIC_FLAG_INIT;
std::atomic < KCPP::CounterType > inputCounters {};

std::unordered_set < DWORD > pressedKeys {};

#define LL_HOOKS

LRESULT kbHookProc(int code, WPARAM wParam, LPARAM lParam) {
	PKBDLLHOOKSTRUCT lParamStruct = reinterpret_cast < PKBDLLHOOKSTRUCT >(lParam);

	if ((wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) && !pressedKeys.contains(lParamStruct->scanCode)) {
		KCPP::InputChecker::safeAddToInputCounter(inputCounters, KCPP::InputChecker::keyReward);
		receivedInput.test_and_set();

		pressedKeys.insert(reinterpret_cast < PKBDLLHOOKSTRUCT >(lParam)->scanCode);
	} else if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
		pressedKeys.erase(reinterpret_cast < PKBDLLHOOKSTRUCT >(lParam)->scanCode);
	}

	return CallNextHookEx(nullptr, code, wParam, lParam);
}

POINT lastMousePos = {-1, -1};
unsigned short xButtonsPressed = 0;

LRESULT mouseHookProc(int code, WPARAM wParam, LPARAM lParam) {
	PMSLLHOOKSTRUCT lParamStruct = reinterpret_cast < PMSLLHOOKSTRUCT >(lParam);

	if (wParam == WM_LBUTTONDOWN || wParam == WM_RBUTTONDOWN || wParam == WM_MBUTTONDOWN) {
		KCPP::InputChecker::safeAddToInputCounter(inputCounters, KCPP::InputChecker::mouseButtonReward);
		receivedInput.test_and_set();
	} else if (wParam == WM_MOUSEMOVE) {
		if (lastMousePos.x != -1 && lastMousePos.y != -1) {
			POINT distance = {abs(lParamStruct->pt.x - lastMousePos.x), abs(lParamStruct->pt.y - lastMousePos.y)};

			KCPP::InputChecker::safeAddToInputCounter(inputCounters, static_cast < KCPP::CounterType >(sqrt(static_cast < double >(distance.x * distance.x + distance.y * distance.y)) * KCPP::InputChecker::mouseMotionReward));
			receivedInput.test_and_set();
		}

		lastMousePos = lParamStruct->pt;
	} else if (unsigned short xButton = (lParamStruct->mouseData & 0xFFFF0000) >> 16; wParam == WM_XBUTTONDOWN && !(xButtonsPressed & xButton)) {
		xButtonsPressed |= xButton;
		KCPP::InputChecker::safeAddToInputCounter(inputCounters, KCPP::InputChecker::mouseButtonReward);
		receivedInput.test_and_set();
	} else if (wParam == WM_XBUTTONUP) {
		xButtonsPressed &= ~xButton;
	} else if (wParam == WM_MOUSEWHEEL || wParam == WM_MOUSEHWHEEL) {
		signed short wheelDelta = static_cast < signed short >((lParamStruct->mouseData & 0xFFFF0000) >> 16);
		unsigned short unsignedWheelDelta = static_cast < unsigned short >(abs(wheelDelta)); // convert to unsigned

		double wheelDeltaDouble = static_cast < double >(unsignedWheelDelta) / WHEEL_DELTA;

		KCPP::InputChecker::safeAddToInputCounter(inputCounters, static_cast < KCPP::CounterType >(trunc(wheelDeltaDouble * KCPP::InputChecker::mouseWheelReward)));
		receivedInput.test_and_set();
	}

	return CallNextHookEx(nullptr, code, wParam, lParam);
}

HHOOK keyboardHook = nullptr;
HHOOK mouseHook = nullptr;

std::atomic < uint8_t > enabledInputTypes = 0;

std::atomic_flag continueWin32Thread = ATOMIC_FLAG_INIT;
std::thread win32InputThread;

static void updateInputTypes() {
	uint8_t enabledInputTypesLocal = enabledInputTypes.load();
	if (enabledInputTypesLocal & static_cast < uint8_t >(KCPP::InputChecker::InputType::Keyboard) && !keyboardHook) {
		keyboardHook = SetWindowsHookExA(WH_KEYBOARD_LL, kbHookProc, nullptr, 0);
		if (!keyboardHook) {
			std::cout << "KEYBOARD HOOK FAIL! " << GetLastError() << '\n';
			enabledInputTypes ^= static_cast < uint8_t >(KCPP::InputChecker::InputType::Keyboard);
		}
	} else if (!(enabledInputTypesLocal & static_cast < uint8_t >(KCPP::InputChecker::InputType::Keyboard)) && keyboardHook) {
		UnhookWindowsHookEx(keyboardHook);
		keyboardHook = nullptr;
	}
	if (enabledInputTypesLocal & static_cast < uint8_t >(KCPP::InputChecker::InputType::Mouse) && !mouseHook) {
		mouseHook = SetWindowsHookExA(WH_MOUSE_LL, mouseHookProc, nullptr, 0);
		if (!mouseHook) {
			std::cout << "MOUSE HOOK FAIL! " << GetLastError() << '\n';
			enabledInputTypes ^= static_cast < uint8_t >(KCPP::InputChecker::InputType::Mouse);
		}
	} else if (!(enabledInputTypesLocal & static_cast < uint8_t >(KCPP::InputChecker::InputType::Mouse)) && mouseHook) {
		UnhookWindowsHookEx(mouseHook);
		mouseHook = nullptr;
	}
}

static void win32InputThreadFunc() {
	while (continueWin32Thread.test_and_set()) {
		MSG msg;
		updateInputTypes();
		while (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessageA(&msg);
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	updateInputTypes();
}

void KCPP::InputChecker::init() {;
	continueWin32Thread.test_and_set();
	win32InputThread = std::thread(win32InputThreadFunc);
}

bool KCPP::InputChecker::getInputEnabled(InputType type) {
	return enabledInputTypes & static_cast < uint8_t >(type);
}

void KCPP::InputChecker::setInputEnabled(InputType type, bool enabled) {
	if (enabled) {
		enabledInputTypes |= static_cast < uint8_t >(type);
	} else {
		// tilde does unsigned -> signed
		// so we do a double cast
		enabledInputTypes &= static_cast < uint8_t >(~static_cast < uint8_t >(type));
	}
}

void KCPP::InputChecker::clear() {
	inputCounters.store(0);
	receivedInput.clear();
}

bool KCPP::InputChecker::checkInput() {
	return receivedInput.test();
}
																	 
KCPP::CounterType KCPP::InputChecker::newInputCount() {
	return inputCounters.load();
}

void KCPP::InputChecker::quit() {
	continueWin32Thread.clear();
	win32InputThread.join();
}
