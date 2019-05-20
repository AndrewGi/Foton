#pragma once
#include "rbo.hpp"
namespace foton::GL {
		struct fbo_t {
			struct fbo_bind_t {
				fbo_bind_t(fbo_t& parent) : _parent(&parent), _lock(_mutex) {
					glBindFramebuffer(_target, parent.id());
				}
				fbo_bind_t(fbo_bind_t&& other) : _parent(other._parent), _lock(std::move(other._lock)) {
					other._parent = nullptr;
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
				const fbo_t& parent() const {
					return *_parent;
				}
				rbo_t::rbo_bind_t bind_to_rbo(rbo_t& rbo, GLenum attachment = GL_DEPTH_STENCIL_ATTACHMENT) {
					return bind_to_rbo(rbo.bind(), attachment);
				}
				rbo_t::rbo_bind_t bind_to_rbo(rbo_t::rbo_bind_t r_bind, GLenum attachment = GL_DEPTH_STENCIL_ATTACHMENT) {
					glFramebufferRenderbuffer(parent().target(), attachment, r_bind.internal_format(), r_bind.parent().id());
					return std::move(r_bind);
				}
				template<class T>
				void read_pixels(GLint x, GLint y, GLsizei width, GLsizei height, T* pixels, GLenum format = GL_RGBA) {
					
					glReadPixels(x, y, width, height, )
				}
				~fbo_bind_t() {
					if (valid())
						glBindFramebuffer(_target, 0);
				}
			private:
				fbo_t* _parent;
				std::unique_lock<thread_mutex_t> _lock;
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
			GLuint target() const {
				return _target;
			}
			fbo_bind_t bind() {
				return fbo_bind_t(*this);
			}
		private:
			static constexpr GLuint _target = GL_FRAMEBUFFER;
			GLuint _id;
			static thread_mutex_t _mutex;
		};
	
}