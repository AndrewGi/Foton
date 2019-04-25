#pragma once
#include <string>
#include <stdexcept>
#include <shared_mutex>
#include <filesystem>
#include <fstream>
#include "../drawer.hpp"
#include "glew/glew.h"
namespace foton {
	namespace shader {
		namespace filesystem = std::experimental::filesystem; //why is this still in experimental?
		struct shader_error_t : std::logic_error {
			shader_error_t(std::string msg) : std::logic_error(msg) {};
		};
		struct file_not_found_error_t : std::runtime_error {
			file_not_found_error_t(const filesystem::path& path) : std::runtime_error(path.string()) {}
			file_not_found_error_t(const char* msg) : std::runtime_error(msg) {}
		};

		struct uniform_location_t {
			//TODO: all the other glUniform functions
			//TODO: update on reload
			const GLuint& program;
			const GLint location;
		};
		template<class T>
		struct uniform_t {
			static_assert(sizeof(T) == 0, "not implemented uniform type");
		};
		template<>
		struct uniform_t<float> : uniform_location_t {
			explicit operator float() {
				float out = 0.f;
				glGetUniformfv(program, location, &out);
				return out;
			}
			float operator=(float x) {
				glProgramUniform1f(program, location, x);
				return x;
			}

		};
		template<>
		struct uniform_t<int> : uniform_location_t {
			explicit operator int() {
				int out = 0;
				glGetUniformiv(program, location, &out);
				return out;
			}
			int operator=(int x) {
				glProgramUniform1i(program, location, x);
				return x;
			}
		};
		//TODO: more specializations
		class shader_t {
		public:
			struct unknown_uniform_error_t : std::logic_error {
				unknown_uniform_error_t(const char* uniform_error) : logic_error(uniform_error) {}
			};

			struct shader_bind_t {
				const GLuint program = 0;
				shader_bind_t(GLuint program) : program(program), lock(_master_shader_mutex) {
					glUseProgram(program);
				}
				shader_bind_t(const shader_bind_t&) = delete;
				shader_bind_t(shader_bind_t&& other) : program(other.program), lock(std::move(other.lock)) {

				}
				~shader_bind_t() {
					if (lock.owns_lock())
						glUseProgram(0);
				}
				static std::mutex _master_shader_mutex;
			private:
				std::unique_lock<std::mutex> lock;
			};
			static constexpr GLuint INVALID_SHADER_ID = 0;

			static GLuint load_shader(const char* code, GLenum which_shader) {
				auto guess_shader = [](GLenum which_shader) {
					switch (which_shader){
					case GL_VERTEX_SHADER:
						return "vertex_shader";
					case GL_FRAGMENT_SHADER:
						return "fragment_shader";
					case GL_GEOMETRY_SHADER:
						return "geometry_shader";
					default:
						return "unknown_shader";
					}
				};
				if (code == nullptr) {
					return INVALID_SHADER_ID;
				}
				GLuint shader = glCreateShader(which_shader);
				glShaderSource(shader, 1, &code, nullptr);
				glCompileShader(shader);
				GLint success = 0;
				glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
				if (!success)
					throw shader_error_t(std::string("shader compile failed for ")+guess_shader(which_shader));
				return shader;
			}

			GLuint id = INVALID_SHADER_ID;
		public:
			shader_t(GLuint vertex_shader, GLuint fragment_shader, GLuint geometry_shader) {
				if (vertex_shader == INVALID_SHADER_ID || fragment_shader == INVALID_SHADER_ID) {
					throw shader_error_t("shader requires a valid vertex AND fragment shader atleast");
				}
					
				id = glCreateProgram();
				glAttachShader(id, vertex_shader);
				glAttachShader(id, fragment_shader);
				if (geometry_shader != INVALID_SHADER_ID)
					glAttachShader(id, geometry_shader);
				glLinkProgram(id);
				//the shaders should be linked to the program so we can release our hold on the memory
				glDeleteShader(vertex_shader);
				glDeleteShader(fragment_shader);
				if (geometry_shader != INVALID_SHADER_ID)
					glDeleteShader(geometry_shader);

				if (auto err = glGetError(); err != GL_NO_ERROR)
					throw shader_error_t(std::string("glError after shader_t construction: ") + std::to_string(err));
			};
			shader_t(const char* vertex_source, const char* fragment_source, const char* geometry_source) :
				shader_t(load_shader(vertex_source, GL_VERTEX_SHADER), load_shader(fragment_source, GL_FRAGMENT_SHADER), load_shader(geometry_source, GL_GEOMETRY_SHADER)) {};
			shader_t(const shader_t&) = delete;
			shader_t operator=(const shader_t&) = delete;
			shader_t(shader_t&& other) : id(other.id) {
				other.id = INVALID_SHADER_ID;
			}
			shader_t operator=(shader_t&& other) {
				std::swap(*this, other);
			}
			void update_from(shader_t&& other) {
				auto lock = std::unique_lock<std::mutex>(shader_bind_t::_master_shader_mutex);
				if (id > 0) {
					glDeleteProgram(id);
				}
				id = other.id;
				other.id = 0;
			}
			~shader_t() {
				delete_program();
			}
			template<class T>
			uniform_t<T> get_uniform(const char* name) {
				GLint uniform_location = glGetUniformLocation(id, name);
				if (uniform_location == -1)
					throw shader_error_t("unable to get uniform location");
				return uniform_t<T>{ id, uniform_location };
			}
			shader_bind_t use() {
				return shader_bind_t(id);
			}
			struct shader_drawer_wrapper_t : drawer_t {
				shader_t& parent;
				drawer_t& drawer;
				shader_drawer_wrapper_t(shader_t& shader, drawer_t& drawer) : parent(shader), drawer(drawer) {}
				void draw() override {
					auto shader_bind = parent.use();
					drawer.draw();
				}
			};
			shader_drawer_wrapper_t wrap(drawer_t& drawer) {
				return shader_drawer_wrapper_t(*this, drawer);
			}
			private:
				void delete_program() {

					if (id > 0) {
						auto lock = std::unique_lock<std::mutex>(shader_bind_t::_master_shader_mutex);
						glDeleteProgram(id);
						id = INVALID_SHADER_ID;
					}
				}
		};
		struct shader_with_paths_t {
			const filesystem::path vertex_path;
			const filesystem::path fragment_path;
			const filesystem::path geometry_path;
			shader_with_paths_t(const filesystem::path& vertex_path, const filesystem::path& fragment_path, const filesystem::path& geometry_path) :
			 vertex_path(vertex_path), fragment_path(fragment_path), geometry_path(geometry_path), _shader(load_new_shader()) {
			}
			void reload_shader() {
				_shader.update_from(load_new_shader());
			}
			static shader_with_paths_t guess_filetypes(std::initializer_list<const filesystem::path> paths) {
				using path = filesystem::path;
				auto find_path = [&](std::string extension, bool no_throw = false) {
					for (const path& p : paths) {
						if (p.extension() == extension)
							return p;
					}
					if (no_throw)
						return path();
					throw file_not_found_error_t((extension + " shader file not found").c_str());
				};
				const path vertex_path = find_path(".vert");
				const path fragment_path = find_path(".frag");
				const path geometry_path = find_path(".geom", true);
				return shader_with_paths_t(vertex_path, fragment_path, geometry_path);
			}
			shader_t& shader() {
				return _shader;
			}
		private:
			shader_t load_new_shader() {
				if (auto err = glGetError(); err != GL_NO_ERROR)
					throw shader_error_t(std::string("trying to load new shader while glError is ") + std::to_string(err));
			
				auto load_file = [](const filesystem::path& filename) {
					if (filename.empty()) {
						return std::string();
					}
					else {
						std::ifstream input(filename);
						if (input.fail()) {
							throw file_not_found_error_t(filename);
						}
						using file_iter = std::istreambuf_iterator<char>;
						return std::string(file_iter(input), file_iter());
					}
				};
				std::string vertex_source = load_file(vertex_path);
				std::string fragment_source = load_file(fragment_path);
				std::string geometry_source = load_file(geometry_path);
				return shader_t(vertex_source.empty() ? nullptr : vertex_source.c_str(),
					fragment_source.empty() ? nullptr : fragment_source.c_str(),
					geometry_source.empty() ? nullptr : geometry_source.c_str());
			}
			shader_t _shader;
		};
	}
}
std::mutex foton::shader::shader_t::shader_bind_t::_master_shader_mutex;