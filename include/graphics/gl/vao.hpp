#pragma once
#include "vbo.hpp"
#include "../../containers/dynamic_vector.hpp"
namespace foton::GL {
		struct vao_t {
			struct vertex_attribute_location_t {
				const GLuint offset;
				const GLuint index;
				const GLuint stride;
				static_assert(std::is_trivial_v<vertex_attribute_location_t>);
			};
			struct ebo_info_t {

			};
			struct vao_buffer_info_t {
				GLenum draw_mode;
				union {
					ebo_info_t ebo;
					vertex_attribute_location_t vertex_attribute;
				} info;
			};

			struct vao_any_buffer_t : buffer_t, vao_buffer_info_t {

			};
			template<class T>
			struct vao_buffer_t : typed_buffer_t<T>, vao_buffer_info_t {
				static_assert(sizeof(*this) == sizeof(vao_any_buffer_t));
			};
			template<class T>
			struct vertex_attribute_buffer_object_t : vao_buffer_t<T> {
				vertex_attribute_buffer_object_t(vbo_t<T>&& vbo, const vertex_attribute_location_t& location)
					: vao_buffer_t<T>{ vbo_t<T>(std::move(vbo)), vertex_attribute_location_t{
					location.offset, location.index, location.stride } } {

				}
			};

			template<class T>
			struct ebo_t : typed_buffer_t<T>, vao_buffer_info_t {
				struct ebo_bind_t : buffer_t::buffer_bind_t {
					ebo_bind_t(buffer_t::buffer_bind_t&& b_bind)
						: buffer_bind_t(std::move(b_bind)) {}
					ebo_t& parent() {
						return *static_cast<ebo_t*>(_parent);
					}
					void draw(GLsizei element_count, GLsizei offset = 0) {
						if ((element_count + offset) > parent().count())
							throw ("elements out of range");
						glDrawElements(parent().draw_mode, element_count, typed_buffer_t<T>::gl_type(), (const void*)(offset));
					}
				};
				
				ebo_bind_t bind() {
					return typed_buffer_t<T>::bind();
				}
			};
			static_assert(sizeof(ebo_t<float>) == sizeof(vao_any_buffer_t));

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
					if (_id != 0 && _lock.owns_lock())
						glBindVertexArray(0); //Unbind itself (maybe not needed)
				}
				operator GLuint() const {
					return _id;
				}
				template<class T>
				void assign_vertex_attribute(vertex_attribute_buffer_object_t<T>& va) {
					uint64_t offset = va.offset;
					auto bind = va.bind();
					glVertexAttribPointer(va.index, va.amount_per_element(), va.gl_type(), GL_FALSE, va.stride, ((const void*)offset)); //cast to void point is on purpose
					glEnableVertexAttribArray(va.index);
					check_gl_errors("after assigning vertex attribute");
				}

				template<class T, class... Args>
				vertex_attribute_buffer_object_t<T>& emplace_vertex_attribute(GLuint index, GLuint stride, GLuint offset, Args&& ... vbo_args) {
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
				template<class T, class... Args> ebo_t<T>& emplace_ebo(Args&& ... args) {
					{
						ebo_t<T> ebo = { std::forward<Args>(args)..., {} };
						static_assert(sizeof(vertex_attribute_storage_location_t) == sizeof(ebo_t<T>), "ebo types need to be same size/layout so we can reinterupt_cast");
						vertex_attribute_storage_location_t& location = *reinterpret_cast<vertex_attribute_storage_location_t*>(&ebo);
						_parent._buffers.emplace_back(std::move(location));
					}
					return *reinterpret_cast<ebo_t<T>*>(&*(_parent._buffers.end() - 1));
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
			dynamic_vector_t<vao_any_buffer_t> _buffers;
			GLuint _id;
			GLenum _draw_shapes = GL_TRIANGLES;


		};
	}
