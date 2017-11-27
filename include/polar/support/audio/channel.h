#pragma once

#include <polar/component/audiosource.h>

namespace polar::support::audio::channel {
	enum class messagetype { add, remove };

	struct message {
		using audiosource = component::audiosource;

		static auto add(IDType id, std::shared_ptr<audiosource> source) {
			return message{messagetype::add, id, source};
		}
		static auto remove(IDType id) {
			return message{messagetype::remove, id, nullptr};
		}

		messagetype type;
		IDType id;
		std::shared_ptr<audiosource> source;
	};
} // namespace polar::support::audio::channel
