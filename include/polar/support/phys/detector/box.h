#pragma once

#include <polar/support/phys/detector/base.h>

namespace polar {
namespace support {
	namespace phys {
		namespace detector {
			class box : public base {
			  public:
				Point3 size;
			};
		}
	}
}
}
