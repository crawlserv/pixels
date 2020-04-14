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
	// approximates sine for x > 0 using a Taylor series of five summands
	template<typename F = double>
	F approxSinTaylor(F x) {
		static_assert(std::is_floating_point<F>::value, "The type of the argument needs to be a floating point number");

		constexpr F epsilon = 0.0001;

		if(x < epsilon)
			return 0.;

		if(x > 2 * M_PI)
			x = std::fmod(x, 2 * M_PI);

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

	// approximates sine for x > 0 with a quadratic equation
	template<typename F = double>
	F approxSinQuad(F x) {
		static_assert(std::is_floating_point<F>::value, "The type of the argument needs to be a floating point number");

		constexpr F epsilon = 0.0001;

		if(x < epsilon)
			return 0.;

		constexpr double div_2pi = 0.5 * M_1_PI;

		F t = x * div_2pi;	// x / (2 * pi)

		t -= (int) t;		// - floor(x)

		if(t < 0.5)
			return - 16 * t * t + 8 * t;

		return 16 * t * t - 24 * t + 8;
	}

	// aproximate sine for x > 0 with a cubic equation
	template<typename F = double>
	F approxSinCubic(F x) {
		static_assert(std::is_floating_point<F>::value, "The type of the argument needs to be a floating point number");

		constexpr F epsilon = 0.0001;

		if(x < epsilon)
			return 0.;

		constexpr double div_2pi = 0.5 * M_1_PI;

		F t = x * div_2pi;	// x / (2 * pi)

		t -= (int) t;		// - floor(x)

		const F tSquared = t * t;

		return 20.785 * t * tSquared - 31.1775 * tSquared + 10.3925 * t;
	}
}

#endif /* MATH_H_ */
