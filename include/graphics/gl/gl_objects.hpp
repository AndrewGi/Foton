#pragma once
#include <mutex>
#include <stdexcept>
#include "../drawer.hpp"
#include "glew/glew.h"
#include <vector>
namespace foton {
	namespace GL {
		struct gl_error_t : std::runtime_error {
			gl_error_t(GLenum err, std::string msg) : std::runtime_error(std::string(
				"glError: (") + std::to_string(err) + ") " + msg
			) {}
		};
		void check_gl_errors(const char* message) {
			if constexpr (_DEBUG) { //Should probably use a macro but this is cleaner for now
				auto err = glGetError();
				if (err != GL_NO_ERROR) {
					throw gl_error_t(err, message);
					check_gl_errors("multiple errors");
				}
			} 
		}
		void clear_gl_errors(bool throw_on_no_error = true) {
			auto err = glGetError();
			if (err != GL_NO_ERROR && throw_on_no_error) {
				throw gl_error_t(err, "clear_gl_errors called while no errors");
			}
			while (err != GL_NO_ERROR)
				glGetError();
		}
		template<class T>
		static constexpr std::pair<GLenum, GLint> T_gl_type() {
			if constexpr (std::is_same_v<T, GLfloat>) {
				return { GL_FLOAT, 1 };
			}
			else if constexpr (std::is_same_v<T, GLint>) {
				return { GL_INT, 1 };
			} 
			else if constexpr (std::is_same_v<T, GLuint>) {
				return { GL_UNSIGNED_INT, 1 };
			}
			else if constexpr (std::is_same_v<T, Eigen::Vector3f>) {
				return { GL_FLOAT, 3 };
			}
			else if constexpr (std::is_same_v<T, Eigen::Vector2f>) {
				return { GL_FLOAT, 2 };
			}
			else {
				//TODO: add more types
				static_assert(false, "unsupported vertex attribute type");
			}
		}
		struct wrong_enum_error_t : std::logic_error {
			GLenum incorrect_enum;
			wrong_enum_error_t(GLenum incorrect_enum) : incorrect_enum(incorrect_enum), std::logic_error("TODO: enum to string") {}
		};
		static constexpr size_t gl_type_size(GLenum type) {
			switch (type) {
			case GL_INT:
			case GL_FLOAT:
			case GL_UNSIGNED_INT:
				static_assert(sizeof(GLint) == 4 && sizeof(GLfloat) == 4 && sizeof(GLuint) == 4);
				return 4;
			default:
				//implement more types here
				throw wrong_enum_error_t(type);
			}
		}
		namespace buffer_locks {
			thread_mutex_t vertex_attributes;
			thread_mutex_t atomic_counter;
			thread_mutex_t copy_read;
			thread_mutex_t dispatch_indirect;
			thread_mutex_t draw_indirect;
			thread_mutex_t element_array;
			thread_mutex_t pixel_pack;
			thread_mutex_t pixel_unpack;
			thread_mutex_t query;
			thread_mutex_t shader_storage;
			thread_mutex_t texture;
			thread_mutex_t transform_feedback;
			thread_mutex_t uniform;
			thread_mutex_t* get_mutex(GLenum buffer_enum) {
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
					static thread_mutex_t _map_buffer_mutex;
					const GLenum target;
					std::unique_lock<thread_mutex_t> map_buffer_lock;
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
				buffer_bind_t(GLuint buffer_id, GLenum target, thread_mutex_t& mutex, GLsizei& parent_size) : buffer_id(buffer_id), target(target), _lock(mutex), parent_size(parent_size) {
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
				std::unique_lock<thread_mutex_t> _lock;
			};
			
			buffer_t() : buffer_t(GL_ARRAY_BUFFER) {
			}
			buffer_t(GLenum target)  {
				glGenBuffers(1, &_buffer_id);
				set_target(target);
			}
			buffer_t(const buffer_t&) = delete;
			buffer_t(buffer_t&& other) : _target(other._target), _target_mutex(other._target_mutex),
				_buffer_id(other._buffer_id), _size(other._size) {
				other._buffer_id = 0;
				other._target = 0;
				other._target_mutex = nullptr;
				other._size = 0;
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
			buffer_bind_t bind() {
				return buffer_bind_t(_buffer_id, _target, *_target_mutex, _size);
			}
			buffer_bind_t bind(GLenum target) {
				set_target(target);
				return bind();
			}
			template<class T>
			void upload_objects(const T* data, size_t count, GLenum usage = GL_STATIC_DRAW) {
				auto b = bind();
				b.upload_data(reinterpret_cast<const byte_t*>(data), sizeof(T)*count, usage);
			}
			const GLint size() const {
				return _size;
			}
		private:
			GLsizei _size = 0;
			GLuint _buffer_id = 0;
			GLenum _target = 0;
			thread_mutex_t* _target_mutex = nullptr;
		};
		template<class T>
		struct vbo_t : buffer_t {
			static constexpr GLint amount_per_element() {
				return std::get<1>(gl_type_pair());
			}
			static constexpr GLenum gl_type() {
				return std::get<0>(gl_type_pair());
			}
			static constexpr std::pair<GLenum, GLint> gl_type_pair() {
				return T_gl_type<T>();
			}
			void upload(const T* data, GLsizei count, GLenum usage = GL_STATIC_DRAW) {
				bind().upload_data(reinterpret_cast<const uint8_t*>(data), sizeof(T)*count, usage);
			}
			vbo_t() : buffer_t(GL_ARRAY_BUFFER) {
			}
			vbo_t(const T* data, GLsizei count, GLenum usage) : vbo_t() {
				upload(data, count, usage);
			}
			vbo_t(std::initializer_list<T> list, GLenum usage = GL_STATIC_DRAW) : vbo_t() {
				std::vector<T> buffer(list);
				upload(buffer.data(), static_cast<GLsizei>(buffer.size()), usage);
			}
			
		};
		struct ebo_t : buffer_t {

		};
		static_assert(sizeof(buffer_t) == sizeof(vbo_t<float>));
		struct vao_t {
			struct vertex_attribute_location_t {
				GLuint offset;
				GLuint index;
				GLuint stride;
				std::pair<GLenum, GLuint> gl_type_info;
			};
			struct vertex_attribute_storage_location_t : buffer_t, vertex_attribute_location_t {

			};
			template<class T>
			struct vertex_attribute_buffer_object_t : vbo_t<T>, vertex_attribute_location_t {
				vertex_attribute_buffer_object_t(vbo_t<T>&& vbo, const vertex_attribute_location_t& location)
					: vbo_t<T>(std::move(vbo)), vertex_attribute_location_t(location) {
					gl_type_info = vbo_t<T>::gl_type_pair();
				}
			};
			template<class T>
			struct vertex attribute_element_object_t : ebo_t<T>, vertex_attribute_lock_t {

			}

			struct vao_bind_t {
				static thread_mutex_t _mutex;
				vao_bind_t(GLuint id, vao_t& parent) : _id(id), _lock(_mutex), _parent(parent) {
					glBindVertexArray(id);
				}
				vao_bind_t(const vao_bind_t&) = delete;
				vao_bind_t(vao_bind_t&& other) : _id(other._id), _lock(std::move(other._lock)), _parent(other._parent) {
					other._id = 0;
				}
				~vao_bind_t() {
					glBindVertexArray(0); //Unbind itself (maybe not needed)
				}
				operator GLuint() const {
					return _id;
				}
				template<class T>
				void assign_vertex_attribute(vertex_attribute_buffer_object_t<T>& va) {
					auto bind = va.bind();
					uint64_t offset = va.offset;
					glVertexAttribPointer(va.index, va.amount_per_element(), va.gl_type(), GL_FALSE, va.stride, ((const void*)offset)); //cast to void point is on purpose
					glEnableVertexAttribArray(va.index);
					GL_MAX_VERTEX_ATTRIBS;
					check_gl_errors("after assigning vertex attribute");
				}

				template<class T, class... Args>
				vertex_attribute_buffer_object_t<T>& emplace_vertex_attribute(GLuint index, GLuint stride, GLuint offset, Args&&... vbo_args) {
					{
						vertex_attribute_buffer_object_t<T> va = { {vbo_t<T>(std::forward<Args>(vbo_args)...)}, {index, stride, offset } };
						static_assert(sizeof(vertex_attribute_storage_location_t) == sizeof(vertex_attribute_buffer_object_t<T>), "vbo types need to be same size/layout so we can reinterupt_cast");
						vertex_attribute_storage_location_t& location = *reinterpret_cast<vertex_attribute_storage_location_t*>(&va);
						_parent._buffers.emplace_back(std::move(location));

					} //vbo is no longer valid
					vertex_attribute_buffer_object_t<T>& va = *reinterpret_cast<vertex_attribute_buffer_object_t<T>*>(&*(_parent._buffers.end() - 1));
					assign_vertex_attribute(va);
					return va;
				}
				template<class T> vertex emplace_ebo() {

				}
				vao_t& _parent;
				GLuint _id;
				std::unique_lock<thread_mutex_t> _lock;
			};

			vao_bind_t bind() {
				return vao_bind_t(_id, *this);
			}
			vao_t() {
				glGenVertexArrays(1, &_id);
			}
		private:
			std::vector<vertex_attribute_storage_location_t> _buffers;
			GLuint _id;
			GLenum _draw_shapes = GL_TRIANGLES;
			
		};
		struct texture_t {
			struct texture_bind_t {

				texture_bind_t(texture_t& parent) : _parent(parent), _lock(_mutex) {
					glBindTexture(_target, _parent._id);
				}
				bool valid() const {
					return _lock.owns_lock();
				}
				~texture_bind_t() {
					if (valid())
						glBindTexture(_target, 0);
				}
				texture_t& parent() {
					return _parent;
				}
				void upload(const byte_t* pixels, GLsizei width, GLsizei height, GLint internal_format = GL_RGB, GLint format = GL_RGB, GLenum type = GL_FLOAT) {
					glTexImage2D(_target, 0, internal_format, width, height, 0, format, type, pixels);
					_parent.width() = width;
					_parent.height() = height;
				}
			protected:
				texture_bind_t(texture_t& parent, std::unique_lock<thread_mutex_t> lock) :
					_parent(parent), _lock(std::move(lock)) {
					if constexpr (_DEBUG) {
						if (lock.mutex != &_mutex)
							throw gl_error_t(0, "texture bind created from invalid mutex");
					}
				}

				static constexpr GLenum _target = GL_TEXTURE_2D;
				texture_t& _parent;
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
				auto lock = activate_texture_unit(texture_unit);
				glBindTexture(GL_TEXTURE_2D, _id);
				return texture_bind_t(*this, std::move(lock));
			}
			~texture_t() {
				if (valid()) {
					glDeleteTextures(1, &_id);
					_id = 0;
				}
			}
			static std::unique_lock<thread_mutex_t> activate_texture_unit(GLsizei texture_unit) {
				if (texture_unit > 32)
					throw gl_error_t(texture_unit, "texture_unit too high");
				auto lock = std::unique_lock<foton::thread_mutex_t>(texture_bind_t::_mutex);
				glActiveTexture(GL_TEXTURE + texture_unit);
				return lock;
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