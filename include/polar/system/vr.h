#pragma once

#include <openvr.h>
#include <polar/support/action/vr.h>
#include <polar/support/vr/eye.h>
#include <polar/system/base.h>
#include <unordered_set>

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
		std::vector<std::string> render_models_loading;

		bool _ready = false;
		uint32_t _width = 0;
		uint32_t _height = 0;

		Mat4 _head_view = Mat4(1);
		Mat4 _left_hand_view = Mat4(1);
		Mat4 _right_hand_view = Mat4(1);

		Mat4 _last_head_view = Mat4(1);
		Mat4 _last_left_hand_view = Mat4(1);
		Mat4 _last_right_hand_view = Mat4(1);

		std::string tracked_device_str(::vr::TrackedDeviceIndex_t i, ::vr::TrackedDeviceProperty p) {
			::vr::TrackedPropertyError err;
			auto len = vr_system->GetStringTrackedDeviceProperty(i, p, nullptr, 0, &err);
			if(len == 0) { return ""; }

			std::string str(len, '\0');
			vr_system->GetStringTrackedDeviceProperty(i, p, str.data(), len, &err);
			return str;
		}

		::vr::RenderModel_t * load_render_model(std::string name) {
			auto vr_models = ::vr::VRRenderModels();
			if(vr_models) {
				::vr::RenderModel_t *vr_model;
				auto err = vr_models->LoadRenderModel_Async(name.data(), &vr_model);
				auto err_str = vr_models->GetRenderModelErrorNameFromEnum(err);
				if(err == ::vr::VRRenderModelError_None) {
					debugmanager()->verbose("loaded render model ", name);
					return vr_model;
				} else {
					debugmanager()->verbose("failed to load render model ", name, ": ", err_str);
				}
			}
			return nullptr;
		}

		void load_render_models() {
			for(auto it = render_models_loading.begin(); it != render_models_loading.end();) {
				auto vr_model = load_render_model(*it);
				if(vr_model) {
					it = render_models_loading.erase(it);
					debugmanager()->verbose(vr_model->unVertexCount);
					/* XXX: we need support for indexed rendering and textured
					 *      rendering, and we also need to be able to specify
					 *      which shader (or which stage of the deferred
					 *      pipeline) should be used to render a model
					 */
				} else { ++it; }
			}
		}

	public:
		static bool supported() { return true; }

		static Mat4 pose_view(::vr::TrackedDevicePose_t pose) {
			auto vr_view = pose.mDeviceToAbsoluteTracking;
			return Mat4(vr_view.m[0][0], vr_view.m[1][0], vr_view.m[2][0], 0,
			            vr_view.m[0][1], vr_view.m[1][1], vr_view.m[2][1], 0,
			            vr_view.m[0][2], vr_view.m[1][2], vr_view.m[2][2], 0,
			            vr_view.m[0][3], vr_view.m[1][3], vr_view.m[2][3], 1);
			return Mat4(vr_view.m[0][0], vr_view.m[0][1], vr_view.m[0][2], vr_view.m[0][3],
			            vr_view.m[1][0], vr_view.m[1][1], vr_view.m[1][2], vr_view.m[1][3],
			            vr_view.m[2][0], vr_view.m[2][1], vr_view.m[2][2], vr_view.m[2][3],
			            0, 0, 0, 1);
		}

		inline auto ready()                const { return _ready; }
		inline auto width()                const { return _width; }
		inline auto height()               const { return _height; }
		inline auto head_view()            const { return _head_view; }
		inline auto left_hand_view()       const { return _left_hand_view; }
		inline auto right_hand_view()      const { return _right_hand_view; }
		inline auto last_head_view()       const { return _last_head_view; }
		inline auto last_left_hand_view()  const { return _last_left_hand_view; }
		inline auto last_right_hand_view() const { return _last_right_hand_view; }

		vr(core::polar *engine) : base(engine) {
			if(!::vr::VR_IsHmdPresent()) { return; }

			::vr::HmdError err;
			vr_system = ::vr::VR_Init(&err, ::vr::VRApplication_Scene);
			if(vr_system == nullptr) {
				auto err_str = ::vr::VR_GetVRInitErrorAsSymbol(err);
				debugmanager()->critical("failed to initialize OpenVR: ", err_str, " (", err, ')');
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

			for(uint32_t i = 0; i < ::vr::k_unMaxTrackedDeviceCount; ++i) {
				if(vr_system->IsTrackedDeviceConnected(i)) {
					auto dev_class = vr_system->GetTrackedDeviceClass(i);

					std::string dev_class_str;
					switch(dev_class) {
					default:
						dev_class_str = "unknown";
						break;
					case ::vr::TrackedDeviceClass_HMD:
						dev_class_str = "head-mounted display";
						break;
					case ::vr::TrackedDeviceClass_Controller:
						dev_class_str = "controller";
						break;
					case ::vr::TrackedDeviceClass_GenericTracker:
						dev_class_str = "generic tracker";
						break;
					case ::vr::TrackedDeviceClass_TrackingReference:
						dev_class_str = "tracking reference";
						break;
					case ::vr::TrackedDeviceClass_DisplayRedirect:
						dev_class_str = "display redirect";
						break;
					}

					debugmanager()->verbose("found VR ", dev_class_str, " device at index ", i);

					tracked_devices.emplace(i);

					if(dev_class == ::vr::TrackedDeviceClass_Controller) {
						auto model_str = tracked_device_str(i, ::vr::Prop_RenderModelName_String);
						debugmanager()->verbose("with render model ", model_str);

						render_models_loading.emplace_back(model_str);
					}
				}
			}

			load_render_models();

			_ready = true;
		}

		~vr() {
			if(ready()) { ::vr::VR_Shutdown(); }
		}

		void update(DeltaTicks &) {
			if(!ready()) { return; }

			load_render_models();

			::vr::VREvent_t ev;
			while(vr_system->PollNextEvent(&ev, sizeof(ev))) {
				// TODO: handle OpenVR events
			}

			auto action = engine->get<system::action>().lock();
			if(action) {
				namespace a_vr = support::action::vr;

				::vr::VRControllerState_t state;

				bool any_app_menu = false;
				bool any_a        = false;
				bool any_trigger  = false;

				auto left_hand = vr_system->GetTrackedDeviceIndexForControllerRole(::vr::TrackedControllerRole_LeftHand);
				if(vr_system->GetControllerState(left_hand, &state, sizeof(state))) {
					bool app_menu = state.ulButtonPressed & ::vr::ButtonMaskFromId(::vr::k_EButton_ApplicationMenu);
					bool a        = state.ulButtonPressed & ::vr::ButtonMaskFromId(::vr::k_EButton_A);
					bool touchpad = state.ulButtonPressed & ::vr::ButtonMaskFromId(::vr::k_EButton_SteamVR_Touchpad);
					bool trigger  = state.ulButtonPressed & ::vr::ButtonMaskFromId(::vr::k_EButton_SteamVR_Trigger);

					any_app_menu |= app_menu;
					any_a        |= a | touchpad;
					any_trigger  |= trigger;

					action->trigger<a_vr::app_menu<a_vr::type::left_hand>>(app_menu);
					action->trigger<a_vr::a       <a_vr::type::left_hand>>(a | touchpad);
					action->trigger<a_vr::trigger <a_vr::type::left_hand>>(trigger);

					for(int i = 0; i < ::vr::k_unControllerStateAxisCount; ++i) {
						auto prop = ::vr::ETrackedDeviceProperty(::vr::Prop_Axis0Type_Int32 + i);
						auto type = vr_system->GetInt32TrackedDeviceProperty(left_hand, prop);
						switch(type) {
						default:
							break;
						case ::vr::k_eControllerAxis_TrackPad:
						case ::vr::k_eControllerAxis_Joystick:
							action->accumulate<a_vr::axis_x<a_vr::type::left_hand>>(state.rAxis[i].x);
							action->accumulate<a_vr::axis_y<a_vr::type::left_hand>>(state.rAxis[i].y);
							action->accumulate<a_vr::axis_x<a_vr::type::any      >>(state.rAxis[i].x);
							action->accumulate<a_vr::axis_y<a_vr::type::any      >>(state.rAxis[i].y);
							break;
						}
					}
				}

				auto right_hand = vr_system->GetTrackedDeviceIndexForControllerRole(::vr::TrackedControllerRole_RightHand);
				if(vr_system->GetControllerState(right_hand, &state, sizeof(state))) {
					bool app_menu = state.ulButtonPressed & ::vr::ButtonMaskFromId(::vr::k_EButton_ApplicationMenu);
					bool a        = state.ulButtonPressed & ::vr::ButtonMaskFromId(::vr::k_EButton_A);
					bool touchpad = state.ulButtonPressed & ::vr::ButtonMaskFromId(::vr::k_EButton_SteamVR_Touchpad);
					bool trigger  = state.ulButtonPressed & ::vr::ButtonMaskFromId(::vr::k_EButton_SteamVR_Trigger);

					any_app_menu |= app_menu;
					any_a        |= a | touchpad;
					any_trigger  |= trigger;

					action->trigger<a_vr::app_menu<a_vr::type::right_hand>>(app_menu);
					action->trigger<a_vr::a       <a_vr::type::right_hand>>(a | touchpad);
					action->trigger<a_vr::trigger <a_vr::type::right_hand>>(trigger);

					for(size_t i = 0; i < ::vr::k_unControllerStateAxisCount; ++i) {
						auto prop = ::vr::ETrackedDeviceProperty(::vr::Prop_Axis0Type_Int32 + i);
						auto type = vr_system->GetInt32TrackedDeviceProperty(right_hand, prop);
						switch(type) {
						default:
							break;
						case ::vr::k_eControllerAxis_TrackPad:
						case ::vr::k_eControllerAxis_Joystick:
							action->accumulate<a_vr::axis_x<a_vr::type::right_hand>>(state.rAxis[i].x);
							action->accumulate<a_vr::axis_y<a_vr::type::right_hand>>(state.rAxis[i].y);
							action->accumulate<a_vr::axis_x<a_vr::type::any       >>(state.rAxis[i].x);
							action->accumulate<a_vr::axis_y<a_vr::type::any       >>(state.rAxis[i].y);
							break;
						}
					}
				}

				action->trigger<a_vr::app_menu<a_vr::type::any>>(any_app_menu);
				action->trigger<a_vr::a       <a_vr::type::any>>(any_a);
				action->trigger<a_vr::trigger <a_vr::type::any>>(any_trigger);
			}
		}

		void update_poses() {
			auto vr_comp = ::vr::VRCompositor();
			if(ready() && vr_comp) {
				::vr::TrackedDevicePose_t poses[::vr::k_unMaxTrackedDeviceCount];
				vr_comp->WaitGetPoses(poses, ::vr::k_unMaxTrackedDeviceCount, nullptr, 0);

				auto left_hand  = vr_system->GetTrackedDeviceIndexForControllerRole(::vr::TrackedControllerRole_LeftHand);
				auto right_hand = vr_system->GetTrackedDeviceIndexForControllerRole(::vr::TrackedControllerRole_RightHand);

				_last_head_view       = head_view();
				_last_left_hand_view  = left_hand_view();
				_last_right_hand_view = right_hand_view();

				_head_view       = pose_view(poses[::vr::k_unTrackedDeviceIndex_Hmd]);
				_left_hand_view  = pose_view(poses[left_hand]);
				_right_hand_view = pose_view(poses[right_hand]);
			}
		}

		Mat4 projection(eye e, Decimal zNear, Decimal zFar) const {
			if(!ready()) { return Mat4(1); }

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
