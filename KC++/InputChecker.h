#pragma once

#include "KCCommon.h"
#include <limits>
#include <atomic>
#include <iostream>

namespace KCPP {
	namespace InputChecker {
		constexpr KCPP::CounterType keyReward = 1000000;

		constexpr KCPP::CounterType mouseButtonReward = keyReward;
		constexpr KCPP::CounterType mouseMotionReward = 50;
		constexpr KCPP::CounterType mouseWheelReward = 50000;

		constexpr KCPP::CounterType controllerButtonReward = keyReward;
		constexpr KCPP::CounterType controllerMotionReward = 100;

		enum class InputType : uint8_t {
			Keyboard   = 0b001,
			Mouse	   = 0b010,
			Controller = 0b100,
			All        = 0b111
		};

		void init();

		bool getInputEnabled(InputType type);
		void setInputEnabled(InputType type, bool enabled);

		void quit();

		bool checkInput();
		KCPP::CounterType newInputCount();
		void clear();

		inline void safeAddToInputCounter(std::atomic < KCPP::CounterType > &target, KCPP::CounterType newCount) {
			/*KCPP::CounterType currentValue = target.load();

			if (currentValue >= KCPP::calculateMaximumCounterAllowingForPrecision() - newCount) {
				target.store(currentValue + newCount);
				std::cout << currentValue << '\n';
			} else {
				target.store(KCPP::calculateMaximumCounterAllowingForPrecision());
				std::cout << "OVR" << '\n';
			}*/

			target.fetch_add(newCount);
		}
	}
}