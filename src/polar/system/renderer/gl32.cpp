#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>
#include <polar/component/color.h>
#include <polar/component/orientation.h>
#include <polar/component/playercamera.h>
#include <polar/component/position.h>
#include <polar/component/scale.h>
#include <polar/component/screenposition.h>
#include <polar/component/text.h>
#include <polar/core/polar.h>
#include <polar/math/constants.h>
#include <polar/math/mat.h>
#include <polar/math/point.h>
#include <polar/support/phys/detector/ball.h>
#include <polar/support/phys/detector/box.h>
#include <polar/system/asset.h>
#include <polar/system/renderer/gl32.h>
#include <polar/system/vr.h>

namespace polar::system::renderer {
	void gl32::update(DeltaTicks &dt) {
		return;

		fps_object = engine->add();

		if(dt.Seconds() > 0) { fps = glm::mix(fps, 1 / dt.Seconds(), math::decimal(0.1)); }

		if(showFPS) {
			std::ostringstream oss;
			oss << (int)fps << " fps";

			auto assetM = engine->get<asset>().lock();
			auto font   = assetM->get<polar::asset::font>("nasalization-rg");

			engine->add<component::text>(fps_object, font, oss.str());
			engine->add<component::screenposition>(fps_object, math::point2(5, 5), support::ui::origin::topleft);
			engine->add<component::color>(fps_object, math::point4(1, 1, 1, 0.8));
			engine->add<component::scale>(fps_object, math::point3(0.125));
		}

		auto clock_ref = engine->own<tag::clock::simulation>();
		auto clock     = engine->add_as<component::clock::base, component::clock::simulation>(clock_ref);
		float delta    = clock->delta();

		/*
		auto vr = engine->get<system::vr>().lock();
		if(vr && vr->ready()) {
			using eye = support::vr::eye;

			vr->update_poses();

			cameraView = glm::transpose(vr->head_view()) * cameraView;

			render(vr->projection(eye::left, zNear, zFar), cameraView, delta);
			GL(vr->submit_gl(eye::left, nodes.back().outs.at("color")));
			render(vr->projection(eye::right, zNear, zFar), cameraView, delta);
			GL(vr->submit_gl(eye::right, nodes.back().outs.at("color")));
		} else {
			render(calculate_projection(), cameraView, delta);
		}
		*/
	}
} // namespace polar::system::renderer
