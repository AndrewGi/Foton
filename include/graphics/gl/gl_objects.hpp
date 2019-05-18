#pragma once
#include <mutex>
#include "../drawer.hpp"
#include <vector>
#include <variant>
namespace foton {
	namespace GL {
		
		
		

		struct rbo_t {
			struct rbo_bind_t {
				rbo_bind_t(rbo_t& parent) : _parent(&parent), _lock(_mutex) {
					glBindRenderbuffer(_target, parent.id());
				}
				rbo_bind_t(rbo_bind_t&& other) : _parent(other._parent), _lock(std::move(other._lock)) {
					other._parent = nullptr;
				}
				bool valid() {
					return parent().id() != 0 && _lock.owns_lock();
				}
				void set_storage(GLsizei width, GLsizei height) {
					glRenderbufferStorage(_target, _internal_format, width, height);
				}
				rbo_t& parent() {
					return *_parent;
				}
				static constexpr GLenum internal_format() {
					return _internal_format;
				}
				~rbo_bind_t() {
					if (valid())
						glBindRenderbuffer(_target, 0);
				}
			private:
				rbo_t* _parent;
				std::unique_lock<thread_mutex_t> _lock;
				static constexpr GLuint _target = GL_RENDERBUFFER;
				static constexpr GLenum _internal_format = GL_DEPTH24_STENCIL8;
			};
			rbo_t() {
				glGenRenderbuffers(1, &_id);
			}
			~rbo_t() {
				if (_id != 0)
					glDeleteRenderbuffers(1, &_id);
			}
			rbo_bind_t bind() {
				return rbo_bind_t(*this);
			}
			GLuint id() const {
				return _id;
			}
		private:
			GLuint _id;
			static thread_mutex_t _mutex;
		};
		struct fbo_t {
			struct fbo_bind_t {
				fbo_bind_t(fbo_t& parent) : _parent(&parent), _lock(_mutex) {
					glBindFramebuffer(_target, parent.id());
				}
				bool valid() {
					return parent().id() != 0 && _lock.owns_lock();
				}
				GLenum status() {
					if (valid())
						return glCheckFramebufferStatus(_target);
					else
						return GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT;
				}
				bool done() {
					return status() == GL_FRAMEBUFFER_COMPLETE;
				}
				fbo_t& parent() {
					return *_parent;
				}
				rbo_t::rbo_bind_t bind_to_rbo(rbo_t& rbo, GLenum attachment=GL_DEPTH_STENCIL_ATTACHMENT) {
					return bind_to_rbo(rbo.bind(), attachment);
				}
				rbo_t::rbo_bind_t bind_to_rbo(rbo_t::rbo_bind_t r_bind, GLenum attachment=GL_DEPTH_STENCIL_ATTACHMENT) {
					glFramebufferRenderbuffer(_target, attachment, r_bind.internal_format(), r_bind.parent().id());
					return std::move(r_bind);
				}
				
				~fbo_bind_t() {
					if (valid())
						glBindFramebuffer(_target, 0);
				}
			private:
				fbo_t* _parent;
				std::unique_lock<thread_mutex_t> _lock;
				static constexpr GLuint _target = GL_FRAMEBUFFER;
			};
			fbo_t() {
				glGenFramebuffers(1, &_id);
			}
			~fbo_t() {
				if (_id != 0)
					glDeleteFramebuffers(1, &_id);
			}
			GLuint id() const {
				return _id;
			}
		private:
			GLuint _id;
			static thread_mutex_t _mutex;
		};
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
					return _parent!=nullptr && _lock.owns_lock();
				}
				~texture_bind_t() {
					if (_parent!=nullptr && valid())
						glBindTexture(_target, 0);
				}
				void dont_unbind() {
					_parent = nullptr;
				}
				void upload(const byte_t* pixels, GLsizei width, GLsizei height, GLint internal_format = GL_RGB, GLint format = GL_RGB, GLenum type = GL_FLOAT) {
					glTexImage2D(_target, 0, internal_format, width, height, 0, format, type, pixels);
					parent().width() = width;
					parent().height() = height;
				}
			protected:
				texture_bind_t(texture_t& parent, std::unique_lock<thread_mutex_t> lock) :
					_parent(&parent), _lock(std::move(lock)) {
					if constexpr (_DEBUG) {
						if (lock.mutex() != &_mutex && lock)
							throw gl_error_t(0, "texture bind created from invalid mutex");
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
					throw gl_error_t(texture_unit, "texture_unit too high");
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
}
foton::thread_mutex_t foton::GL::vao_t::vao_bind_t::_mutex = {};
foton::thread_mutex_t foton::GL::texture_t::texture_bind_t::_mutex = {};