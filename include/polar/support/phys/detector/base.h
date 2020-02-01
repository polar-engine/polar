#pragma once

namespace polar::support::phys::detector {
	class base {
	  public:
		math::point3 size{1};

		base() = default;
		base(math::point3 size) : size(size) {}

		virtual ~base() = default;
	};
} // namespace polar::support::phys::detector
