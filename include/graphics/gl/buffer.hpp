#pragma once
#include "../../mutex.hpp"
#include "../../exceptions.hpp"
#include "glew/glew.h"
#include <stdexcept>
#include <string>
namespace foton {
	namespace GL {
		void check_gl_errors(const char* message) {
			if constexpr (_DEBUG) { //Should probably use a macro but this is cleaner for now
				auto err = glGetError();
				if (err != GL_NO_ERROR) {
					throw exceptions::gl_error_t(err, message);
				}
			}
		}
		void clear_gl_errors(bool throw_on_no_error = true) {
			auto err = glGetError();
			if (err != GL_NO_ERROR && throw_on_no_error) {
				throw exceptions::gl_error_t(err, "clear_gl_errors called while no errors");
			}
			while (err != GL_NO_ERROR)
				glGetError();
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
					operator T* () {
						return ptr;
					}
					operator T* () const {
						return ptr;
					}
				};

				buffer_bind_t(buffer_t& parent) : _lock(*parent._target_mutex), _parent(&parent) {
					glBindBuffer(target(), buffer_id());
				};
				buffer_bind_t(const buffer_bind_t&) = delete;
				buffer_bind_t(buffer_bind_t&& other) : _lock(std::move(other._lock)), _parent(other._parent) {
					other._parent = nullptr;
				}
				~buffer_bind_t() {
					if (buffer_id() > 0) {
						glBindBuffer(target(), 0); //unbind itself
					}
				}
				GLsizei buffer_id() const {
					return parent().buffer_id();
				}
				GLenum target() const {
					return parent().target();
				}
				void upload_data(const byte_t* data, size_t size_in_bytes, GLenum usage = GL_STATIC_DRAW) {
					glBufferData(_parent->_target, size_in_bytes, data, usage);
					_parent->_size = static_cast<GLsizei>(size_in_bytes);
				}
				buffer_t& parent() {
					return *_parent;
				}
				const buffer_t& parent() const {
					return *_parent;
				}
				GLsizei size() const {
					return parent().size();
				}
			protected:
				buffer_t* _parent;
				std::unique_lock<thread_mutex_t> _lock;
			};

			buffer_t() : buffer_t(GL_ARRAY_BUFFER) {
			}
			buffer_t(GLenum target) {
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
			~buffer_t() {
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
				return buffer_bind_t(*this);
			}
			buffer_bind_t bind(GLenum target) {
				set_target(target);
				return bind();
			}
			template<class T>
			void upload_objects(const T* data, size_t count, GLenum usage = GL_STATIC_DRAW) {
				auto b = bind();
				b.upload_data(reinterpret_cast<const byte_t*>(data), sizeof(T) * count, usage);
			}
			const GLint size() const {
				return _size;
			}
			GLenum target() const {
				return _target;
			}
		protected:
			GLsizei _size = 0;
			GLuint _buffer_id = 0;
			GLenum _target = 0;
			thread_mutex_t* _target_mutex = nullptr;
		};
		template<class T>
		struct typed_buffer_t : buffer_t {
			static_assert(std::is_trivial_v<T>, "only trival objects can be sent to the GPU");
			typed_buffer_t(GLenum target) : buffer_t(target) {}
			typed_buffer_t(GLenum target, const T* data, GLsizei count, GLenum usage) : typed_buffer_t(target) {
				upload(data, count, usage);
			}
			static constexpr GLint amount_per_element() {
				return std::get<1>(gl_type_pair());
			}
			static constexpr GLenum gl_type() {
				return std::get<0>(gl_type_pair());
			}
			static constexpr std::pair<GLenum, GLint> gl_type_pair() {
				return T_gl_type<T>();
			}
			static constexpr size_t bytes_per_element() {
				return sizeof(T) * amount_per_element();
			}
			size_t count() const {
				return size() / bytes_per_element();
			}
			void upload(const T* data, GLsizei count, GLenum usage = GL_STATIC_DRAW) {
				bind().upload_data(reinterpret_cast<const uint8_t*>(data), sizeof(T) * count, usage);
			}
		private:

		};
	}
}