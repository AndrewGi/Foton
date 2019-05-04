#pragma once
#include "Eigen/Geometry"
namespace foton {
	using byte_t = uint8_t;
	
	using vec3f = Eigen::Vector3f;
	using vec2f = Eigen::Vector2f;

	using quatf = Eigen::Quaternionf;
	using aff3f = Eigen::Affine3f;

	using mat3f = Eigen::Matrix3f;
	using mat4f = Eigen::Matrix4f;

	static_assert(sizeof(vec3f) == sizeof(float) * 3);
	static_assert(sizeof(vec2f) == sizeof(float) * 2);
	static_assert(sizeof(mat4f) == sizeof(float) * 4 * 4);
	static_assert(sizeof(mat3f) == sizeof(float) * 3 * 3);

	struct vertex_t {
		vec3f position;
		vec3f normal;
		vec2f texture_coords;
	};
	static_assert(sizeof(vertex_t) == sizeof(float) * (3 + 3 + 2));

}

