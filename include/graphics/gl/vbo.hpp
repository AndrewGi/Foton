#pragma once
#include "buffer.hpp"
namespace foton {
	namespace GL {
		template<class T>
		struct vbo_t : typed_buffer_t<T> {
			vbo_t() : typed_buffer_t<T>(GL_ARRAY_BUFFER) {
			}
			vbo_t(const T* data, GLsizei count, GLenum usage = GL_STATIC_DRAW) : vbo_t() {
				typed_buffer_t<T>::upload(data, count, usage);
			}
			vbo_t(std::initializer_list<T> list, GLenum usage = GL_STATIC_DRAW) : vbo_t() {
				std::vector<T> buffer(list);
				typed_buffer_t<T>::upload(buffer.data(), static_cast<GLsizei>(buffer.size()), usage);
			}

		};
		static_assert(sizeof(buffer_t) == sizeof(vbo_t<float>));
	}
}