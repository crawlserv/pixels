/*
 * Geometry.h
 *
 *  Created on: Apr 10, 2020
 *      Author: ans
 */

#ifndef GEOMETRY_H_
#define GEOMETRY_H_

#include <deque>		// std::deque
#include <vector>		// std::vector

namespace Geometry {
	// generic one-colored rectangle
	template<typename T, typename C>
	struct Rectangle {
		T x1;
		T y1;
		T x2;
		T y2;
		C c;

		Rectangle() : x1(0), y1(0), x2(0), y2(0), c() {}
		Rectangle(T _x1, T _y1, T _x2, T _y2, C _c) : x1(_x1), y1(_y1), x2(_x2), y2(_y2), c(_c) {}

		T w() const { return x2 - x1; }
		T h() const { return y2 - y1; }
	};

	// add rectangle if it has a certain minimum size
	template<typename T, typename C>
	inline void addIfLarger(std::vector<Rectangle<T, C>>& rects, T x1, T y1, T x2, T y2, C c, T min) {
		if(x2 - x1 >= min && y2 - y1 >= min)
			rects.emplace_back(x1, y1, x2, y2, c);
	}

	// add rectangle if it has a certain minimum size
	template<typename T, typename C>
	inline void addIfLarger(std::vector<Rectangle<T, C>>& rects, const Rectangle<T, C>& newRect, T min) {
		if(newRect.w() >= min && newRect.h() >= min)
			rects.emplace_back(newRect);
	}

	// check whether two rectangles overlap
	template<typename T, typename C>
	inline bool overlap(const Rectangle<T, C>& r1, const Rectangle<T, C>& r2) {
		return ((r1.x1 > r2.x1 && r1.x1 < r2.x2)		// is r1.x1 inside [r2.x1; r2.x2]?
				|| (r1.x2 > r2.x1 && r1.x2 < r2.x2)		// is r1.x2 inside [r2.x1; r2.x2]?
				|| (r2.x1 >= r1.x1 && r2.x2 <= r1.x2))	// is [r2.x1; r2.x2] inside [r1.x1; r1.x2]?
				&& ((r1.y1 > r2.y1 && r1.y1 < r2.y2)	// is r1.y1 inside [r2.y1; r2.y2]?
				|| (r1.y2 > r2.y1 && r1.y2 < r2.y2)		// is r1.y2 inside [r2.y1; r2.y2]?
				|| (r2.y1 >= r1.y1 && r2.y2 <= r1.y2));	// is [r2.y1; r2.y2] inside [r1.y1; r1.y2]?
	}

	// add a rectangle and split the clipped rectangles removing the overlapped parts
	template<typename T, typename C>
	inline void addAndSplit(std::vector<Rectangle<T, C>>& rects, const Rectangle<T, C>& newRect, T min = 0) {
		const size_t oldSize = rects.size();
		std::deque<size_t> toDelete;

		for(size_t index = 0; index < oldSize; ++index) {
			// get reference to current rect
			const auto& ref = rects.at(index);

			// check whether new rectangle overlaps with the current rectangle
			if(overlap(ref, newRect)) {
				// queue rectangle for deletion
				toDelete.push_back(index);

				/*
				 * ALLOCATE ENOUGH MEMORY TO NOT INVALIDATE THE REFERENCE WHILE ADDING RECTANGLES
				 * 	(and get a new reference after that)
				 */
				rects.reserve(rects.size() + 4);

				const auto& ref = rects.at(index);

				/*
				 * CHECK FOR ENCOMPASSING
				 *
				 * if the new rectangle encompasses the existing one (on X AND on Y axis),
				 *  no additional rectangles will be added
				 *
				 *
				 */
				bool x_begin_before = newRect.x1 < ref.x1;
				bool x_end_after = newRect.x2 > ref.x2;
				bool y_begin_before = newRect.y1 < ref.y1;
				bool y_end_after = newRect.y2 > ref.y2;

				if(x_begin_before && x_end_after && y_begin_before && y_end_after)
					continue;

				/*
				 * CHECK X AXIS
				 */
				T x_before = 0;
				T x_after = 0;

				if(x_begin_before && !x_end_after)
					// new rectangle begins before old rectangle on X axis
					x_after = ref.x2;
				else if(x_end_after && !x_begin_before)
					// new rectangle ends after old rectangle on X axis
					x_before = ref.x1;
				else if(!x_begin_before && !x_end_after) {
					// new rectangle is inside old rectangle on X axis
					x_before = ref.x1;
					x_after = ref.x2;
				}

				/*
				 * CHECK Y AXIS
				 */
				T y_before = 0;
				T y_after = 0;

				if(y_begin_before && !y_end_after)
					// new rectangle begins before old rectangle on Y axis
					y_after = ref.y2;
				else if(y_end_after && !y_begin_before)
					// new rectangle ends after old rectangle on Y axis
					y_before = ref.y1;
				else if(!y_begin_before && !y_end_after) {
					// new rectangle is inside old rectangle on Y axis
					y_before = ref.y1;
					y_after = ref.y2;
				}

				/*
				 * ADD ADDITIONAL RECTANGLES
				 */

				if(x_before) {
					if(y_before && y_after)
						// combine the Y_BEFORE, Y_MIDDLE and Y_AFTER parts of X_BEFORE
						addIfLarger(rects, x_before, y_before, newRect.x1, y_after, ref.c, min);
					else if(y_before)
						// combine the Y_BEFORE and Y_MIDDLE parts of X_BEFORE
						addIfLarger(rects, x_before, y_before, newRect.x1, ref.y2, ref.c, min);
					else if(y_after)
						// combine the Y_MIDDLE and Y_AFTER parts of X_BEFORE
						addIfLarger(rects, x_before, ref.y1, newRect.x1, y_after, ref.c, min);
					else
						// only the Y_MIDDLE part of X_BEFORE
						addIfLarger(rects, x_before, ref.y1, newRect.x1, ref.y2, ref.c, min);
				}

				if(x_after) {
					if(y_before && y_after)
						// combine the Y_BEFORE, Y_MIDDLE and Y_AFTER parts of X_AFTER
						addIfLarger(rects, newRect.x2, y_before, x_after, y_after, ref.c, min);
					else if(y_before)
						// combine the Y_BEFORE and Y_MIDDLE parts of X_AFTER
						addIfLarger(rects, newRect.x2, y_before, x_after, ref.y2, ref.c, min);
					else if(y_after)
						// combine the Y_MIDDLE and Y_AFTER parts of X_AFTER
						addIfLarger(rects, newRect.x2, ref.y1, x_after, y_after, ref.c, min);
					else
						// only the Y_MIDDLE part of X_AFTER
						addIfLarger(rects, newRect.x2, ref.y1, x_after, ref.y2, ref.c, min);
				}

				if(y_before) {
					// the X_MIDDLE part of Y_BEFORE
					if(x_begin_before) {
						if(x_end_after)
							addIfLarger(rects, ref.x1, y_before, ref.x2, newRect.y1, ref.c, min);
						else
							addIfLarger(rects, ref.x1, y_before, newRect.x2, newRect.y1, ref.c, min);
					}
					else {
						if(x_end_after)
							addIfLarger(rects, newRect.x1, y_before, ref.x2, newRect.y1, ref.c, min);
						else
							addIfLarger(rects, newRect.x1, y_before, newRect.x2, newRect.y1, ref.c, min);
					}
				}

				if(y_after) {
					// the X_MIDDLE part of Y_AFTER
					if(x_begin_before) {
						if(x_end_after)
							addIfLarger(rects, ref.x1, newRect.y2, ref.x2, y_after, ref.c, min);
						else
							addIfLarger(rects, ref.x1, newRect.y2, newRect.x2, y_after, ref.c, min);
					}
					else {
						if(x_end_after)
							addIfLarger(rects, newRect.x1, newRect.y2, ref.x2, y_after, ref.c, min);
						else
							addIfLarger(rects, newRect.x1, newRect.y2, newRect.x2, y_after, ref.c, min);
					}
				}
			}
		}

		// delete clipped and overdrawn rectangles
		while(!toDelete.empty()) {
			rects.erase(rects.begin() + toDelete.back());

			toDelete.pop_back();
		}

		// add the new rectangle
		addIfLarger(rects, newRect, min);
	}
}

#endif /* GEOMETRY_H_ */
