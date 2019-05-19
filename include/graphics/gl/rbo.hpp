#pragma once
#include "../../types.hpp"
#include "../../glew/glew.h"
#include "../../mutex.hpp"

namespace foton::GL {
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
			void set_dim(GLsizei width, GLsizei height) {
				glRenderbufferStorage(parent().target(), parent().internal_format(), width, height);
			}
			rbo_t& parent() {
				return *_parent;
			}
			const rbo_t& parent() const {
				return *_parent;
			}
			GLenum internal_format() const {
				return parent().internal_format();
			}
			GLuint target() const {
				return parent().target();
			}
			~rbo_bind_t() {
				if (valid())
					glBindRenderbuffer(parent().target(), 0);
			}
		private:
			rbo_t* _parent;
			std::unique_lock<thread_mutex_t> _lock;
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
		GLenum internal_format() const {
			return _internal_format;
		}
		GLuint target() const {
			return _target;
		}
	private:
		static constexpr GLuint _target = GL_RENDERBUFFER;
		GLenum _internal_format = GL_DEPTH24_STENCIL8;
		GLuint _id;
		static thread_mutex_t _mutex;
	};
}