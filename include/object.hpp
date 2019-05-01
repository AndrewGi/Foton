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
		quatf rotation;
		void draw() override {
			for (drawer_t* drawer : _drawers) {
				drawer->draw();
			}
		}
		mat4f model_mat() {
			aff3f out;
			out.translate(position);
			out.rotate(rotation);
			return out.matrix;
		}
	};
}