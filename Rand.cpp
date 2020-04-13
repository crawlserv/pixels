/*
 * Rand.cpp
 *
 *  Created on: Apr 11, 2020
 *      Author: ans
 */

#include "Rand.h"

// constructor for using the default algorithm (or setting it later)
Rand::Rand()
		: algo(RAND_ALGO_STD_RAND),
		  byteMin(0),
		  byteMax(std::numeric_limits<unsigned char>::max()),
		  byteHalf(std::numeric_limits<unsigned char>::max() / 2),
		  intMin(0),
		  intMax(std::numeric_limits<int>::max()),
		  intHalf(std::numeric_limits<int>::max() / 2),
		  realMin(0.),
		  realMax(1.),
		  realHalf(.5),
		  lehmer(0) {
	const auto defaultSeed = std::random_device()();

	std::srand(defaultSeed);

	this->mt.seed(defaultSeed);

	this->lehmer = defaultSeed;
}

// constructor setting the algorithm
Rand::Rand(Algo algo) : Rand() {
	this->setAlgo(algo);
}

// destructor stub
Rand::~Rand() {}

// set the seed ONLY for the currently selected algorithm
void Rand::seed(unsigned int s) {
	switch(this->algo) {
	case RAND_ALGO_STD_RAND:
		std::srand(s);

		break;

	case RAND_ALGO_STD_MT19937:
		this->mt.seed(s);

		break;

	case RAND_ALGO_LEHMER32:
		this->lehmer = s;

		break;
	}
}

// set the current algorithm
void Rand::setAlgo(Algo value) {
	if(value >= RAND_ALGO_NUM)
		throw std::runtime_error("Invalid pseudo-random generation algorithm: " + std::to_string(value));

	this->algo = value;
}

// get the current algorithm
Rand::Algo Rand::getAlgo() const {
	return this->algo;
}

// get the name of the current algorithm
std::string Rand::str() const {
	switch(this->algo) {
	case RAND_ALGO_STD_RAND:
		return "std::rand";

	case RAND_ALGO_STD_MT19937:
		return "std::mt19937";

	case RAND_ALGO_LEHMER32:
		return "lehmer32";
	}

	return "undefined";
}

// set the limits for pseudo-random byte creation (default: 0 to std::numeric_limits<unsigned char>::max())
void Rand::setByteLimits(unsigned char from, unsigned char to) {
	if(from > to) {
		if(this->byteMin == from && this->byteMax == to)
			return;

		this->byteMin = to;
		this->byteMax = from;
	}
	else {
		if(this->byteMin == to && this->byteMax == from)
			return;

		this->byteMin = from;
		this->byteMax = to;
	}

	this->byteHalf = (this->byteMax - this->byteMin) / 2;

	std::uniform_int_distribution<unsigned char> newByteDist(this->byteMin, this->byteMax);

	std::swap(this->byteDist, newByteDist);
}

// set the limits for pseudo-random integer creation (default: 0 to std::numeric_limits<int>::max())
void Rand::setIntLimits(int from, int to) {
	if(from > to) {
		if(this->intMin == from && this->intMax == to)
			return;

		this->intMin = to;
		this->intMax = from;
	}
	else {
		if(this->intMin == to && this->intMax == from)
			return;

		this->intMin = from;
		this->intMax = to;
	}

	this->intHalf = (this->intMax - this->intMin) / 2;

	std::uniform_int_distribution<int> newIntDist(this->intMin, this->intMax);

	std::swap(this->intDist, newIntDist);
}

// set the limits for pseudo-random float creation (default: 0 to 1)
void Rand::setRealLimits(double from, double to) {
	if(from > to) {
		if(this->realMin == from && this->realMax == to)
			return;

		this->realMin = to;
		this->realMax = from;
	}
	else {
		if(this->realMin == to && this->realMax == from)
			return;

		this->realMin = from;
		this->realMax = to;
	}

	this->realHalf = (this->realMax - this->realMin) / 2;

	std::uniform_real_distribution<double> newRealDist(this->realMin, this->realMax);

	std::swap(this->realDist, newRealDist);
}

// generate a pseudo-random byte
unsigned char Rand::generateByte() {
	switch(this->algo) {
	case RAND_ALGO_STD_RAND:
		return this->byteMin + std::rand() % (this->byteMax + 1 - this->byteMin);

	case RAND_ALGO_STD_MT19937:
		return this->byteDist(this->mt);

	case RAND_ALGO_LEHMER32:
		return this->byteMin + this->lehmer32() % (this->byteMax + 1 - this->byteMin);
	}

	return 0;
}

// generate a pseudo-random integer
int Rand::generateInt() {
	switch(this->algo) {
	case RAND_ALGO_STD_RAND:
		return this->intMin + std::rand() % (this->intMax + 1 - this->intMin);

	case RAND_ALGO_STD_MT19937:
		return this->intDist(this->mt);

	case RAND_ALGO_LEHMER32:
		return this->intMin + this->lehmer32() % (this->intMax + 1 - this->intMin);
	}

	return 0;
}

// generate a pseudo-random real number
double Rand::generateReal() {
	switch(this->algo) {
	case RAND_ALGO_STD_RAND:
		return this->realMin + static_cast<double>(std::rand()) / RAND_MAX * (this->realMax - this->realMin);

	case RAND_ALGO_STD_MT19937:
		return this->realDist(this->mt);

	case RAND_ALGO_LEHMER32:
		return this->realMin + static_cast<double>(this->lehmer32()) / 0xFFFFFFFF * (this->realMax - this->realMin);
	}

	return 0;
}

// generate a pseudo-random boolean value
bool Rand::generateBool() {
	switch(this->algo) {
	case RAND_ALGO_STD_RAND:
		return std::rand() > RAND_MAX / 2;

	case RAND_ALGO_STD_MT19937:
		return this->byteDist(this->mt) - this->byteMin > this->byteHalf;

	case RAND_ALGO_LEHMER32:
		return this->lehmer32() > 0xFFFFFFFF / 2;
	}

	return false;
}

// 32-bit Lehmer generator
uint32_t Rand::lehmer32() {
	this->lehmer += 0xe120fc15;

	uint64_t tmp = static_cast<uint64_t>(this->lehmer) * 0x4a39b70d;
	uint32_t m1 = (tmp >> 32) ^ tmp;

	tmp = static_cast<uint64_t>(m1) * 0x12fad5c9;

	uint32_t m2 = (tmp >> 32) ^ tmp;

	return m2;
}
