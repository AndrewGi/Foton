#pragma once
#include <vector>
#include "Eigen/Sparse"
#include "drawer.hpp"
namespace foton {
	namespace gfx_2D { //TODO: Better name
		using vec2d = Eigen::Vector2d;
		namespace shapes {
			struct shape_t {
				vec2d position;
				double rotation;
			};
			struct square_t : shape_t {
				double raduis;
			};
			struct rectangle_t : shape_t {
				vec2d dimensions;
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