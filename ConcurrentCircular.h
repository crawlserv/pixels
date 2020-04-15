/*
 * ConcurrentCircular.h
 *
 *  Created on: Apr 15, 2020
 *      Author: ans
 */

#ifndef CONCURRENTCIRCULAR_H_
#define CONCURRENTCIRCULAR_H_

#include <atomic>		// std::atomic
#include <cstddef>		// std::size_t
#include <memory>		// std::allocator
#include <stdexcept>	// std::logic_error
#include <vector>		// std::vector

// a simple thread-safe, yet non-locking, circular FIFO buffer with a fixed size
//	NOTE:	The Container class used for passing multiple values needs ::insert(it, value) and ::size().
//			The element class T needs a constructor without arguments (or with default values).
template<
		typename T,
		typename SizeType = std::size_t,
		template<typename, typename = std::allocator<T>> class Container = std::vector
		>
class ConcurrentCircular {
public:
	ConcurrentCircular(SizeType size)
			: buffer(new T[size]), bufferSize(size), readHead(0), writeHead(0), isEmpty(true) {
		if(this->bufferSize < 2)
			throw std::logic_error("Size of circular buffer must be at least 2");
	}

	virtual ~ConcurrentCircular() {
		if(this->buffer) {
			delete[] this->buffer;

			this->buffer = nullptr;
		}
	}

	// return whether the buffer is empty
	bool empty() const {
		return this->isEmpty.load() && this->readHead.load() == this->writeHead.load();
	}

	// return whether the buffer is full
	bool full() const {
		return this->readHead.load() == this->writeHead.load() && !(this->isEmpty.load());
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
	bool read(T& out) {
		SizeType currentReadHead = 0;
		SizeType currentWriteHead = 0;

		if(this->getState(currentReadHead, currentWriteHead) == STATE_EMPTY)
			// no element to read
			return false;

		// read next element, i.e. element at the read head
		out = this->buffer[currentReadHead];

		// check whether read head would be moved at the end
		++currentReadHead;

		if(currentReadHead == this->bufferSize)
			currentReadHead = 0;

		// check whether buffer will be empty
		if(currentReadHead == currentWriteHead)
			this->isEmpty.store(true);

		// update read head
		this->readHead.store(currentReadHead);

		return true;
	}

	// read all available elements, return an empty container if none were available
	Container<T> read() {
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
			return Container<T>();

		case STATE_FULL:
			// elements can be read from the read head to the end of the buffer
			//	AND from the beginning of the buffer to one before the read head
			readLinear = this->bufferSize - currentReadHead;
			readCircular = currentReadHead;

			break;

		case STATE_READ_BEFORE_WRITE:
			// elements can be written from the read head to one before the write head
			readLinear = currentWriteHead - currentReadHead;

			break;

		case STATE_WRITE_BEFORE_READ:
			// elements can be read from the read head to the end of the buffer
			//	AND from the beginning of the buffer to one before the write head
			readLinear = this->bufferSize - currentReadHead;
			readCircular = currentWriteHead;

			break;
		}

		Container<T> result;

		reserve(result, readLinear + readCircular);

		// read from the read head towards the end of the buffer
		for(SizeType n = 0; n < readLinear; ++n)
			result.insert(result.end(), this->buffer[currentReadHead + n]);

		// read from the beginning of the buffer towards the write head
		for(SizeType n = 0; n < readCircular; ++n)
			result.insert(result.end(), this->buffer[n]);

		// check whether read head would be moved at or beyond the end
		currentReadHead += readLinear + readCircular;

		if(currentReadHead >= this->bufferSize)
			currentReadHead -= this->bufferSize;

		// set buffer to empty
		this->isEmpty.store(true);

		// update read head
		this->readHead.store(currentReadHead);

		return result;
	}

	// write one element if possible, return whether there was enough space in the buffer
	bool write(const T& in) {
		if(!(this->bufferSize))
			throw std::out_of_range("Cannot read from empty circular buffer");

		SizeType currentReadHead = 0;
		SizeType currentWriteHead = 0;
		State currentState = this->getState(currentReadHead, currentWriteHead);

		if(currentState == STATE_FULL)
			// no space to write
			return false;

		// overwrite next element, i.e. element at current write head
		this->buffer[currentWriteHead] = in;

		// check whether write head would be moved at the end
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
	SizeType write(const Container<T>& in) {
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
		SizeType written = 0;

		for(; written < freeLinear && written < in.size();) {
			this->buffer[writeHead + written] = in[written];

			++written;
		}

		// write from the beginning of the buffer towards the read head
		for(SizeType n = 0; n < freeCircular && written < in.size(); ++n) {
			this->buffer[n] = in[written];

			++written;
		}

		// update write head
		currentWriteHead += written;

		if(currentWriteHead >= this->bufferSize)
			// flip write head around the end of the buffer
			currentWriteHead -= this->bufferSize;

		this->writeHead.store(currentWriteHead);

		// check whether buffer was empty
		if(currentState == STATE_EMPTY && written)
			this->isEmpty.store(false);

		return written;
	}

	// clear the buffer and free its memory (cannot be used anymore afterwards)
	//	NOTE:	This method is not thread-safe and should only be used when both
	//			the reading and the writing thread have finished using the buffer !
	void clear() {
		this->bufferSize = 0;

		if(this->buffer) {
			delete[] buffer;

			this->buffer = nullptr;
		}

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
		// clear current buffer
		this->bufferSize = 0;
		this->readHead.store(0);
		this->writeHead.store(0);
		this->isEmpty.store(true);

		if(this->buffer) {
			delete[] this->buffer;

			this->buffer = nullptr;
		}

		this->buffer = new T[other.bufferSize];

		if(this->buffer) {
			this->bufferSize = other.bufferSize;

			for(SizeType n = 0; n < this->bufferSize; ++n)
				this->buffer[n] = other.buffer[n];

			this->readHead.store(other.readHead.load());
			this->writeHead.store(other.writeHead.load());
			this->isEmpty.store(other.isEmpty.load());
		}

		return *this;
	}

	// not moveable
	ConcurrentCircular(ConcurrentCircular&&) = delete;
	ConcurrentCircular& operator=(ConcurrentCircular&&) = delete;

private:
	// return the state of the circular buffer
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

	// optional reserve function for container
	static auto reserve(Container<T>& c, SizeType n)
	-> decltype(c.reserve(n), void()) {
		return c.reserve(n);
	}

	T * buffer;
	SizeType bufferSize;

	std::atomic<SizeType> readHead;
	std::atomic<SizeType> writeHead;
	std::atomic<bool> isEmpty;
};

#endif /* CONCURRENTCIRCULAR_H_ */