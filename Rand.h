/*
 * Rand.h
 *
 *  Created on: Apr 11, 2020
 *      Author: ans
 */

#ifndef RAND_H_
#define RAND_H_

#define RAND_ALGOS 3

#include <cstdlib>		// std::rand
#include <limits>		// std::numeric_limits
#include <random>		// std::mt19937, std::random_device, std::uniform_int_distribution, std::uniform_real_distribution
#include <stdexcept>	// std::runtime_error
#include <string>		// std::string

class Rand {
public:
	enum Algo {
		RAND_ALGO_STD_RAND = 0,
		RAND_ALGO_STD_MT19937 = 1,
		RAND_ALGO_LEHMER32 = 2
	};

	Rand();
	Rand(Algo algo);
	virtual ~Rand();

	void seed(unsigned int s);
	void setAlgo(Algo n);

	Algo getAlgo() const;
	std::string str() const;

	void setByteLimits(unsigned char from, unsigned char to);
	void setIntLimits(int from, int to);
	void setRealLimits(double from, double to);

	unsigned char generateByte();
	int generateInt();
	double generateReal();
	bool generateBool();

private:
	Algo algo;

	unsigned char byteMin;
	unsigned char byteMax;
	unsigned char byteHalf;
	int intMin;
	int intMax;
	int intHalf;
	double realMin;
	double realMax;
	double realHalf;

	std::mt19937 mt;

	std::uniform_int_distribution<unsigned char> byteDist;
	std::uniform_int_distribution<int> intDist;
	std::uniform_real_distribution<double> realDist;

	uint32_t lehmer;
	uint32_t lehmer32();
};

#endif /* RAND_H_ */
