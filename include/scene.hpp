#pragma once
#include "object.hpp"
#include "graphics/camera.hpp"
#include "graphics/gl/shader.hpp"
namespace foton {
	namespace scene {
		struct scene_t : object_t {
			std::vector<object_t*> objects;
			void draw_with(const camera::camera_t& camera) {
				camera.recalculate();
				mat4f projection = camera.projection_matrix;
				mat4f view = camera.view_matrix;
				for (const object_t* object : objects) {
					object->
				}
			}
		}
	}
}