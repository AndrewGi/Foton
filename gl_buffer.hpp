#pragma once
#include <mutex>
#include "GL/glew.h"
namespace foton {
	namespace GL {
		struct wrong_enum_error_t : std::exception {
			GLenum incorrect_enum;
			wrong_enum_error_t(GLenum incorrect_enum) : incorrect_enum(incorrect_enum), std::exception("TODO: enum to string") {}
		};
		namespace buffer_locks {
			using mutex = std::mutex;
			std::mutex vertex_attributes;
			std::mutex atomic_counter;
			std::mutex copy_read;
			std::mutex dispatch_indirect;
			std::mutex draw_indirect;
			std::mutex element_array;
			std::mutex pixel_pack;
			std::mutex pixel_unpack;
			std::mutex query;
			std::mutex shader_storage;
			std::mutex texture;
			std::mutex transform_feedback;
			std::mutex uniform;
			std::mutex* get_mutex(GLenum buffer_enum) {
				switch (buffer_enum) {
				case GL_ARRAY_BUFFER:
					return &vertex_attributes;
				case GL_ATOMIC_COUNTER_BUFFER:
					return &atomic_counter;
				case GL_COPY_READ_BUFFER:
					return &copy_read;
				case GL_DISPATCH_INDIRECT_BUFFER:
					return &dispatch_indirect;
				case GL_DRAW_INDIRECT_BUFFER:
					return &draw_indirect;
				case GL_ELEMENT_ARRAY_BUFFER:
					return &element_array;
				case GL_PIXEL_PACK_BUFFER:
					return &pixel_pack;
				case GL_PIXEL_UNPACK_BUFFER:
					return &pixel_unpack;
				case GL_QUERY_BUFFER:
					return &query;
				case GL_SHADER_STORAGE_BUFFER:
					return &shader_storage;
				case GL_TEXTURE_BUFFER:
					return &texture;
				case GL_TRANSFORM_FEEDBACK_BUFFER:
					return &transform_feedback;
				case GL_UNIFORM_BUFFER:
					return &uniform;
				default:
					throw wrong_enum_error_t{ buffer_enum };
			}
		}
		struct buffer_t {
			using byte_t = uint8_t;
			struct buffer_bind_t {
				buffer_bind_t(GLuint buffer_id, GLenum target, std::mutex& mutex) : buffer_id(buffer_id), target(target), _lock(mutex) {
					glBindBuffer(target, buffer_id);
				};
				~buffer_bind_t() {
					glBindBuffer(target, 0); //unbind itself
				}
				operator GLuint() const {
					return target;
				}
				const GLuint buffer_id;
				const GLenum target;
			private:
				const std::unique_lock<std::mutex> _lock;
			};
			buffer_t() {
				glGenBuffers(1, &_buffer_id);
			}
			buffer_t(const buffer_t&) = delete;
			buffer_t(buffer_t&& other) : _buffer_id(other._buffer_id), _target(other._target) {
				other._buffer_id = 0;
				other._target = 0;
			}
			~buffer_t(){
				if (_buffer_id != 0)
					glDeleteBuffers(1, &_buffer_id);
			}
			/*
				**WARNING**
				don't use buffer_id for any context sensitive calls
				anything that needs glBindBuffers() called first should use bind_buffer() to safely bind the buffer
			*/
			GLuint buffer_id() const {
				return _buffer_id;
			}
			void set_target(GLenum target) {
				if (_target == target) //target already set
					return;
				_target_mutex = buffer_locks::get_mutex(target);
				_target = target;
			}
			buffer_bind_t bind_buffer() {
				return buffer_bind_t(_buffer_id, _target, *_target_mutex);
			}
			buffer_bind_t bind_buffer(GLenum target) {
				set_target(target);
				return bind_buffer();
			}
			void upload_data(const byte_t* data, size_t size, GLenum usage = GL_STATIC_DRAW) {
				auto id = bind_buffer();
				glBufferData(id, size, data, usage);
			}
		private:
			GLuint _buffer_id = 0;
			GLenum _target = 0;
			std::mutex* _target_mutex = nullptr;
		};
		struct vao_t {

		};
	}
}
