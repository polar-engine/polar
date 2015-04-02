#pragma once

#include <vector>
#include "Component.h"

struct BoundingBox {
	Point3 position;
	Point3 size;
	std::vector<BoundingBox> children;

	BoundingBox() {}
	BoundingBox(const Point3 &position, const Point3 &size) : position(position), size(size) {}

	inline Point3 Center() const { return position + size / 2.0f; }

	inline std::pair<bool, float> TestRay(const Point3 &origin, const Point3 &direction, float tMax, const Point3 &ownPos) const {
		auto tMin = 0.0f;
		auto p = ownPos + Center() - origin;
		auto h = size / 2.0f;

		for(glm::length_t i = 0; i < 3; ++i) {
			if(glm::abs(direction[i]) > 0.00000000000000000001f) {
				float invDir = 1.0f / direction[i];
				float tEntry = (p[i] - h[i]) * invDir;
				float tExit = (p[i] + h[i]) * invDir;
				if(tEntry > tExit) { std::swap(tEntry, tExit); }
				if(tEntry > tMin) { tMin = tEntry; }
				if(tExit < tMax) { tMax = tExit; }
				if(tMin > tMax) { return std::make_pair(false, 0.0f); }
				if(tMax < 0.0f) { return std::make_pair(false, 0.0f); }
			} else if(-p[i] - h[i] > 0.0f || -p[i] + h[i] < 0.0f) { return std::make_pair(false, 0.0f); }
		}

		/* return child with soonest entry time */
		if(!children.empty()) {
			auto result = std::make_pair(false, std::numeric_limits<float>::infinity());
			for(auto &child : children) {
				auto r = child.TestRay(origin, direction, tMax, ownPos);
				if(std::get<0>(r) && std::get<1>(r) < std::get<1>(result)) { result = r; }
			}
			return result;
		}

		return std::make_pair(true, tMin > 0.0f ? tMin : tMax);
	}

	inline bool AABBCheck(const BoundingBox &b, const Point3 &ownPos, const Point3 &bPos) const {
		Point3 aBegin = position + ownPos;
		Point3 bBegin = b.position + bPos;
		auto aEnd = aBegin + size;
		auto bEnd = bBegin + b.size;

		return !(
			aEnd.x < bBegin.x ||
			aEnd.y < bBegin.y ||
			aEnd.z < bBegin.z ||
			aBegin.x > bEnd.x ||
			aBegin.y > bEnd.y ||
			aBegin.z > bEnd.z);
	}

	std::pair<float, Point3> AABBSwept(const BoundingBox &b, const std::tuple<Point3, Point3, Point3> &own, const Point3 &bPos) const {
		std::pair<float, Point3> result = std::make_pair(1.0f, Point3(0.0f));

		auto &ownPos = std::get<0>(own);
		auto &ownPos2 = std::get<1>(own);
		auto &ownVel = std::get<2>(own);

		BoundingBox broadphase(
			Point3(
				ownVel.x > 0.0f ? ownPos.x : ownPos2.x,
				ownVel.y > 0.0f ? ownPos.y : ownPos2.y,
				ownVel.z > 0.0f ? ownPos.z : ownPos2.z
			),
			Point3(
				ownVel.x > 0.0f ? size.x + ownVel.x : size.x - ownVel.x,
				ownVel.y > 0.0f ? size.y + ownVel.y : size.y - ownVel.y,
				ownVel.z > 0.0f ? size.z + ownVel.z : size.z - ownVel.z
			)
		);

		if(!broadphase.AABBCheck(b, position, bPos)) { return result; }

		/* return child with soonest entry time */
		if(!b.children.empty()) {
			for(auto &child : b.children) {
				auto r = AABBSwept(child, own, bPos);
				if(std::get<0>(r) < std::get<0>(result)) { result = r; }
			}
			return result;
		}

		auto aBegin = position + ownPos;
		auto aEnd = aBegin + size;
		auto bBegin = bPos + b.position;
		auto bEnd = bBegin + b.size;

		Point3 inverseEntry;
		Point3 inverseExit;
		Point3 entry;
		Point3 exit;

		for(glm::length_t i = 0; i < 3; ++i) {
			/* find distance between self and b on near and far sides of all three axes */
			if(ownVel[i] > 0.00000000000000000001f) {
				inverseEntry[i] = bBegin[i] - aEnd[i];
				inverseExit[i] = bEnd[i] - aBegin[i];
			} else {
				inverseEntry[i] = bEnd[i] - aBegin[i];
				inverseExit[i] = bBegin[i] - aEnd[i];
			}

			/* find time of collision and time of exit for all three axes
			* prevent division by zero by setting entry time to minumum infinity and exit time to maximum infinity
			*/
			if(ownVel[i] == 0.0f) {
				entry[i] = -std::numeric_limits<float>::infinity();
				exit[i] = std::numeric_limits<float>::infinity();
			} else {
				entry[i] = inverseEntry[i] / ownVel[i];
				exit[i] = inverseExit[i] / ownVel[i];
			}
		}

		/* find time at which all axes have entered b */
		float entryTime = (std::max)({entry.x, entry.y, entry.z});

		/* find time at which any axis has exited b */
		float exitTime = (std::min)({exit.x, exit.y, exit.z});

		/* false if entry time is after exit time
		 * OR if all entry times are less than 0
		 * OR if any entry time is greater than 0
		 */
		if(entryTime > exitTime ||
		   (entry.x < 0.0f && entry.y < 0.0f && entry.z < 0.0f) ||
		   entry.x > 1.0f || entry.y > 1.0f || entry.z > 1.0f) {
			return result;
		}

		std::get<0>(result) = entryTime;

		/* determine normal of collided surface */

		if(entryTime == entry.x) {
			std::get<1>(result) = Point3(inverseEntry.x < 0.0f ? 1.0f : -1.0f, 0.0f, 0.0f);
		} else if(entryTime == entry.y) {
			std::get<1>(result) = Point3(0.0f, inverseEntry.y < 0.0f ? 1.0f : -1.0f, 0.0f);
		} else {
			std::get<1>(result) = Point3(0.0f, 0.0f, inverseEntry.z < 0.0f ? 1.0f : -1.0f);
		}

		return result;
	}
};

class BoundingComponent : public Component {
public:
	BoundingBox box;
	BoundingComponent() {}
	BoundingComponent(const Point3 &position, const Point3 &size) : box(position, size) {}
};