#pragma once

namespace polar::support::phys::detector {
	class base {
	  public:
		Point3 size{1};

		base() = default;
		base(Point3 size) : size(size) {}

		virtual ~base() = default;
	};
} // namespace polar::support::phys::detector
