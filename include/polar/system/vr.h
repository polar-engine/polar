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

		::vr::IVRSystem *vr_system = nullptr;
		std::unordered_set<::vr::TrackedDeviceIndex_t> tracked_devices;

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
			vr_system = ::vr::VR_Init(&err, ::vr::VRApplication_Scene);
			if(vr_system == nullptr) {
				auto err_string = ::vr::VR_GetVRInitErrorAsSymbol(err);
				debugmanager()->critical("failed to initialize OpenVR: ", err_string, " (", err, ')');
				return;
			}

			if(!::vr::VRCompositor()) {
				debugmanager()->critical("failed to initialize VR compositor");
				if(vr_system != nullptr) {
					::vr::VR_Shutdown();
					vr_system = nullptr;
				}
				return;
			}

			vr_system->GetRecommendedRenderTargetSize(&_width, &_height);

			debugmanager()->verbose("initialized OpenVR (width=", width(), ", height=", height(), ')');

			_ready = true;
		}

		~vr() {
			if(ready()) { ::vr::VR_Shutdown(); }
		}

		const Mat4 calc_head_view() {
			auto vr_comp = ::vr::VRCompositor();
			if(ready() && vr_comp) {
				::vr::TrackedDevicePose_t trackedDevicePose[::vr::k_unMaxTrackedDeviceCount];
				vr_comp->WaitGetPoses(trackedDevicePose, ::vr::k_unMaxTrackedDeviceCount, NULL, 0);

				auto vr_head_view = trackedDevicePose[0].mDeviceToAbsoluteTracking;
				_head_view = Mat4(vr_head_view.m[0][0], vr_head_view.m[0][1], vr_head_view.m[0][2], vr_head_view.m[0][3],
								  vr_head_view.m[1][0], vr_head_view.m[1][1], vr_head_view.m[1][2], vr_head_view.m[1][3],
								  vr_head_view.m[2][0], vr_head_view.m[2][1], vr_head_view.m[2][2], vr_head_view.m[2][3],
								  0, 0, 0, 1);
			}
			return head_view();
		}

		Mat4 projection(eye e, Decimal zNear, Decimal zFar) const {
			if(!ready()) { return Mat4(); }

			auto vr_eye = (e == eye::left) ? ::vr::Eye_Left : ::vr::Eye_Right;
			auto vr_proj = vr_system->GetProjectionMatrix(vr_eye, zNear, zFar);
			return glm::transpose(glm::make_mat4(&vr_proj.m[0][0]));
		}

		bool submit_gl(eye e, uintptr_t tex) {
			auto vr_comp = ::vr::VRCompositor();
			if(!(ready() && vr_comp)) { return false; }

			auto vr_eye = (e == eye::left) ? ::vr::Eye_Left : ::vr::Eye_Right;
			::vr::Texture_t vr_tex = {(void *)tex, ::vr::TextureType_OpenGL, ::vr::ColorSpace_Gamma};

			auto err = vr_comp->Submit(vr_eye, &vr_tex);
			if(err) {
				debugmanager()->critical("failed to submit texture to VR compositor (", err, ')');
				return false;
			} else {
				return true;
			}
		}
	};
}
