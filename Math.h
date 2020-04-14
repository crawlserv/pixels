/*
 * Math.h
 *
 *  Created on: Apr 14, 2020
 *      Author: ans
 */

#ifndef MATH_H_
#define MATH_H_

#include <cmath>		// std::fabs
#include <type_traits>	// std::is_floating_point

namespace Math {
	// calculates the Taylor series for approximating sine
	template<typename F = double>
		F taylor(F x) {
			static_assert(std::is_floating_point<F>::value, "The type of the argument needs to be a floating point number");

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

			F result = x
					- x3 * divF3
					+ x5 * divF5
					- x7 * divF7
					+ x9 * divF9
					- x9 * x2 * divF11;

			return result;
		}

	// approximates sine with a Taylor series
	template<typename F = double>
		F approxSin(F x) {
			static_assert(std::is_floating_point<F>::value, "The type of the argument needs to be a floating point number");

			x = std::fmod(x, 2 * M_PI);

			if(x == 0.)
				return 0.;

			if(x <= M_PI_2)
				return taylor(x);

			if(x <= M_PI)
				return taylor(M_PI - x);

			if(x <= M_PI + M_PI_2)
				return - taylor(x - M_PI);

			return - taylor(2 * M_PI - x);
	}
}

#endif /* MATH_H_ */
