#pragma once
#include "gl/fbo.hpp"
#include "gl/viewport.hpp"
namespace foton {
	namespace camera {
		struct projection_t {

			float fov_vertical;
			float near_plane = 0.1f;
			float far_plane;
			uint32_t width;
			uint32_t height;
			float aspect_ratio() const {
				return static_cast<float>(width) / height;
			}
			float range() const {
				return far_plane - near_plane;
			}
			mat4f as_mat() const {
				//Thank you eigen for reference
				mat4f mat;
				float theta = 0.5f * fov_vertical;
				float r = range();
				float invtan = 1.f / tan(theta);
				mat(0, 0) = invtan / aspect_ratio();
				mat(1, 1) = invtan;
				mat(2, 2) = -(near_plane + far_plane) / r;
				mat(3, 2) = -1;
				mat(2, 3) = -2 * near_plane * far_plane / r;
				mat(3, 3) = 0;
				return mat;
			}
		};
		struct view_t {
			vec3f position;
			quatf rotation;
			mat4f as_mat() const {
				aff3f mat;
				mat.linear() = rotation.conjugate().toRotationMatrix();
				mat.translation() = -(mat.linear()*position);
				return mat4f(mat.matrix());
			}
		};
		struct camera_t {
			struct camera_bind_t : GL::fbo_t::fbo_bind_t {
				camera_bind_t(camera_t& camera_parent, GL::fbo_t::fbo_bind_t&& bind) :
					_camera_parent(&camera_parent), GL::fbo_t::fbo_bind_t(std::move(bind)) {
					camera().apply_viewport();
				}
				camera_t& camera() {
					return *_camera_parent;
				}
				const camera_t& camera() const {
					return *_camera_parent;
				}
			private:
				camera_t* _camera_parent;
			};
			view_t view;
			projection_t projection;
			mutable mat4f view_matrix;
			mutable mat4f projection_matrix;
			GL::fbo_t fbo;
			GL::viewport_t viewport;
			void recalculate() const {
				view_matrix = view.as_mat();
				projection_matrix = projection.as_mat();
			}
			void apply_viewport() {
				viewport.apply();
			}
			camera_bind_t bind() {
				return camera_bind_t(*this, fbo.bind());
			}
		};
	}
}