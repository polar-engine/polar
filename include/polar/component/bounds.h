#pragma once

#include <polar/component/base.h>
#include <vector>

struct BoundingBox {
	Point3 position;
	Point3 size;
	bool skipRoot = false;
	std::vector<BoundingBox> children;

	BoundingBox() {}
	BoundingBox(const Point3 &position, const Point3 &size, const bool &skipRoot = false) : position(position), size(size), skipRoot(skipRoot) {}

	inline Point3 Center() const {
		return position + size / Decimal(2.0);
	}

	inline std::tuple<bool, Decimal, Point3, Point3> TestRay(const Point3 &origin, const Point3 &direction, Decimal tMax, const Point3 &ownPos) const {
		auto failure = std::make_tuple(false, std::numeric_limits<Decimal>::infinity(), Point3(0.0), Point3(0.0));
		Decimal tMin(0.0);
		auto p = ownPos + Center() - origin;
		auto h = size / Decimal(2.0);
		glm::length_t axis = 0;
		Point3 tEntry;
		Point3 tExit;
		Point3 normal;

		for(glm::length_t i = 0; i < 3; ++i) {
			if(glm::abs(direction[i]) > Decimal(0.00000000000000000001)) {
				Decimal invDir = Decimal(1.0) / direction[i];
				tEntry[i] = (p[i] - h[i]) * invDir;
				tExit[i] = (p[i] + h[i]) * invDir;
				if(tEntry[i] > tExit[i]) { std::swap(tEntry[i], tExit[i]); }
				if(tEntry[i] > tMin) { tMin = tEntry[i]; axis = i; }
				if(tExit[i] < tMax) { tMax = tExit[i]; }
				if(tMin > tMax) { return failure; }
				if(tMax < 0) { return failure; }
			} else if(-p[i] - h[i] > 0 || -p[i] + h[i] < 0) { return failure; }
		}

		/* return child with soonest entry time */
		if(skipRoot) {
			auto result = failure;
			for(auto &child : children) {
				auto r = child.TestRay(origin, direction, tMax, ownPos);
				if(std::get<0>(r) && std::get<1>(r) < std::get<1>(result)) { result = r; }
			}
			return result;
		}

		/* determine normal of collided surface */
		if(axis == 0) {
			normal = glm::normalize(Point3(-direction.x, 0, 0));
		} else if(axis == 1) {
			normal = glm::normalize(Point3(0, -direction.y, 0));
		} else {
			normal = glm::normalize(Point3(0, 0, -direction.z));
		}

		return std::make_tuple(true, tMin > 0.0f ? tMin : tMax, ownPos + position, normal);
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

	std::pair<Decimal, Point3> AABBSwept(const BoundingBox &b, const Point3 &ownPos, const std::tuple<Point3, Point3, Point3> &bTuple) const {
		std::pair<Decimal, Point3> result = std::make_pair(1.0f, Point3(0.0f));

		auto &bPos1 = std::get<0>(bTuple);
		auto &bPos2 = std::get<1>(bTuple);
		auto &bVel = std::get<2>(bTuple);

		BoundingBox broadphase(
			Point3(
				bVel.x > 0.0f ? bPos1.x : bPos2.x,
				bVel.y > 0.0f ? bPos1.y : bPos2.y,
				bVel.z > 0.0f ? bPos1.z : bPos2.z
			),
			Point3(
				bVel.x > 0.0f ? b.size.x + bVel.x : b.size.x - bVel.x,
				bVel.y > 0.0f ? b.size.y + bVel.y : b.size.y - bVel.y,
				bVel.z > 0.0f ? b.size.z + bVel.z : b.size.z - bVel.z
			)
		);

		if(!AABBCheck(broadphase, ownPos, b.position)) { return result; }

		/* return child with soonest entry time */
		if(skipRoot) {
			for(auto &child : children) {
				auto r = child.AABBSwept(b, ownPos, bTuple);
				if(std::get<0>(r) < std::get<0>(result)) { result = r; }
			}
			return result;
		}

		auto aBegin = bPos1 + b.position;
		auto aEnd = aBegin + b.size;
		auto bBegin = position + ownPos;
		auto bEnd = bBegin + size;

		Point3 inverseEntry;
		Point3 inverseExit;
		Point3 entry;
		Point3 exit;

		for(glm::length_t i = 0; i < 3; ++i) {
			/* find distance between self and b on near and far sides of all three axes */
			if(bVel[i] > 0.00000000000000000001f) {
				inverseEntry[i] = bBegin[i] - aEnd[i];
				inverseExit[i] = bEnd[i] - aBegin[i];
			} else {
				inverseEntry[i] = bEnd[i] - aBegin[i];
				inverseExit[i] = bBegin[i] - aEnd[i];
			}

			/* find time of collision and time of exit for all three axes
			* prevent division by zero by setting entry time to minumum infinity and exit time to maximum infinity
			*/
			if(bVel[i] == 0.0f) {
				entry[i] = -std::numeric_limits<Decimal>::infinity();
				exit[i] = std::numeric_limits<Decimal>::infinity();
			} else {
				entry[i] = inverseEntry[i] / bVel[i];
				exit[i] = inverseExit[i] / bVel[i];
			}
		}

		/* find time at which all axes have entered b */
		Decimal entryTime = std::max({entry.x, entry.y, entry.z});

		/* find time at which any axis has exited b */
		Decimal exitTime = std::min({exit.x, exit.y, exit.z});

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
	BoundingComponent(const Point3 &position, const Point3 &size, const bool &skipRoot = false) : box(position, size, skipRoot) {}
};
