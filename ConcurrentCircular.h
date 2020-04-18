/*
 * ConcurrentCircular.h
 *
 *  Created on: Apr 15, 2020
 *      Author: ans
 */

#ifndef CONCURRENTCIRCULAR_H_
#define CONCURRENTCIRCULAR_H_

#pragma once

#include <algorithm>	// std::swap
#include <atomic>		// std::atomic
#include <cstddef>		// std::size_t
#include <memory>		// std::allocator
#include <stdexcept>	// std::logic_error
#include <vector>		// std::vector

// a simple thread-safe, yet non-locking, circular FIFO buffer with a fixed size
//	NOTE:	Thread-safe for TWO threads, one reader and one writer only.
//			The Container class used for passing multiple values needs to provide
//			the member functions ::insert(it, value), ::size(), and ::swap(ref).
//			The element class T needs a constructor without arguments (or with default values).
template<
		typename T,
		typename SizeType = std::size_t,
		template<typename, typename = std::allocator<T>> class Container = std::vector
		>
class ConcurrentCircular {
public:
	// constructor to create a circular buffer with the specified size
	ConcurrentCircular(SizeType size)
			: buffer(nullptr), bufferSize(0), readHead(0), writeHead(0), isEmpty(true) {
		if(size < 2)
			throw std::logic_error("Size of circular buffer must be at least 2");

		this->buffer = new T[size];

		if(this->buffer)
			this->bufferSize = size;
	}

	// destructor deallocating the memory used
	virtual ~ConcurrentCircular() {
		if(this->buffer) {
			delete[] this->buffer;

			this->buffer = nullptr;
		}
	}

	// return whether the buffer is empty
	bool empty() const {
		return this->isEmpty.load()
				&& this->readHead.load() == this->writeHead.load();
	}

	// return whether the buffer is full
	bool full() const {
		return this->readHead.load() == this->writeHead.load()
				&& !(this->isEmpty.load());
	}

	// return the size of the circular buffer
	SizeType capacity() const {
		return this->bufferSize;
	}

	// return the number of unread elements in the buffer
	SizeType size() const {
		SizeType currentReadHead = 0;
		SizeType currentWriteHead = 0;

		switch(this->getState(currentReadHead, currentWriteHead)) {
		case STATE_EMPTY:
			// empty buffer: nothing can be read
			return 0;

		case STATE_FULL:
			// full buffer: everything can be read
			return this->bufferSize;

		case STATE_READ_BEFORE_WRITE:
			// only linear read possible
			return currentWriteHead - currentReadHead;

		case STATE_WRITE_BEFORE_READ:
			// circular read possible
			return currentWriteHead + this->bufferSize - currentReadHead;
		}

		return 0;
	}

	// read one element if available, return whether one was available
	bool pop(T& out) {
		SizeType currentReadHead = 0;
		SizeType currentWriteHead = 0;

		if(this->getState(currentReadHead, currentWriteHead) == STATE_EMPTY)
			// no element to read
			return false;

		// read next element, i.e. element at the read head
		using std::swap;

		swap(out, this->buffer[currentReadHead]);

		// check whether the read head would be moved to the end of the buffer
		++currentReadHead;

		if(currentReadHead == this->bufferSize)
			currentReadHead = 0;

		// check whether the buffer will be empty
		if(currentReadHead == currentWriteHead)
			this->isEmpty.store(true);

		// update read head
		this->readHead.store(currentReadHead);

		return true;
	}

	// read all or up to max available elements, give back an empty container if no elements were available
	void pop(Container<T>& out, SizeType max = 0 /* zero means infinite */) {
		SizeType currentReadHead = 0;
		SizeType currentWriteHead = 0;
		State currentState = this->getState(currentReadHead, currentWriteHead);

		/*
		 * NOTE:	The following variables will store how many element can be read
		 * 			(a) linear (after the position of the read head),
		 * 			(b) circular (at the beginning of the buffer after having reached its end)
		 */
		SizeType readLinear = 0;
		SizeType readCircular = 0;

		switch(currentState) {
		case STATE_EMPTY:
			// no elements can be read from an empty buffer
			Container<T>().swap(out);

			return;

		case STATE_FULL:
			// elements can be read from the read head to the end of the buffer
			//	AND from the beginning of the buffer to one before the read head
			readLinear = this->bufferSize - currentReadHead;
			readCircular = currentReadHead;

			break;

		case STATE_READ_BEFORE_WRITE:
			// elements can be read from the read head to one before the write head
			readLinear = currentWriteHead - currentReadHead;

			break;

		case STATE_WRITE_BEFORE_READ:
			// elements can be read from the read head to the end of the buffer
			//	AND from the beginning of the buffer to one before the write head
			readLinear = this->bufferSize - currentReadHead;
			readCircular = currentWriteHead;

			break;
		}

		// make sure the maximum of elements to read is respected
		bool readAll = true;

		if(max) {
			if(readLinear > max) {
				readLinear = max;

				readCircular = 0;

				readAll = false;
			}
			else if(readLinear + readCircular > max) {
				readCircular = max - readLinear;

				readAll = false;
			}
		}

		// reserve memory for the result if possible
		Container<T> result;

		reserve(result, readLinear + readCircular);

		// read from the read head towards the end of the buffer
		T * readFrom = this->buffer + currentReadHead;

		result.insert(result.end(), readFrom, readFrom + readLinear);

		// read from the beginning of the buffer towards the write head
		result.insert(result.end(), this->buffer, this->buffer + readCircular);

		// check whether the read head would be moved to the end or beyond the end of the buffer
		currentReadHead += readLinear + readCircular;

		if(currentReadHead >= this->bufferSize)
			currentReadHead -= this->bufferSize;

		// set the buffer to empty if applicable
		if(readAll)
			this->isEmpty.store(true);

		// update read head
		this->readHead.store(currentReadHead);

		// swap out the result
		using std::swap;

		swap(result, out);
	}

	// write one element if possible, return whether there was enough space in the buffer
	//	NOTE:	The referenced element will be SWAPPED INTO the buffer and therefore not usable afterwards !
	bool push(T& in) {
		if(!(this->bufferSize))
			throw std::out_of_range("Cannot read from empty circular buffer");

		SizeType currentReadHead = 0;
		SizeType currentWriteHead = 0;

		State currentState = this->getState(currentReadHead, currentWriteHead);

		if(currentState == STATE_FULL)
			// no space to write
			return false;

		// overwrite next element, i.e. element at current write head
		using std::swap;

		swap(this->buffer[currentWriteHead], in);

		// check whether the write head would be moved to the end of the buffer
		++currentWriteHead;

		if(currentWriteHead == this->bufferSize)
			currentWriteHead = 0;

		// update write head
		this->writeHead.store(currentWriteHead);

		// check whether buffer was empty
		if(currentState == STATE_EMPTY)
			this->isEmpty.store(false);

		return true;
	}

	// write specified elements if possible, return the number of elements that could be written
	//	NOTE:	The elements INSIDE the container will be SWAPPED INTO the buffer and therefore not usable afterwards !
	SizeType push(Container<T>& in) {
		if(!(this->bufferSize) || in.empty())
			return 0;

		SizeType currentReadHead = 0;
		SizeType currentWriteHead = 0;

		State currentState = this->getState(currentReadHead, currentWriteHead);

		/*
		 * NOTE:	The following variables will store how many element can be written
		 * 			(a) linear (after the position of the write head),
		 * 			(b) circular (at the beginning of the buffer after having reached its end)
		 */
		SizeType freeLinear = 0;
		SizeType freeCircular = 0;

		switch(currentState) {
		case STATE_FULL:
			// no elements can be written when the buffer is already full
			return 0;

		case STATE_EMPTY:
			// elements can be written from the write head to the end of the buffer
			//  AND from the beginning of the buffer to one before the write head
			freeLinear = this->bufferSize - currentWriteHead;
			freeCircular = currentWriteHead;

			break;

		case STATE_READ_BEFORE_WRITE:
			// elements can be written from the write head to the end of the buffer
			//  AND from the beginning of the buffer to one before the read head
			freeLinear = this->bufferSize - currentWriteHead;
			freeCircular = currentReadHead;

			break;

		case STATE_WRITE_BEFORE_READ:
			// elements can be written from the write head to one before the read head
			freeLinear = currentReadHead - currentWriteHead;

			break;
		}

		// write from the write head towards the end of the buffer
		SizeType writeLinear = std::min(freeLinear, in.size());

		T * writeTo = this->buffer + writeHead;

		for(unsigned int n = 0; n < writeLinear; ++n) {
			using std::swap;

			swap(writeTo[n], in.data()[n]);
		}

		// write from the beginning of the buffer towards the read head
		SizeType writeCircular = std::min(freeCircular, in.size() - writeLinear);

		T * writeFrom = in.data() + writeLinear;

		for(unsigned int n = 0; n < writeCircular; ++n) {
			using std::swap;

			swap(this->buffer[n], writeFrom[n]);
		}

		// check whether the write heand would be moved to the end or beyond the end of the buffer
		currentWriteHead += writeLinear + writeCircular;

		if(currentWriteHead >= this->bufferSize)
			// flip the write head around the end of the buffer
			currentWriteHead -= this->bufferSize;

		// update the write head
		this->writeHead.store(currentWriteHead);

		// check whether the buffer was empty
		if(currentState == STATE_EMPTY && (writeLinear || writeCircular))
			// set the buffer to non-empty
			this->isEmpty.store(false);

		// return the number of actually written elements
		return writeLinear + writeCircular;
	}

	// clear the buffer and free its memory (cannot be used anymore afterwards)
	//	NOTE:	This method is not thread-safe and should only be used when both
	//			the reading and the writing thread have finished using the buffer !
	void clear() {
		// set the buffer size to zero
		this->bufferSize = 0;

		// free allocated memory
		if(this->buffer) {
			delete[] buffer;

			this->buffer = nullptr;
		}

		// reset the state of the buffer
		this->isEmpty.store(true);
		this->readHead.store(0);
		this->writeHead.store(0);
	}

	// copy constructor
	ConcurrentCircular(const ConcurrentCircular& other) {
		this->buffer = nullptr;

		this->buffer = new T[other.bufferSize];
		this->bufferSize = other.bufferSize;

		for(SizeType n = 0; n < this->bufferSize; ++n)
			this->buffer[n] = other.buffer[n];

		this->isFull.store(other.isFull.load());
		this->readHead.store(other.readHead.load());
		this->writeHead.store(other.writeHead.load());
	}

	// copy operator
	ConcurrentCircular& operator=(const ConcurrentCircular& other) {
		this->buffer = nullptr;

		this->buffer = new T[other.bufferSize];
		this->bufferSize = other.bufferSize;

		for(SizeType n = 0; n < this->bufferSize; ++n)
			this->buffer[n] = other.buffer[n];

		this->isFull.store(other.isFull.load());
		this->readHead.store(other.readHead.load());
		this->writeHead.store(other.writeHead.load());

		return *this;
	}

	// the buffer is NOT moveable !
	ConcurrentCircular(ConcurrentCircular&&) = delete;
	ConcurrentCircular& operator=(ConcurrentCircular&&) = delete;

private:
	// the four possible states of the circular buffer
	enum State {
		STATE_EMPTY = 0,				// the buffer is empty
		STATE_READ_BEFORE_WRITE = 1,	// the read head is before the write head (linear read, circular write)
		STATE_WRITE_BEFORE_READ = 2,	// the write head is before the read head (circular read, linear write)
		STATE_FULL = 3				// the buffer is full
	};

	// get the current state of the buffer
	State getState(SizeType& readHeadOut, SizeType& writeHeadOut) const {
		readHeadOut = this->readHead.load();
		writeHeadOut = this->writeHead.load();

		if(readHeadOut == writeHeadOut || !(this->bufferSize))
			return this->isEmpty.load() ? STATE_EMPTY : STATE_FULL;

		if(readHeadOut < writeHeadOut)
			return STATE_READ_BEFORE_WRITE;

		return STATE_WRITE_BEFORE_READ;
	}

	// optional reserve function for the container
	static auto reserve(Container<T>& c, SizeType n)
	-> decltype(c.reserve(n), void()) {
		return c.reserve(n);
	}

	// pointer to and size of the allocated memory
	T * buffer;
	SizeType bufferSize;

	// atomic reading/writing state
	std::atomic<SizeType> readHead;
	std::atomic<SizeType> writeHead;
	std::atomic<bool> isEmpty;
};

#endif /* CONCURRENTCIRCULAR_H_ */
