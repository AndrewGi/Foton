#pragma once
#include "../glew/glew.h"
#include "../gl/shader.hpp"
#include "types.hpp"
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
			view_t view;
			projection_t projection;
			mat4f view_matrix;
			mat4f projection_matrix;
			void recalculate() {
				view_matrix = view.as_mat();
				projection_matrix = projection.as_mat();
			}
			void set_uniforms(shader::shader_transform_uniforms_t& uniforms) const {
				uniforms.projection_mat = projection_matrix;
				uniforms.view_mat = view_matrix;
			}
			void set_viewport() const {
				glViewport(0, 0, projection.width, projection.height);
			}
		};
	}
}