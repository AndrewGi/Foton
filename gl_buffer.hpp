#pragma once
#include <mutex>
#include <stdexcept>
#include "GL/glew.h"

namespace foton {
	namespace GL {
		using mutex = std::mutex;
		using lock = std::unique_lock<mutex>;
		struct wrong_enum_error_t : std::logic_error {
			GLenum incorrect_enum;
			wrong_enum_error_t(GLenum incorrect_enum) : incorrect_enum(incorrect_enum), std::logic_error("TODO: enum to string") {}
		};
		namespace buffer_locks {
			mutex vertex_attributes;
			mutex atomic_counter;
			mutex copy_read;
			mutex dispatch_indirect;
			mutex draw_indirect;
			mutex element_array;
			mutex pixel_pack;
			mutex pixel_unpack;
			mutex query;
			mutex shader_storage;
			mutex texture;
			mutex transform_feedback;
			mutex uniform;
			mutex* get_mutex(GLenum buffer_enum) {
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
		}
		struct buffer_t {
			using byte_t = uint8_t;
			struct buffer_bind_t {
				buffer_bind_t(GLuint buffer_id, GLenum target, mutex& mutex) : buffer_id(buffer_id), target(target), _lock(mutex) {
					glBindBuffer(target, buffer_id);
				};
				buffer_bind_t(const buffer_bind_t&) = delete;
				buffer_bind_t(buffer_bind_t&& other) : buffer_id(other.buffer_id), target(other.target), _lock(std::move(other._lock)) {
					other.buffer_id = 0;
				}
				~buffer_bind_t() {
					if (buffer_id > 0) {
						glBindBuffer(target, 0); //unbind itself
					}
				}
				operator GLuint() const {
					return target;
				}
				void upload_data(const byte_t* data, size_t size_in_bytes, GLenum usage = GL_STATIC_DRAW) {
					glBufferData(target, size_in_bytes, data, usage);
				}
				GLuint buffer_id;
				const GLenum target;
			private:
				lock _lock;
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
			template<class T>
			void upload_Ts(const T* data, size_t count, GLenum usage = GL_STATIC_DRAW) {
				auto bind = bind_buffer();
				bind.upload_data(data, sizeof(T)*count, usage);
			}
		private:
			GLuint _buffer_id = 0;
			GLenum _target = 0;
			mutex* _target_mutex = nullptr;
		};
		struct vao_t {
			
			struct vao_bind_t{
				static mutex _vao_bind_lock;
				vao_bind_t(GLuint id) : id(id), _lock(_vao_bind_lock) {
					glBindVertexArray(id);
				}
				vao_bind_t(const vao_bind_t&) = delete;
				vao_bind_t(vao_bind_t&& other) : id(other.id), _lock(std::move(other._lock)) {
					other.id = 0;
				}
				~vao_bind_t(){
					glBindVertexArray(0); //Unbind itself (maybe not needed)
				}
				operator GLuint() const {
					return id;
				}
				void assign_vertex_attribute(GLuint index, GLint size_per_element, GLenum type, GLsizei stride) {
					glEnableVertexAttribArray(index);
					glVertexAttribPointer(index, size_per_element, type, GL_FALSE, stride, 0);
				}
				template<class T>
				buffer_t assign_attributes(GLuint index, GLint count, const T* data, GLsizei stride) {
					GLenum type = []() {
						if constexpr (std::is_same_v<T, float>) {
							return GL_FLOAT;
						} //TODO: add more types
						static_assert(false, "unsupported vertex attribute type");
					}();
					buffer_t buf = buffer_t();
					auto bind = buf.bind_buffer();
					bind.upload_data(data, sizeof(T)*count);
					assign_vertex_attribute(index, sizeof(T), type, stride);
				}
				GLuint id;
				lock _lock;
			};
			vao_t() {
				glGenVertexArrays(1, &_id);
			}
			vao_bind_t bind() {
				return vao_bind_t(_id);
			}
		private:
			GLuint _id;
		};
	}
}
