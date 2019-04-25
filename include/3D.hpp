#pragma once
#include "Eigen/Sparse"
namespace foton {
	namespace gfx_3D {
		using vec3f = Eigen::Vector3f;
		struct triangle {
			vec3f v1, v2, v3;
		};
	}
}
