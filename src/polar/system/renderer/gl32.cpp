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

		math::mat4x4 cameraView(1);

		auto ti_range = engine->objects.get<core::index::ti>().equal_range(typeid(component::playercamera));
		for(auto ti_it = ti_range.first; ti_it != ti_range.second; ++ti_it) {
			auto camera = static_cast<component::playercamera *>(ti_it->ptr.get());

			component::position *pos       = nullptr;
			component::orientation *orient = nullptr;
			// component::scale *sc           = nullptr;

			auto ref_range = engine->objects.get<core::index::ref>().equal_range(ti_it->r);
			for(auto ref_it = ref_range.first; ref_it != ref_range.second; ++ref_it) {
				auto type = ref_it->ti;
				if(type == typeid(component::position)) {
					pos = static_cast<component::position *>(ref_it->ptr.get());
				} else if(type == typeid(component::orientation)) {
					orient = static_cast<component::orientation *>(ref_it->ptr.get());
				}
			}

			cameraView = glm::translate(cameraView, -camera->distance.temporal(delta));
			cameraView *= glm::toMat4(camera->orientation.temporal(delta));
			if(orient != nullptr) { cameraView *= glm::toMat4(orient->orient.temporal(delta)); }
			cameraView = glm::translate(cameraView, -camera->position.temporal(delta));
			if(pos != nullptr) { cameraView = glm::translate(cameraView, -pos->pos.temporal(delta)); }
		}

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

	math::mat4x4 gl32::calculate_projection() {
		auto heightF    = static_cast<math::decimal>(height);
		auto fovy       = 2.0f * glm::atan(heightF, math::decimal(2) * pixelDistanceFromScreen) + fovPlus;
		auto projection = glm::perspective(fovy, static_cast<math::decimal>(width) / heightF, zNear, zFar);
		// auto projection = glm::infinitePerspective(fovy, static_cast<math::decimal>(width) / heightF, zNear);
		return projection;
	}
} // namespace polar::system::renderer
