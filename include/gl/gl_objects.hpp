#pragma once
#include <mutex>
#include <stdexcept>
#include "../drawer.hpp"
#include "glew/glew.h"
#include <vector>
namespace foton {
	namespace GL {
		template<class T>
		static constexpr GLenum _c_to_gl_type() {
			if constexpr (std::is_same_v<T, GLfloat>) {
				return GL_FLOAT;
			}
			if constexpr (std::is_same_v<T, GLint>) {
				return GL_INT;
			}
			//TODO: add more types
			static_assert(false, "unsupported vertex attribute type");
		}
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
				template<class T>
				struct map_t {
					static mutex _map_buffer_mutex;
					const GLenum target;
					lock map_buffer_lock;
					T* ptr = nullptr;
					map_t(GLenum target, GLenum access) : target(target), map_buffer_lock(_map_buffer_mutex) {
						ptr = glMapBuffer(target, access);
					}
					~map_t() {
						if (map_buffer_lock.owns_lock()) {
							glUnmapBuffer(target);
						}
					}
					map_t(const map_t<T>&) = delete;
					map_t(map_t<T>&& other) : target(other.target), map_buffer_lock(std::move(other.map_buffer_lock)), ptr(other.ptr) {
						other.ptr = nullptr;
					}
					operator T*() {
						return ptr;
					}
					operator T*() const {
						return ptr;
					}
				};
				buffer_bind_t(GLuint buffer_id, GLenum target, mutex& mutex, GLsizei& parent_size) : buffer_id(buffer_id), target(target), _lock(mutex), parent_size(parent_size) {
					glBindBuffer(target, buffer_id);
				};
				buffer_bind_t(const buffer_bind_t&) = delete;
				buffer_bind_t(buffer_bind_t&& other) : buffer_id(other.buffer_id), target(other.target), _lock(std::move(other._lock)), parent_size(other.parent_size) {
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
					parent_size = static_cast<GLsizei>(size_in_bytes);
				}
				GLsizei size() const {
					return parent_size;
				}
				GLuint buffer_id;
				GLsizei& parent_size;
				const GLenum target;
			private:
				lock _lock;
			};
			
			buffer_t() {
				glGenBuffers(1, &_buffer_id);
			}
			buffer_t(GLenum target) : buffer_t() {
				set_target(target);
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
				return buffer_bind_t(_buffer_id, _target, *_target_mutex, _size);
			}
			buffer_bind_t bind_buffer(GLenum target) {
				set_target(target);
				return bind_buffer();
			}
			template<class T>
			void upload_Ts(const T* data, size_t count, GLenum usage = GL_STATIC_DRAW) {
				auto bind = bind_buffer();
				bind.upload_data(reinterpret_cast<const byte_t*>(data), sizeof(T)*count, usage);
			}
			const GLint size() const {
				return _size;
			}
		private:
			GLsizei _size = 0;
			GLuint _buffer_id = 0;
			GLenum _target = 0;
			mutex* _target_mutex = nullptr;
		};
		struct vbo_location_t {
			const GLenum mode;
			buffer_t buffer;
		};
		template<class T>
		struct vbo_t : vbo_location_t, drawer_t {
			void upload(const T* data, size_t count, GLenum usage = GL_STATIC_DRAW) {
				buffer.upload_Ts(data, count, usage);
			}
			vbo_t(GLenum mode) : vbo_location_t{ GL_ARRAY_BUFFER, mode } {}
			vbo_t(const T* data, size_t count, GLenum usage, GLenum mode) : vbo_t(mode) {
				upload(data, count, usage);
			}
			vbo_t(std::initializer_list<T> list, GLenum mode, GLenum usage = GL_STATIC_DRAW) : vbo_t(mode) {
				std::vector<T> buffer(list);
				upload(buffer.data(), buffer.size(), usage);
			}
			GLsizei amount() const {
				return buffer.size() / sizeof(T);
			}
			void draw() const override {
				glDrawArrays(mode, 0, amount());
			}
		};
		struct vao_t {
			
			struct vao_bind_t{
				static mutex _vao_bind_lock;
				vao_bind_t(GLuint id, std::vector<vbo_location_t>& vbos) : _id(id), _lock(_vao_bind_lock), _vbos(vbos) {
					glBindVertexArray(id);
				}
				vao_bind_t(const vao_bind_t&) = delete;
				vao_bind_t(vao_bind_t&& other) : _id(other._id), _lock(std::move(other._lock)), _vbos(other._vbos) {
					other._id = 0;
				}
				~vao_bind_t(){
					glBindVertexArray(0); //Unbind itself (maybe not needed)
				}
				operator GLuint() const {
					return _id;
				}
				void assign_vertex_attribute(buffer_t& buf, GLuint index, GLint size_per_element, GLenum type, GLsizei stride) {
					auto bind = buf.bind_buffer();
					glEnableVertexAttribArray(index);
					glVertexAttribPointer(index, size_per_element, type, GL_FALSE, stride, 0);
				}

				template<class T, class... Args>
				vbo_t<T>& emplace_vbo(GLuint index, GLsizei stride, Args&&... args) {
					{
						vbo_t<T> vbo = vbo_t<T>(std::move(args)...);
						static_assert(sizeof(vbo_location_t) == sizeof(vbo_t<T>), "vbo types need to be same size/layout so we can reinterupt_cast");
						vbo_location_t& location = *reinterpret_cast<vbo_location_t*>(&vbo);
						_vbos.emplace_back(std::move(location));
					} //vbo is no longer valid
					vbo_t<T>& vbo = *reinterpret_cast<vbo_t<T>*>(&*(_vbos.end() - 1));
					assign_attributes(vbo.buffer, index, sizeof(T), _c_to_gl_type<T>(), stride);
					return vbo;
				}
				std::vector<vbo_location_t>& _vbos;
				GLuint _id;
				lock _lock;
			};
			vao_t() {
				glGenVertexArrays(1, &_id);
			}
			vao_bind_t bind() {
				return vao_bind_t(_id, _vbos);
			}
		private:
			std::vector<vbo_location_t> _vbos;
			GLuint _id;
		};
	}
}
