#pragma once

#include <polar/component/audiosource.h>

namespace polar {
namespace support {
	namespace audio {
		namespace channel {
			enum class messagetype { add, remove };

			struct message {
				using audiosource = component::audiosource;

				static message add(IDType id,
				                   std::shared_ptr<audiosource> source) {
					return message{messagetype::add, id, source};
				}
				static message remove(IDType id) {
					return message{messagetype::remove, id, nullptr};
				}

				messagetype type;
				IDType id;
				std::shared_ptr<audiosource> source;
			};
		}
	}
}
}
