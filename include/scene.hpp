#pragma once
#include "object.hpp"
#include "../graphics/camera.hpp"
namespace foton {
	namespace scene {
		struct scene_t {
			std::vector<object_t*> objects;
			void draw_with(const camera::camera_t& camera, shader_t& shader) {

			}
		}
	}
}