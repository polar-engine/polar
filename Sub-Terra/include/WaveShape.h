#pragma once

#include <boost/container/vector.hpp>

struct WaveShape {
	double preferredFrequency;
	boost::container::vector<double> buffer;
};
