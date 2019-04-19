#pragma once
#include "Eigen/Sparse"
#include "renderable.hpp"
namespace foton {
	namespace gfx_2D { //TODO: Better name
		using vec2d = Eigen::Vector2d;
		namespace shapes {
			struct shape_t : renderable_t {
				vec2d position;
				double rotation;
				shape_t(vec2d position, double rotation) : position(position), rotation(rotation) {};
				shape_t(vec2d position) : shape_t(position, 0.0) {};
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
		}
	}
}