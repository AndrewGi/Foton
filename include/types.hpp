#pragma once
#include <ctype.h>
#include <algorithm>
#include <numeric>
namespace foton {
	using uint_t = uint32_t;
	using int_t = int32_t;
	template<class T, uint_t _length>
	struct vec_t {
		static constexpr uint_t length = _length;
		using this_t = vec_t<T, length>;
		static constexpr bool has_x = true;
		static constexpr auto greater_t = std::greater<uint_t>{};
		static constexpr bool has_y = (_length, 1);
		static constexpr bool has_z = greater_t(length, 2);
		static constexpr bool has_w = greater_t(length, 3);
		static_assert(length > 0);
		static_assert(std::is_trivial_v<T>);
		constexpr vec_t() = default;
		constexpr vec_t(T x) : x(std::move(x)) {}
		template<bool B = has_y, typename std::enable_if<B>::type...>
		constexpr vec_t(T x, T y) : x(std::move(x)), y(std::move(y)) {}
		template<bool B = has_z, typename std::enable_if<B>::type...>
		constexpr vec_t(T x, T y, T z) : x(std::move(x)), y(std::move(y)), z(std::move(z)) {}
		template<bool B = has_w, typename std::enable_if<B>::type...>
		constexpr vec_t(T x, T y, T z, T w) : x(std::move(x)), y(std::move(y)), z(std::move(z)), w(std::move(w)) {}
		vec_t(const T* in_data, uint32_t in_length = length) {
			std::copy(in_data, &in_data[in_length], data());
		}
		template<class... Args>
		constexpr vec_t(Args&& ... values) : _data{std::forward<Args>(values)...} {
		}
		union {
			struct {
				T x;
				std::enable_if_t<has_y, T> y;
				std::enable_if_t<has_z, T> z;
				std::enable_if_t<has_w, T> w;
			};
			T _data[length];
		};
		this_t operator-() const {
			this_t out(*this);
			std::transform(out.begin(), out.end(), std::negate<T>{});
			return out;

		}
		this_t operator+(const this_t& o) const {
			this_t out(*this);
			std::transform(out.begin(), out.end(), o.begin(), std::plus<T>{});
			return out;
		}

		this_t operator-(const this_t& o) const {
			this_t out(*this);
			std::transform(out.begin(), out.end(), o.begin(), std::minus<T>{});
			return out;
		}
		this_t operator*(T scalar) const {
			this_t out(*this);
			for (T& val : out)
				val *= scalar;
			return out;
		}
		this_t operator*(const this_t& o) const {
			this_t out(*this);
			std::transform(out.begin(), out.end(), o.begin(), std::multiplies<T>{});
			return out;
		}
		T& operator[](uint_t index) {
			return data()[index];
		}
		const T& operator[](uint_t index) const {
			return data()[index];
		}
		T dot(const this_t& other) const {
			return (*this * other).sed();
		}
		T sed() const {
			return std::accumulate(begin(), end(), T{});
		}
		T* data() {
			return reinterpret_cast<T*>(this);
		}
		const T* data() const {
			return reinterpret_cast<const T*>(this);
		}
		T* begin() {
			return data();
		}
		const T* begin() const {
			return data();
		}
		T* end() {
			return &data()[length];
		}
		const T* end() const {
			return &data()[length];
		}
	private:
	};
	template<class T, uint_t _rows, uint_t _cols>
	struct mat_t {
		struct identity_t {};
		static constexpr uint_t rows = _rows;
		static constexpr uint_t cols = _cols;
		using type = T;
		using colT = vec_t<T, rows>; //each colunm is 'rows' high
		using rowT = vec_t<T, cols>; //each row is 'cols' wide
		/*
			vecT == rowT


		*/

		mat_t(identity_t) {
			set_diagonal(1);
		}
		T* data() {
			return _data;
		}
		const T* data() const {
			return  _data;
		}
		rowT* begin() {
			return _data;
		}
		const rowT* begin() const {
			return _data;
		}
		rowT* end() {
			return &_data[rows];
		}
		const rowT& end() const {
			return &_data[rows];
		}
		void zero() {
			std::for_each(begin(), end(), [](rowT& row) {row.zero(); });
		}
		void set_diagonal(const T& value) {
			for (uint_t i = 0; i < std::min<uint_t>(rows, cols); i++) {
				data()[i][i] = value;
			}
		}
		rowT operator*(const rowT& o) {

		}
	private:
		rowT _data[cols];
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

	static_assert(sizeof(vec3f_t) == sizeof(float) * 3);
	static_assert(sizeof(vec2f_t) == sizeof(float) * 2);
	static_assert(sizeof(mat4f) == sizeof(float) * 4 * 4);
	static_assert(sizeof(mat3f) == sizeof(float) * 3 * 3);

	struct vertex_t {
		vec3f_t position;
		vec3f_t normal;
		vec2f_t texture_coords;
	};
	static_assert(sizeof(vertex_t) == sizeof(float) * (3 + 3 + 2));

}

