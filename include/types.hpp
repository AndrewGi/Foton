#pragma once
#include <ctype.h>
#include <algorithm>
namespace foton {
	
	template<class T, uint32_t _length>
	struct vec_t {
		using this_t = vec_t<T, _length>;
		static constexpr bool has_x = true;
		static constexpr bool has_y = std::greater(_length, 1);
		static constexpr bool has_z = std::greater(_length, 2);
		static constexpr bool has_w = std::greater(_length, 3);
		static constexpr auto length = _length;
		static_assert(_length > 0);
		static_assert(std::is_trivial_v<T>);
		vec_t() = default;
		vec_t(T x) : x(std::move(x)) {}
		vec_t(T x, T y, std::enable_if_t<has_y>*=nullptr) : x(std::move(x)), y(std::move(y)) {}
		vec_t(T x, T y, T z, std::enable_if_t<has_z>*=nullptr) : x(std::move(x)), y(std::move(y)), z(std::move(z)) {}
		vec_t(T x, T y, T z, T w, std::enable_if_t<has_w>*=nullptr) : x(std::move(x)), y(std::move(y)), z(std::move(z)), w(std::move(w)) {}
		vec_t(const T* in_data) {
			std::copy(in_data, &in_data[_length], data());
		}
		vec_t(const T* in_data, uint32_t in_length) {
			std::copy(in_data, &in_data[in_length], data());
		}
		vec_t(std::initializer_list<T> list) {
			static_assert(list.size() == length);
			std::copy(list.begin(), list.end(), data());
		}
		T x = {};
		std::enable_if_t<has_y, T> y = {};
		std::enable_if_t<has_z, T> z = {};
		std::enable_if_t<has_w, T> w = {};
		std::enable_if_t<std::greater(length, 4), T[length - 4]> rest = {};
		this_t operator-() const {
			if constexpr (has_w) {
				return this_t(-x, -y, -z, -w);
			}
			else if constexpr (has_z) {
				return this_t(-x, -y, -z);
			}
			else if constexpr (has_y) {
				return this_t(-x, -y);
			}
			else {
				return this_t(-x);
			}

		}
		this_t operator+(const this_t& o) const {
			this_t out(*this);
			std::transform(out.begin(), out.end(), o.begin(), std::minus);
			return out;
		}

		this_t operator-(const this_t& o) const {
			this_t out(*this);
			std::transform(out.begin(), out.end(), o.begin(), std::minus);
			return out;
		}
		this_t operator*(T scalar) const {
			this_t out(*this);
			std::transform(out.begin(), out.end(), [scalar](const T& t) {return t * scalar; });
		}
		T dot(const this_t& other) const {
			T out = {};
			for (uint32_t i = 0; i < length; i++)
				out += data()[i] * other.data()[i];
			return out;
			
		}
		T* data() {
			return static_cast<T*>(this);
		}
		const T* data() const {
			return static_cast<T*>(this);
		}
		T* begin() {
			return data();
		}
		const T* begin() {
			return data();
		}
		T* end() {
			return &data()[length];
		}
		const T* end() const {
			return &data()[length];
		}
	};
	using vec2f_t = vec_t<float, 2>;
	using vec3f_t = vec_t<float, 3>;
	using vec4f_t = vec_t<float, 4>;
	static_assert(std::is_trivially_copyable_v<vec4f_t>);

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
		vec3f_t position;
		vec3f_t normal;
		vec2f_t texture_coords;
	};
	static_assert(sizeof(vertex_t) == sizeof(float) * (3 + 3 + 2));

}

