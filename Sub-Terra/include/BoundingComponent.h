#pragma once

#include <vector>
#include "Component.h"

struct BoundingBox {
	Point3 position;
	Point3 size;
	std::vector<BoundingBox> children;

	BoundingBox() {}
	BoundingBox(const Point3 &position, const Point3 &size) : position(position), size(size) {}

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

	bool AABBSwept(const BoundingBox &b, const std::pair<Point3, Point3> &ownPoses, const Point3 &bPos) const {
		auto &ownPos = std::get<0>(ownPoses);
		auto &ownPos2 = std::get<1>(ownPoses);
		auto &ownVel = ownPos2 - ownPos;

		BoundingBox broadphase(
			Point3(
				ownVel.x > 0 ? ownPos.x : ownPos2.x,
				ownVel.y > 0 ? ownPos.y : ownPos2.y,
				ownVel.z > 0 ? ownPos.z : ownPos2.z
			),
			Point3(
				ownVel.x > 0 ? size.x + ownVel.x : size.x - ownVel.x,
				ownVel.y > 0 ? size.y + ownVel.y : size.y - ownVel.y,
				ownVel.z > 0 ? size.z + ownVel.z : size.z - ownVel.z
			)
		);

		if(!broadphase.AABBCheck(b, position, bPos)) { return false; }

		/* if has children, OR results and return immediately */
		if(!b.children.empty()) {
			auto result = false;
			for(auto &child : b.children) {
				result |= AABBSwept(child, ownPoses, bPos);
			}
			return result;
		}

		auto aBegin = position + ownPos;
		auto aEnd = aBegin + size;
		auto bBegin = bPos + b.position;
		auto bEnd = bBegin + b.size;

		/* find distance between self and b on near and far sides of all three axes */

		Point3 inverseEntry;
		Point3 inverseExit;

		if(ownVel.x > 0.0f) {
			inverseEntry.x = bBegin.x - aEnd.x;
			inverseExit.x = bEnd.x - aBegin.x;
		} else {
			inverseEntry.x = bEnd.x - aBegin.x;
			inverseExit.x = bBegin.x - aEnd.x;
		}

		if(ownVel.y > 0.0f) {
			inverseEntry.y = bBegin.y - aEnd.y;
			inverseExit.y = bEnd.y - aBegin.y;
		} else {
			inverseEntry.y = bEnd.y - aBegin.y;
			inverseExit.y = bBegin.y - aEnd.y;
		}

		if(ownVel.z > 0.0f) {
			inverseEntry.z = bBegin.z - aEnd.z;
			inverseExit.z = bEnd.z - aBegin.z;
		} else {
			inverseEntry.z = bEnd.z - aBegin.z;
			inverseExit.z = bBegin.z - aEnd.z;
		}

		/* find time of collision and time of exit for all three axes
		 * prevent division by zero by setting entry time to minumum infinity and exit time to maximum infinity
		 */

		Point3 entry;
		Point3 exit;

		if(ownVel.x == 0.0f) {
			entry.x = -std::numeric_limits<float>::infinity();
			exit.x = std::numeric_limits<float>::infinity();
		} else {
			entry.x = inverseEntry.x / ownVel.x;
			exit.x = inverseExit.x / ownVel.x;
		}

		if(ownVel.y == 0.0f) {
			entry.y = -std::numeric_limits<float>::infinity();
			exit.y = std::numeric_limits<float>::infinity();
		} else {
			entry.y = inverseEntry.y / ownVel.y;
			exit.y = inverseExit.y / ownVel.y;
		}

		if(ownVel.z == 0.0f) {
			entry.z = -std::numeric_limits<float>::infinity();
			exit.z = std::numeric_limits<float>::infinity();
		} else {
			entry.z = inverseEntry.z / ownVel.z;
			exit.z = inverseExit.z / ownVel.z;
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
			return false;
		}

		/* true once all conditions are met */
		return true;
	}
};

class BoundingComponent : public Component {
public:
	BoundingBox box;
	BoundingComponent() {}
	BoundingComponent(const Point3 &position, const Point3 &size) : box(position, size) {}
};