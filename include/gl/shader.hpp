#pragma once
#include <string>
#include <stdexcept>
#include <shared_mutex>
#include <filesystem>
#include <fstream>
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
		class shader_t {
			struct unknown_uniform_error_t : std::logic_error {
				unknown_uniform_error_t(const char* uniform_error) : logic_error(uniform_error) {}
			};
			struct uniform_location_t {
				//TODO: all the other glUniform functions
				const GLuint program;
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
				}
			};
			//TODO: more specializations

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
			private:
				static std::mutex _master_shader_mutex;
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
			};
			shader_t(const char* vertex_source, const char* fragment_source, const char* geometry_source) :
				shader_t(load_shader(vertex_source, GL_VERTEX_SHADER), load_shader(fragment_source, GL_FRAGMENT_SHADER), load_shader(geometry_source, GL_GEOMETRY_SHADER)) {};
			shader_t(const shader_t&) = delete;
			shader_t(shader_t&& other) : id(other.id) {
				other.id = INVALID_SHADER_ID;
			}
			~shader_t() {
				if (id > 0) {
					glDeleteProgram(id);
				}
			}
			template<class T>
			uniform_t<T> get_uniform(const char* name) {
				GLint uniform_location = glGetAttribLocation(id, name);
				if (uniform_location == GL_INVALID_OPERATION)
					throw shader_error_t("unable to get uniform location");
				return uniform_t<T>{ id, uniform_location };
			}
			shader_bind_t use() {
				return shader_bind_t(id);
			}
			static shader_t load_shader_from_known_paths(const filesystem::path& vertex_path, const filesystem::path& fragment_path, const filesystem::path& geometry_path) {
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
			static shader_t load_shader_from_paths(std::initializer_list<const filesystem::path> paths) {
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
				return load_shader_from_known_paths(vertex_path, fragment_path, geometry_path);
			}
		};
	}
}
std::mutex foton::shader::shader_t::shader_bind_t::_master_shader_mutex;