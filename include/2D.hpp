#pragma once
#include <vector>
#include "Eigen/Sparse"
#include "graphics/drawer.hpp"
namespace foton {
	namespace gfx_2D { //TODO: Better name
		using vec2f = Eigen::Vector2f;
		namespace shapes {
			struct shape_t {
				vec2f position;
				double rotation;
			};
			struct square_t : shape_t {
				double raduis;
			};
			struct rectangle_t : shape_t {
				vec2f dimensions;
			};
			struct circle_t : shape_t {
				double radius;
			};
			struct shape_drawer_t : drawer_t {
				std::vector<square_t> squares;
				std::vector<rectangle_t> rectangles;
			};
		}
	}
}