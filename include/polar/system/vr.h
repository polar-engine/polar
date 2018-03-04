#pragma once

#include <openvr.h>
#include <polar/support/vr/eye.h>
#include <polar/system/base.h>

namespace polar::system {
	/* XXX: Oculus Rift does not support hidden area meshes due to
	 *      Asynchronous Time Warp, but we should still implement this
	 *      for other HMDs such as HTC Vive
	 */
	class vr : public base {
	private:
		using eye = support::vr::eye;

		::vr::IVRSystem *vrSystem = nullptr;
		bool _ready = false;
		uint32_t _width = 0;
		uint32_t _height = 0;
		Mat4 _head_view = Mat4();

	public:
		static bool supported() { return true; }

		inline auto ready() const { return _ready; }
		inline auto width() const { return _width; }
		inline auto height() const { return _height; }
		inline auto head_view() const { return _head_view; }

		vr(core::polar *engine) : base(engine) {
			if(!::vr::VR_IsHmdPresent()) { return; }

			::vr::HmdError err;
			vrSystem = ::vr::VR_Init(&err, ::vr::VRApplication_Scene);
			if(vrSystem == nullptr) {
				auto errString = ::vr::VR_GetVRInitErrorAsSymbol(err);
				debugmanager()->critical("failed to initialize OpenVR: ", errString, " (", err, ')');
				return;
			}

			if(!::vr::VRCompositor()) {
				debugmanager()->critical("failed to initialize VR compositor");
				if(vrSystem != nullptr) {
					::vr::VR_Shutdown();
					vrSystem = nullptr;
				}
				return;
			}

			vrSystem->GetRecommendedRenderTargetSize(&_width, &_height);

			debugmanager()->verbose("initialized OpenVR (width=", width(), ", height=", height(), ')');

			_ready = true;
		}

		~vr() {
			if(ready()) { ::vr::VR_Shutdown(); }
		}

		const Mat4 calc_head_view() {
			if(ready()) {
				::vr::TrackedDevicePose_t trackedDevicePose[::vr::k_unMaxTrackedDeviceCount];
				::vr::VRCompositor()->WaitGetPoses(trackedDevicePose, ::vr::k_unMaxTrackedDeviceCount, NULL, 0);

				auto vrHeadView = trackedDevicePose[0].mDeviceToAbsoluteTracking;
				_head_view = Mat4(vrHeadView.m[0][0], vrHeadView.m[0][1], vrHeadView.m[0][2], vrHeadView.m[0][3],
								  vrHeadView.m[1][0], vrHeadView.m[1][1], vrHeadView.m[1][2], vrHeadView.m[1][3],
								  vrHeadView.m[2][0], vrHeadView.m[2][1], vrHeadView.m[2][2], vrHeadView.m[2][3],
								  0, 0, 0, 1);
			}
			return head_view();
		}

		Mat4 projection(eye e, Decimal zNear, Decimal zFar) const {
			if(!ready()) { return Mat4(); }

			auto vrEye = (e == eye::left) ? ::vr::Eye_Left : ::vr::Eye_Right;
			auto vrProj = vrSystem->GetProjectionMatrix(vrEye, zNear, zFar);
			return glm::transpose(glm::make_mat4(&vrProj.m[0][0]));
		}

		bool submit_gl(eye e, uintptr_t tex) {
			if(!ready()) { return false; }

			auto vrEye = (e == eye::left) ? ::vr::Eye_Left : ::vr::Eye_Right;
			::vr::Texture_t vrTex = {(void *)tex, ::vr::TextureType_OpenGL, ::vr::ColorSpace_Gamma};

			auto err = ::vr::VRCompositor()->Submit(vrEye, &vrTex);
			if(err) {
				debugmanager()->critical("failed to submit texture to VR compositor (", err, ')');
				return false;
			} else {
				return true;
			}
		}
	};
}
