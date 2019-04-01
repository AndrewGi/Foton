#pragma once
#include "Eigen/Sparse"
#include "renderable.hpp"
namespace foton {
	namespace gfx_2D { //TODO: Better name
		using vec2d = Eigen::Vector2d;
		struct square_t : renderable_t {
			vec2d position;
			vec2d size;
			double rotation = 0;
		};
	}
}