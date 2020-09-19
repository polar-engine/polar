#pragma once

#include <polar/asset/shaderprogram.h>
#include <polar/component/base.h>
#include <polar/core/ref.h>

namespace polar::component {
	COMPONENT_BEGIN(stage)
		struct shader {
			using shadertype = support::shader::shadertype;

			shadertype type;
			std::string source;

			shader() = default;
			shader(shadertype type, std::string source) : type(type), source(source) {}

			friend inline core::store_serializer &operator<<(core::store_serializer &s, const shader &sh) {
				return s << static_cast<uint8_t>(sh.type) << sh.source;
			}

			friend inline core::store_deserializer &operator>>(core::store_deserializer &s, shader &sh) {
				uint8_t type;
				s >> type;
				sh.type = static_cast<shadertype>(type);

				return s >> sh.source;
			}
		};

		core::ref win;
		std::vector<shader> shaders;

		stage(core::ref win) : win(win) {}
		stage(core::ref win, std::shared_ptr<asset::shaderprogram> as) : win(win) {
			for(auto &sh : as->shaders) {
				shaders.emplace_back(sh.type, sh.source);
			}
		}

		bool serialize(core::store_serializer &s) const override {
			s << win << shaders;
			return true;
		}

		static std::shared_ptr<stage> deserialize(core::store_deserializer &s) {
			core::ref win;
			s >> win;

			auto c = std::make_shared<stage>(win);
			s >> c->shaders;
			return c;
		}
	COMPONENT_END(stage, stage)
} // namespace polar::component
