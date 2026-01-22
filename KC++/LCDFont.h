#pragma once

#include <cstddef>
#include <map>
#include <array>
#include <unordered_map>

namespace KCPP {
	namespace LCDStyle {
		namespace Font {
			constexpr std::size_t height = 8;
			constexpr std::size_t width = 5;

			using Glyph = std::array < std::array < bool, width >, height >;

		#define o 0
		#define X 1

			constexpr Glyph unknownCharacter {{
				{ X, o, X, o, X },
				{ o, X, o, X, o },
				{ X, o, X, o, X },
				{ X, o, X, o, X },
				{ X, o, X, o, X },
				{ o, X, o, X, o },
				{ X, o, X, o, X }
			}};

			constexpr Glyph dragCharacter {{
				{ o, X, o, X, o },
				{ X, X, X, X, X },
				{ o, X, o, X, o },
				{ X, X, X, X, X },
				{ o, X, o, X, o },
				{ X, X, X, X, X },
				{ o, X, o, X, o }
			}};

			constexpr Glyph menuCharacter {{
				{ o, o, o, o, o },
				{ X, X, X, X, X },
				{ o, o, o, o, o },
				{ X, X, X, X, X },
				{ o, o, o, o, o },
				{ X, X, X, X, X },
				{ o, o, o, o, o }
			}};

			static std::unordered_map < char, Glyph > glyphs {
				{' ', Glyph {{
					{ o, o, o, o, o },
					{ o, o, o, o, o },
					{ o, o, o, o, o },
					{ o, o, o, o, o },
					{ o, o, o, o, o },
					{ o, o, o, o, o },
					{ o, o, o, o, o },
					{ o, o, o, o, o }
				}}},
				{'!', Glyph {{
					{ o, o, X, o, o },
					{ o, o, X, o, o },
					{ o, o, X, o, o },
					{ o, o, X, o, o },
					{ o, o, X, o, o },
					{ o, o, o, o, o },
					{ o, o, X, o, o },
					{ o, o, o, o, o }
				}}},
				{',', Glyph {{
					{ o, o, o, o, o },
					{ o, o, o, o, o },
					{ o, o, o, o, o },
					{ o, o, o, o, o },
					{ o, o, o, o, o },
					{ o, X, X, o, o },
					{ o, X, X, o, o },
					{ X, o, o, o, o }
				}}},
				{'.', Glyph {{
					{ o, o, o, o, o },
					{ o, o, o, o, o },
					{ o, o, o, o, o },
					{ o, o, o, o, o },
					{ o, o, o, o, o },
					{ o, X, X, o, o },
					{ o, X, X, o, o },
					{ o, o, o, o, o }
				}}},
				{'A', Glyph {{
					{ o, X, X, X, o },
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ X, X, X, X, X },
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ o, o, o, o, o }
				}}},
				{'B', Glyph {{
					{ X, X, X, X, o },
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ X, X, X, X, o },
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ X, X, X, X, o },
					{ o, o, o, o, o }
				}}},
				{'C', Glyph {{
					{ o, X, X, X, o },
					{ X, o, o, o, X },
					{ X, o, o, o, o },
					{ X, o, o, o, o },
					{ X, o, o, o, o },
					{ X, o, o, o, X },
					{ o, X, X, X, o },
					{ o, o, o, o, o }
				}}},
				{'D', Glyph {{
					{ X, X, X, X, o },
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ X, X, X, X, o },
					{ o, o, o, o, o }
				}}},
				{'E', Glyph {{
					{ X, X, X, X, X },
					{ X, o, o, o, o },
					{ X, o, o, o, o },
					{ X, X, X, X, o },
					{ X, o, o, o, o },
					{ X, o, o, o, o },
					{ X, X, X, X, X },
					{ o, o, o, o, o }
				}}},
				{'F', Glyph {{
					{ X, X, X, X, X },
					{ X, o, o, o, o },
					{ X, o, o, o, o },
					{ X, X, X, X, o },
					{ X, o, o, o, o },
					{ X, o, o, o, o },
					{ X, o, o, o, o },
					{ o, o, o, o, o }
				}}},
				{'G', Glyph {{
					{ o, X, X, X, o },
					{ X, o, o, o, X },
					{ X, o, o, o, o },
					{ X, o, X, X, X },
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ o, X, X, X, o },
					{ o, o, o, o, o }
				}}},
				{'H', Glyph {{
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ X, X, X, X, X },
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ o, o, o, o, o }
				}}},
				{'I', Glyph {{
					{ o, X, X, X, o },
					{ o, o, X, o, o },
					{ o, o, X, o, o },
					{ o, o, X, o, o },
					{ o, o, X, o, o },
					{ o, o, X, o, o },
					{ o, X, X, X, o },
					{ o, o, o, o, o }
				}}},
				{'J', Glyph {{
					{ o, o, o, o, X },
					{ o, o, o, o, X },
					{ o, o, o, o, X },
					{ o, o, o, o, X },
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ o, X, X, X, o },
					{ o, o, o, o, o }
				}}},
				{'K', Glyph {{
					{ X, o, o, o, X },
					{ X, o, o, X, o },
					{ X, o, X, o, o },
					{ X, X, o, o, o },
					{ X, o, X, o, o },
					{ X, o, o, X, o },
					{ X, o, o, o, X },
					{ o, o, o, o, o }
				}}},
				{'L', Glyph {{
					{ X, o, o, o, o },
					{ X, o, o, o, o },
					{ X, o, o, o, o },
					{ X, o, o, o, o },
					{ X, o, o, o, o },
					{ X, o, o, o, o },
					{ X, X, X, X, X },
					{ o, o, o, o, o }
				}}},
				{'M', Glyph {{
					{ X, o, o, o, X },
					{ X, X, o, X, X },
					{ X, o, X, o, X },
					{ X, o, X, o, X },
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ o, o, o, o, o }
				}}},
				{'N', Glyph {{
					{ X, o, o, o, X },
					{ X, X, o, o, X },
					{ X, o, X, o, X },
					{ X, o, o, X, X },
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ o, o, o, o, o }
				}}},
				{'O', Glyph {{
					{ o, X, X, X, o },
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ o, X, X, X, o },
					{ o, o, o, o, o }
				}}},
				{'P', Glyph {{
					{ X, X, X, X, o },
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ X, X, X, X, o },
					{ X, o, o, o, o },
					{ X, o, o, o, o },
					{ X, o, o, o, o },
					{ o, o, o, o, o }
				}}},
				{'Q', Glyph {{
					{ o, X, X, X, o },
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ o, X, X, X, o },
					{ o, o, o, o, X },
					{ o, o, o, o, o }
				}}},
				{'R', Glyph {{
					{ X, X, X, X, o },
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ X, X, X, X, o },
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ o, o, o, o, o }
				}}},
				{'S', Glyph {{
					{ o, X, X, X, o },
					{ X, o, o, o, X },
					{ X, o, o, o, o },
					{ o, X, X, X, o },
					{ o, o, o, o, X },
					{ o, o, o, o, X },
					{ o, X, X, X, o },
					{ o, o, o, o, o }
				}}},
				{'T', Glyph {{
					{ X, X, X, X, X },
					{ o, o, X, o, o },
					{ o, o, X, o, o },
					{ o, o, X, o, o },
					{ o, o, X, o, o },
					{ o, o, X, o, o },
					{ o, o, X, o, o },
					{ o, o, o, o, o }
				}}},
				{'U', Glyph {{
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ o, X, X, X, o },
					{ o, o, o, o, o }
				}}},
				{'V', Glyph {{
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ o, X, o, X, o },
					{ o, o, X, o, o },
					{ o, o, o, o, o }
				}}},
				{'W', Glyph {{
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ X, o, X, o, X },
					{ X, X, o, X, X },
					{ X, o, o, o, X },
					{ o, o, o, o, o }
				}}},
				{'X', Glyph {{
					{ X, o, o, o, X },
					{ o, X, o, X, o },
					{ o, o, X, o, o },
					{ o, o, X, o, o },
					{ o, o, X, o, o },
					{ o, X, o, X, o },
					{ X, o, o, o, X },
					{ o, o, o, o, o }
				}}},
				{'Y', Glyph {{
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ o, X, o, X, o },
					{ o, o, X, o, o },
					{ o, o, X, o, o },
					{ o, o, X, o, o },
					{ o, o, o, o, o }
				}}},
				{'Z', Glyph {{
					{ X, X, X, X, X },
					{ o, o, o, o, X },
					{ o, o, o, X, o },
					{ o, o, X, o, o },
					{ o, X, o, o, o },
					{ X, o, o, o, o },
					{ X, X, X, X, X },
					{ o, o, o, o, o }
				}}},
				{'a', Glyph {{
					{ o, o, o, o, o },
					{ o, o, o, o, o },
					{ o, X, X, X, o },
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ X, o, o, X, X },
					{ o, X, X, o, X },
					{ o, o, o, o, o }
				}}},
				{'b', Glyph {{
					{ X, o, o, o, o },
					{ X, o, o, o, o },
					{ X, X, X, X, o },
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ X, X, X, X, o },
					{ o, o, o, o, o }
				}}},
				{'c', Glyph {{
					{ o, o, o, o, o },
					{ o, o, o, o, o },
					{ o, X, X, X, o },
					{ X, o, o, o, X },
					{ X, o, o, o, o },
					{ X, o, o, o, o },
					{ o, X, X, X, X },
					{ o, o, o, o, o }
				}}},
				{'d', Glyph {{
					{ o, o, o, o, X },
					{ o, o, o, o, X },
					{ o, X, X, X, X },
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ o, X, X, X, X },
					{ o, o, o, o, o }
				}}},
				{'e', Glyph {{
					{ o, o, o, o, o },
					{ o, o, o, o, o },
					{ o, X, X, X, o },
					{ X, o, o, o, X },
					{ X, X, X, X, X },
					{ X, o, o, o, o },
					{ o, X, X, X, X },
					{ o, o, o, o, o }
				}}},
				{'f', Glyph {{
					{ o, o, X, X, o },
					{ o, o, X, o, o },
					{ o, X, X, X, o },
					{ o, o, X, o, o },
					{ o, o, X, o, o },
					{ o, o, X, o, o },
					{ o, o, X, o, o },
					{ o, o, o, o, o }
				}}},
				{'g', Glyph {{
					{ o, o, o, o, o },
					{ o, o, o, o, o },
					{ o, X, X, X, o },
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ o, X, X, X, X },
					{ o, o, o, o, X },
					{ o, X, X, X, o }
				}}},
				{'h', Glyph {{
					{ X, o, o, o, o },
					{ X, o, o, o, o },
					{ X, X, X, X, o },
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ o, o, o, o, o }
				}}},
				{'i', Glyph {{
					{ o, o, X, o, o },
					{ o, o, o, o, o },
					{ o, o, X, o, o },
					{ o, o, X, o, o },
					{ o, o, X, o, o },
					{ o, o, X, o, o },
					{ o, o, X, o, o },
					{ o, o, o, o, o }
				}}},
				{'j', Glyph {{
					{ o, o, X, o, o },
					{ o, o, o, o, o },
					{ o, o, X, o, o },
					{ o, o, X, o, o },
					{ o, o, X, o, o },
					{ o, o, X, o, o },
					{ o, o, X, o, o },
					{ X, X, o, o, o }
				}}},
				{'k', Glyph {{
					{ X, o, o, o, o },
					{ X, o, o, o, o },
					{ X, o, o, X, o },
					{ X, o, X, o, o },
					{ X, X, X, o, o },
					{ X, o, o, X, o },
					{ X, o, o, o, X },
					{ o, o, o, o, o },
				}}},
				{'l', Glyph {{
					{ o, o, X, o, o },
					{ o, o, X, o, o },
					{ o, o, X, o, o },
					{ o, o, X, o, o },
					{ o, o, X, o, o },
					{ o, o, X, o, o },
					{ o, o, o, X, o },
					{ o, o, o, o, o }
				}}},
				{'m', Glyph {{
					{ o, o, o, o, o },
					{ o, o, o, o, o },
					{ X, X, X, X, o },
					{ X, o, X, o, X },
					{ X, o, X, o, X },
					{ X, o, X, o, X },
					{ X, o, X, o, X },
					{ o, o, o, o, o }
				}}},
				{'n', Glyph {{
					{ o, o, o, o, o },
					{ o, o, o, o, o },
					{ X, o, X, X, o },
					{ X, X, o, o, X },
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ o, o, o, o, o }
				}}},
				{'o', Glyph {{
					{ o, o, o, o, o },
					{ o, o, o, o, o },
					{ o, X, X, X, o },
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ o, X, X, X, o },
					{ o, o, o, o, o }
				}}},
				{'p', Glyph {{
					{ o, o, o, o, o },
					{ o, o, o, o, o },
					{ X, X, X, X, o },
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ X, X, o, o, X },
					{ X, o, X, X, o },
					{ X, o, o, o, o }
				}}},
				{'q', Glyph {{
					{ o, o, o, o, o },
					{ o, o, o, o, o },
					{ o, X, X, X, o },
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ X, o, o, X, X },
					{ o, X, X, o, X },
					{ o, o, o, o, X }
				}}},
				{'r', Glyph {{
					{ o, o, o, o, o },
					{ o, o, o, o, o },
					{ X, o, X, X, o },
					{ X, X, o, o, X },
					{ X, o, o, o, o },
					{ X, o, o, o, o },
					{ X, o, o, o, o },
					{ o, o, o, o, o }
				}}},
				{'s', Glyph {{
					{ o, o, o, o, o },
					{ o, o, o, o, o },
					{ o, X, X, X, o },
					{ X, o, o, o, o },
					{ o, X, X, X, o },
					{ o, o, o, o, X },
					{ X, X, X, X, o },
					{ o, o, o, o, o }
				}}},
				{'t', Glyph {{
					{ o, o, X, o, o },
					{ o, o, X, o, o },
					{ o, X, X, X, o },
					{ o, o, X, o, o },
					{ o, o, X, o, o },
					{ o, o, X, o, o },
					{ o, o, o, X, X },
					{ o, o, o, o, o }
				}}},
				{'u', Glyph {{
					{ o, o, o, o, o },
					{ o, o, o, o, o },
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ X, o, o, X, X },
					{ o, X, X, o, X },
					{ o, o, o, o, o }
				}}},
				{'v', Glyph {{
					{ o, o, o, o, o },
					{ o, o, o, o, o },
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ o, X, o, X, o },
					{ o, o, X, o, o },
					{ o, o, o, o, o }
				}}},
				{'w', Glyph {{
					{ o, o, o, o, o },
					{ o, o, o, o, o },
					{ X, o, X, o, X },
					{ X, o, X, o, X },
					{ X, o, X, o, X },
					{ X, o, X, o, X },
					{ X, X, o, X, X },
					{ o, o, o, o, o }
				}}},
				{'x', Glyph {{
					{ o, o, o, o, o },
					{ o, o, o, o, o },
					{ X, o, o, o, X },
					{ o, X, o, X, o },
					{ o, o, X, o, o },
					{ o, X, o, X, o },
					{ X, o, o, o, X },
					{ o, o, o, o, o }
				}}},
				{'y', Glyph {{
					{ o, o, o, o, o },
					{ o, o, o, o, o },
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ o, X, X, X, X },
					{ o, o, o, o, X },
					{ o, X, X, X, o }
				}}},
				{'z', Glyph {{
					{ o, o, o, o, o },
					{ o, o, o, o, o },
					{ X, X, X, X, X },
					{ o, o, o, X, o },
					{ o, o, X, o, o },
					{ o, X, o, o, o },
					{ X, X, X, X, X },
					{ o, o, o, o, o }
				}}},
				{'0', Glyph {{
					{ o, X, X, X, o },
					{ X, o, o, o, X },
					{ X, o, o, X, X },
					{ X, o, X, o, X },
					{ X, X, o, o, X },
					{ X, o, o, o, X },
					{ o, X, X, X, o },
					{ o, o, o, o, o }
				}}},
				{'1', Glyph {{
					{ o, o, X, o, o },
					{ o, X, X, o, o },
					{ o, o, X, o, o },
					{ o, o, X, o, o },
					{ o, o, X, o, o },
					{ o, o, X, o, o },
					{ o, X, X, X, o },
					{ o, o, o, o, o }
				}}},
				{'2', Glyph {{
					{ o, X, X, X, o },
					{ X, o, o, o, X },
					{ o, o, o, o, X },
					{ o, o, o, X, o },
					{ o, o, X, o, o },
					{ o, X, o, o, o },
					{ X, X, X, X, X },
					{ o, o, o, o, o }
				}}},
				{'3', Glyph {{
					{ o, X, X, X, o },
					{ X, o, o, o, X },
					{ o, o, o, o, X },
					{ o, o, X, X, o },
					{ o, o, o, o, X },
					{ X, o, o, o, X },
					{ o, X, X, X, o },
					{ o, o, o, o, o }
				}}},
				{'4', Glyph {{
					{ o, o, o, X, o },
					{ o, o, X, X, o },
					{ o, X, o, X, o },
					{ X, o, o, X, o },
					{ X, X, X, X, X },
					{ o, o, o, X, o },
					{ o, o, o, X, o }
				}}},
				{'5', Glyph {{
					{ X, X, X, X, X },
					{ X, o, o, o, o },
					{ X, X, X, X, o },
					{ o, o, o, o, X },
					{ o, o, o, o, X },
					{ X, o, o, o, X },
					{ o, X, X, X, o },
					{ o, o, o, o, o }
				}}},
				{'6', Glyph {{
					{ o, X, X, X, o },
					{ X, o, o, o, X },
					{ X, o, o, o, o },
					{ X, X, X, X, o },
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ o, X, X, X, o }
				}}},
				{'7', Glyph {{
					{ X, X, X, X, X },
					{ o, o, o, o, X },
					{ o, o, o, o, X },
					{ o, o, o, X, o },
					{ o, o, X, o, o },
					{ o, o, X, o, o },
					{ o, o, X, o, o },
					{ o, o, o, o, o }
				}}},
				{'8', Glyph {{
					{ o, X, X, X, o },
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ o, X, X, X, o },
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ o, X, X, X, o },
					{ o, o, o, o, o }
				}}},
				{'9', Glyph {{
					{ o, X, X, X, o },
					{ X, o, o, o, X },
					{ X, o, o, o, X },
					{ o, X, X, X, X },
					{ o, o, o, o, X },
					{ X, o, o, o, X },
					{ o, X, X, X, o },
					{ o, o, o, o, o }
				}}}
			};

		#undef o
		#undef X
		}
	}
}