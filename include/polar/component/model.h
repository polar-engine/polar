#pragma once

#include <polar/asset/model.h>
#include <polar/component/base.h>

namespace polar::component {
	class model : public base {
	  public:
		std::shared_ptr<asset::model> asset;
		core::ref scene;
		core::ref material;

		model(decltype(asset) asset, core::ref scene, core::ref material) : asset(asset), scene(scene), material(material) {}

		virtual std::string name() const override { return "model"; }
	};
} // namespace polar::component
