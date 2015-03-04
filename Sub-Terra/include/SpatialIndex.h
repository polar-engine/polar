#pragma once

#include <boost/geometry/index/rtree.hpp>
#include "System.h"

class SpatialIndex : public System {
private:
	//boost::geometry::index::rtree<std::pair<BoundsComponent *, Object *>, boost::geometry::index::rstar<16>> index;
protected:
	void ObjectAdded(Object *) override final;
	void ObjectRemoved(Object *) override final;
public:
	static bool IsSupported() { return true; }
	SpatialIndex(Polar *engine) : System(engine) {}
};
