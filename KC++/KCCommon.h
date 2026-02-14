#pragma once

#include <cstddef>
#include <limits>
#include <exception>
#include <array>
#include <utility>
#include <cassert>
#include <string>
#include <format>
#include <iostream>

namespace KCPP {
	using CounterType = std::uint64_t;

	constexpr KCPP::CounterType counterTypePrecisionWanted = 1;

	constexpr std::size_t fixedPoint = 6;

	constexpr KCPP::CounterType incrementCounterTypePrecise(KCPP::CounterType value) {
		//return std::nextafter(value, std::numeric_limits < KCPP::CounterType >::max());
		return value + counterTypePrecisionWanted;
	}

	template < typename Base, typename Exponent >
	constexpr Base constexprExp(Base base, Exponent exponent) {
		Base result = 1;
		for (std::size_t i = 0; i < exponent; ++i) {
			result *= base;
		}
		return result;
	}

	constexpr KCPP::CounterType calculateMaximumCounterAllowingForPrecision() {
		if (std::numeric_limits < KCPP::CounterType >::is_exact)
			return std::numeric_limits < KCPP::CounterType >::max() - (std::numeric_limits < KCPP::CounterType >::max() % constexprExp(10, fixedPoint));

		if (std::is_constant_evaluated())
			throw std::exception();

		std::size_t currentExponent = 0;
		while (true) {
			if (
				constexprExp < KCPP::CounterType > (2, currentExponent) + counterTypePrecisionWanted == constexprExp < KCPP::CounterType >(2, currentExponent)
				) {
				return constexprExp < KCPP::CounterType >(2, currentExponent);
			} else {
				currentExponent++;
			}
		}
	}

	constexpr std::size_t calculateGlyphsNeededForMaximumCounter() {
		if (std::numeric_limits < KCPP::CounterType >::is_exact)
			return std::numeric_limits < KCPP::CounterType >::digits10 + 1 + (fixedPoint != 0 ? 1 : 0);

		KCPP::CounterType maxCounter = calculateMaximumCounterAllowingForPrecision() / counterTypePrecisionWanted;
		std::size_t digitsNeeded = 0;
		while (maxCounter >= 1) {
			maxCounter /= 10;
			digitsNeeded++;
		}
		return digitsNeeded;
	}

	static std::string calculateCounterString(KCPP::CounterType counter) {
		std::string counterString = std::format(std::numeric_limits < KCPP::CounterType >::is_exact ? "{:{}}" : "{:.{}f}", counter, KCPP::calculateGlyphsNeededForMaximumCounter() + 1, 6);

		if constexpr (KCPP::fixedPoint) {
			constexpr std::size_t dotPlace = KCPP::calculateGlyphsNeededForMaximumCounter() - KCPP::fixedPoint;

			for (size_t i = 0; i < dotPlace; i++) {
				counterString[i] = counterString[i + 1];
			}

			if (counter < KCPP::constexprExp < KCPP::CounterType >(10, KCPP::fixedPoint)) {
				for (size_t i = dotPlace - 1; i < KCPP::calculateGlyphsNeededForMaximumCounter(); i++) {
					if (
						i != dotPlace
						&& counterString[i] == ' '
						)
						counterString[i] = '0';
				}
			}

			counterString[dotPlace] = '.';
		}

		if (counterString.length() > KCPP::calculateGlyphsNeededForMaximumCounter() + 1) {
			counterString = counterString.substr(0, KCPP::calculateGlyphsNeededForMaximumCounter() + 1);
			//counterString += "...";
		}

		counterString = counterString.substr(counterString.find_first_not_of(' '));

		return counterString;
	}


	// Prestige

	using PrestigeType = ::std::uint_least8_t;

	struct RomanNumeralStorage {
		PrestigeType romanNumeralValue;
		char romanNumeralChar;
	};
	constexpr std::array romanNumerals {
		RomanNumeralStorage { 1, 'I' },
		RomanNumeralStorage { 5, 'V' },
		RomanNumeralStorage { 10, 'X' },
		RomanNumeralStorage { 50, 'L' },
		RomanNumeralStorage { 100, 'C' },
		//RomanNumeralStorage { 500, 'D'},
		//RomanNumeralStorage { 1000, 'M'}
	};

	constexpr ::std::size_t calculateGlyphsNeededForMaximumPrestigeCounter();

	constexpr std::string calculatePrestigeString(PrestigeType prestigeCounter) {
		std::string romanNumeral = "";

		for (size_t i = 0; i < romanNumerals.size(); i++) {
			std::size_t effectiveI = romanNumerals.size() - 1 - i;
			uint_fast8_t romanNumeralUsedTimes = 0;

			std::array < char, 4 > workingRomanNumeralAdd {{0, 0, 0, 0}};

			while (prestigeCounter >= romanNumerals[effectiveI].romanNumeralValue) {
				romanNumeralUsedTimes++;

				assert(romanNumeralUsedTimes != 5);

				if (romanNumeralUsedTimes == 4 - (effectiveI % 2)) {
					assert(i != 0);
					workingRomanNumeralAdd[0] = romanNumerals[effectiveI].romanNumeralChar;
					workingRomanNumeralAdd[1] = romanNumerals[effectiveI + 1u].romanNumeralChar;
					workingRomanNumeralAdd[2] = '\0';
				} else {
					workingRomanNumeralAdd[romanNumeralUsedTimes - 1u] = romanNumerals[effectiveI].romanNumeralChar;
				}

				prestigeCounter -= romanNumerals[effectiveI].romanNumeralValue;
			}

			romanNumeral += workingRomanNumeralAdd.data();
		}
		return romanNumeral;
	}

	constexpr ::std::size_t calculateGlyphsNeededForMaximumPrestigeCounter() {
		::std::size_t currentGlyphsNeeded = 0;

		for (PrestigeType i = 0; i < std::numeric_limits < PrestigeType >::max(); i++) {
			if (::std::size_t romanNumeralCount = calculatePrestigeString(i).size(); currentGlyphsNeeded < romanNumeralCount)
				currentGlyphsNeeded = romanNumeralCount;
		}

		return currentGlyphsNeeded;
		//return 10;
	}
	static_assert(calculateGlyphsNeededForMaximumPrestigeCounter() > 4);

	constexpr CounterType prestigePoint = static_cast < CounterType >(100000u) * constexprExp < CounterType > (10u, fixedPoint);
	constexpr double prestigePointIncrease = 1.1;

	constexpr CounterType getNextPrestigePoint(PrestigeType prestige) {
		if (prestige == std::numeric_limits < PrestigeType >::max()) 
			return calculateMaximumCounterAllowingForPrecision();
		CounterType currentPrestigePoint = prestigePoint;
		for (PrestigeType i = 0; i < prestige; i++) {
			currentPrestigePoint = static_cast < CounterType >(static_cast < double >(currentPrestigePoint) * prestigePointIncrease);
		}
		return currentPrestigePoint;
	}

	inline bool randomPrestigeProgressEvent(CounterType counter, PrestigeType prestige, double chanceModifier = 1) {
		CounterType nextPrestigePoint = getNextPrestigePoint(prestige);
		double nextPrestigePointProgress = static_cast < double >(counter) / static_cast < double >(nextPrestigePoint);

		nextPrestigePointProgress -= 0.5;

		if (nextPrestigePointProgress <= 0)
			return false;

		nextPrestigePointProgress *= 2;

		int randMin = RAND_MAX - static_cast < int >(static_cast < double >(RAND_MAX) * (nextPrestigePointProgress * chanceModifier));

		int randResult = rand();

		return randResult >= randMin;
	}


	constexpr ::std::size_t calculateGlyphsNeededForMaximumCombinedCounter() {
		return calculateGlyphsNeededForMaximumCounter() + calculateGlyphsNeededForMaximumPrestigeCounter();
	}

	inline ::std::string calculateCombinedString(CounterType counterType, PrestigeType prestigeCounter) {
		std::string combinedString = calculatePrestigeString(prestigeCounter);
		constexpr ::std::size_t prestigeGlyphs = calculateGlyphsNeededForMaximumPrestigeCounter();
		combinedString.resize(prestigeGlyphs, ' ');
		combinedString.insert(combinedString.size(), calculateCounterString(counterType));
		return combinedString;
	}



	constexpr bool EnableVSync = true;
}