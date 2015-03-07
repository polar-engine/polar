#pragma once

#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/index/rtree.hpp>
#include "System.h"

class SpatialIndex : public System {
private:
	//boost::geometry::index::rtree<std::pair<BoundsComponent *, Object *>, boost::geometry::index::rstar<16>> index;
protected:
public:
	static bool IsSupported() { return true; }
	SpatialIndex(Polar *engine) : System(engine) {}
};
