#pragma once

#include <boost/container/vector.hpp>

struct WaveShape {
	double preferredFrequency;
	boost::container::vector<uint16_t> buffer;
};
