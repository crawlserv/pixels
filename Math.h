/*
 * Math.h
 *
 *  Created on: Apr 14, 2020
 *      Author: ans
 */

#ifndef MATH_H_
#define MATH_H_

#define _USE_MATH_DEFINES

#include <cmath>		// std::fmod
#include <type_traits>	// std::is_floating_point

namespace Math {
	// approximates sine with a Taylor series of five
	template<typename F = double>
	F approxSinTaylor(F x) {
		static_assert(std::is_floating_point<F>::value, "The type of the argument needs to be a floating point number");

		if(x > 2 * M_PI)
			x = std::fmod(x, 2 * M_PI);

		if(x < 0.0001)
			return 0.;

		if(x <= M_PI_2) {
			// in this quadrant the Taylor magic happens
			const F x2 = x * x;
			const F x3 = x2 * x;
			const F x5 = x3 * x2;
			const F x7 = x5 * x2;
			const F x9 = x7 * x2;

			constexpr F divF3 = 1. / (3 * 2);
			constexpr F divF5 = 1. / (5 * 4 * 3 * 2);
			constexpr F divF7 = 1. / (7 * 6 * 5 * 4 * 3 * 2);
			constexpr F divF9 = 1. / (9 * 8 * 7 * 6 * 5 * 4 * 3 * 2);
			constexpr F divF11 = 1. / (11 * 10 * 9 * 8 * 7 * 6 * 5 * 4 * 3 * 2);

			return x
					- x3 * divF3
					+ x5 * divF5
					- x7 * divF7
					+ x9 * divF9
					- x9 * x2 * divF11;
		}

		if(x <= M_PI)
			return approxSinTaylor(M_PI - x);

		if(x <= M_PI + M_PI_2)
			return - approxSinTaylor(x - M_PI);

		return - approxSinTaylor(2 * M_PI - x);
	}
}

#endif /* MATH_H_ */
