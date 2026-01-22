#pragma once

#include <cstddef>
#include <limits>
#include <exception>

namespace KCPP {
	using CounterType = std::uint_least64_t;

	constexpr KCPP::CounterType counterTypePrecisionWanted = 1;

	constexpr std::size_t fixedPoint = 6;

	constexpr KCPP::CounterType incrementCounterTypePrecise(KCPP::CounterType value) {
		//return std::nextafter(value, std::numeric_limits < KCPP::CounterType >::max());
		return value + counterTypePrecisionWanted;
	}

	constexpr KCPP::CounterType constexprExp(std::size_t base, std::size_t exponent) {
		KCPP::CounterType result = 1;
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
				constexprExp(2, currentExponent) + counterTypePrecisionWanted == constexprExp(2, currentExponent)
				) {
				return constexprExp(2, currentExponent);
			} else {
				currentExponent++;
			}
		}
	}

	constexpr std::size_t calculateGlyphsNeededForMaximumCounter() {
		if (std::numeric_limits < KCPP::CounterType >::is_exact)
			return std::numeric_limits < KCPP::CounterType >::digits10 + (fixedPoint != 0 ? 1 : 0);

		KCPP::CounterType maxCounter = calculateMaximumCounterAllowingForPrecision() / counterTypePrecisionWanted;
		std::size_t digitsNeeded = 0;
		while (maxCounter >= 1) {
			maxCounter /= 10;
			digitsNeeded++;
		}
		return digitsNeeded;
	}

	//constexpr KCPP::CounterType maximumCounterAllowingForPrecision = calculateMaximumCounterAllowingForPrecision();
}