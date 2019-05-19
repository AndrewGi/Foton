#pragma once
#include "../../glew/glew.h"
#include "../../mutex.hpp"
#include "../../exceptions.hpp"
namespace foton::GL {
	struct texture_t {
		struct texture_bind_t {

			static void activate_unit(GLsizei unit) {
				glActiveTexture(GL_TEXTURE0 + unit);
			}
			texture_t& parent() {
				return *_parent;
			}
			texture_bind_t(texture_t& parent) : _parent(&parent), _lock(_mutex) {
				glBindTexture(_target, parent._id);
			}
			bool valid() const {
				return _parent != nullptr && _lock.owns_lock();
			}
			~texture_bind_t() {
				if (_parent != nullptr && valid())
					glBindTexture(_target, 0);
			}
			void dont_unbind() {
				_parent = nullptr;
			}
			void upload(const uint8_t* pixels, GLsizei width, GLsizei height, GLint internal_format = GL_RGB, GLint format = GL_RGB, GLenum type = GL_FLOAT) {
				glTexImage2D(_target, 0, internal_format, width, height, 0, format, type, pixels);
				parent().width() = width;
				parent().height() = height;
			}
		protected:
			texture_bind_t(texture_t& parent, std::unique_lock<thread_mutex_t> lock) :
				_parent(&parent), _lock(std::move(lock)) {
				if constexpr (_DEBUG) {
					if (lock.mutex() != &_mutex && lock)
						throw exceptions::gl_error_t(0, "texture bind created from invalid mutex");
				}
			}

			static constexpr GLenum _target = GL_TEXTURE_2D;
			texture_t* _parent;
			std::unique_lock<foton::thread_mutex_t> _lock;
			static foton::thread_mutex_t _mutex;
			friend texture_t;
		};
		GLuint& width() {
			return _width;
		}
		GLuint& height() {
			return _height;
		}
		texture_bind_t bind() {
			return texture_bind_t(*this);
		}
		texture_t() {
			glGenTextures(1, &_id);
		}
		bool valid() const {
			return _id != 0;
		}

		texture_bind_t activate(GLsizei texture_unit) {
			if (texture_unit > 32)
				throw exceptions::gl_error_t(texture_unit, "texture_unit too high");
			auto lock = std::unique_lock<foton::thread_mutex_t>(texture_bind_t::_mutex);
			glActiveTexture(GL_TEXTURE + texture_unit);
			glBindTexture(GL_TEXTURE_2D, _id);
			return texture_bind_t(*this, std::move(lock));
		}
		~texture_t() {
			if (valid()) {
				glDeleteTextures(1, &_id);
				_id = 0;
			}
		}
	private:
		GLuint _id = 0;
		GLuint _width = 0;
		GLuint _height = 0;
	};
}

foton::thread_mutex_t foton::GL::texture_t::texture_bind_t::_mutex = {};