#pragma once

#include <polar/component/audiosource.h>
#include <polar/core/ref.h>

namespace polar::support::audio::channel {
	enum class messagetype { add, remove };

	struct message {
		using audiosource = component::audiosource;

		static auto add(core::weak_ref object, std::shared_ptr<audiosource> source) {
			return message{messagetype::add, object, source};
		}
		static auto remove(core::weak_ref object) {
			return message{messagetype::remove, object, nullptr};
		}

		messagetype type;
		core::weak_ref object;
		std::shared_ptr<audiosource> source;
	};
} // namespace polar::support::audio::channel
