#pragma once

#include <vector>
#include "Component.h"

struct BoundingBox {
	Point3 position;
	Point3 size;
	std::vector<BoundingBox> children;
	BoundingBox() {}
	BoundingBox(const Point3 &position, const Point3 &size) : position(position), size(size) {}

	bool CollidesWith(const BoundingBox &b, const Point3 &ownPos, const Point3 &bPos) const {
		auto aBegin = ownPos + position;
		auto aEnd = aBegin + size;
		auto bBegin = bPos + b.position;
		auto bEnd = bBegin + b.size;

		/* false if root boxes do not collide */
		if(!(
			aBegin.x < bEnd.x &&
			aBegin.y < bEnd.y &&
			aBegin.z < bEnd.z &&
			bBegin.x < aEnd.x &&
			bBegin.y < aEnd.y &&
			bBegin.z < aEnd.z)) {
			return false;
		}

		/* false if have children and none collide with b */
		/*if(!children.empty()) {
			auto result = false;
			for(auto &child : children) {
				result |= child.CollidesWith(b, ownPos, bPos);
			}
			return result;
		}*/

		/* false if b has children and none collide with self */
		if(!b.children.empty()) {
			auto result = false;
			for(auto &child : b.children) {
				result |= CollidesWith(child, ownPos, bPos);
			}
			return result;
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