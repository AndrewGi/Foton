#pragma once
#include "object.hpp"
#include "graphics/camera.hpp"
#include "graphics/gl/shader.hpp"
namespace foton {
	namespace scene {
		struct scene_t : drawer_t {
			std::vector<object_t*> objects;
			void draw_with(const camera::camera_t& camera) {
				camera.recalculate();
				const mat4f projection = camera.projection_matrix;
				const mat4f view = camera.view_matrix;
				for (const object_t* object : objects) {
					const mat4f model_mat = object->model_mat();

				}
			}
		};
	}
}