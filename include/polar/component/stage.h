#pragma once

#include <polar/asset/shaderprogram.h>
#include <polar/component/base.h>
#include <polar/core/ref.h>

namespace polar::component {
	class stage : public base {
	  public:
		struct shader {
			using shadertype = support::shader::shadertype;

			shadertype type;
			std::string source;

			shader(shadertype type, std::string source) : type(type), source(source) {}

			friend inline core::store_serializer &operator<<(core::store_serializer &s, const shader &sh) {
				return s << static_cast<uint8_t>(sh.type) << sh.source;
			}
		};

		std::vector<shader> shaders;

		stage(std::shared_ptr<asset::shaderprogram> as) {
			for(auto &sh : as->shaders) {
				shaders.emplace_back(sh.type, sh.source);
			}
		}

		bool serialize(core::store_serializer &s) const override {
			s << shaders;
			return true;
		}

		virtual std::string name() const override { return "stage"; }
	};
} // namespace polar::component
