#pragma once
#include <vector>
#include "drawer.hpp"
#include "model.hpp"
namespace foton {
	class object_t : drawer_t {
		using mat4f = Eigen::Matrix4f;
		std::vector<model::model_t> models;
		std::vector<drawer_t*> _drawers;
		vec3f position;

		void draw() override {
			for (drawer_t* drawer : _drawers) {
				drawer->draw();
			}
		}
		mat4f model_mat() {
			return mat4f(position.x, position.y, position.z);
		}
	};
}