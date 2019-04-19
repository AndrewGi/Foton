#pragma once
#include <string>
#include <stdexcept>
#include <shared_mutex>
#include <filesystem>
#include <fstream>
#include "GL/glew.h"
namespace foton {
	namespace shader {
		namespace filesystem = std::experimental::filesystem; //why is this still in experimental?
		struct shader_error_t : std::logic_error {
			shader_error_t(std::string msg) : std::logic_error(msg) {};
		};
		struct file_not_found_error_t : std::exception {
			file_not_found_error_t(const filesystem::path& path) : std::exception(path.string()) {}
		};
		using vertex_shader_t = GLuint;
		using fragment_shader_t = GLuint;
		using geometry_shader_t = GLuint;
		static constexpr GLuint invalid_shader = static_cast<GLuint>(-1);
		vertex_shader_t load_vertex_shader(const char* code) {
			vertex_shader_t vertex_shader = glCreateShader(GL_VERTEX_SHADER);
			glShaderSource(vertex_shader, 1, &code, nullptr);
			glCompileShader(vertex_shader);
			GLint success = 0;
			glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
			if (!success)
				throw shader_error_t("vertex shader compile failed");
			return vertex_shader;
		}
		fragment_shader_t load_fragment_shader(const char* code) {
			fragment_shader_t fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(fragment_shader, 1, &code, nullptr);
			glCompileShader(fragment_shader);
			GLint success = 0;
			glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
			if (!success)
				throw shader_error_t("fragment shader compile failed");
			return fragment_shader;
		}
		geometry_shader_t load_geometry_shader(const char* code) {
			geometry_shader_t geometry_shader = glCreateShader(GL_GEOMETRY_SHADER);
			glShaderSource(geometry_shader, 1, &code, nullptr);
			glCompileShader(geometry_shader);
			GLint success = 0;
			glGetShaderiv(geometry_shader, GL_COMPILE_STATUS, &success);
			if (!success)
				throw shader_error_t("geometry shader compile failed");
			return geometry_shader;
		}
		class shader_t {
			static std::mutex _master_shader_mutex;
			static constexpr GLuint INVALID_SHADER_ID = static_cast<GLuint>(-1);
			GLuint id = INVALID_SHADER_ID;
			shader_t(vertex_shader_t vertex_shader, fragment_shader_t fragment_shader, geometry_shader_t geometry_shader) {
				id = glCreateProgram();
				glAttachShader(id, vertex_shader);
				glAttachShader(id, fragment_shader);
				if (geometry_shader != invalid_shader)
					glAttachShader(id, geometry_shader);
				glLinkProgram(id);
				//the shaders should be linked to the program so we can release our hold on the memory
				glDeleteShader(vertex_shader);
				glDeleteShader(fragment_shader);
				if (geometry_shader != invalid_shader)
					glDeleteShader(geometry_shader);
			};
			shader_t(const shader_t&) = delete;
			shader_t(shader_t&& other) : id(other.id) {
				other.id = INVALID_SHADER_ID;
			}
			~shader_t() {
				if (id > 0) {
					glDeleteProgram(id);
				}
			}
		public:
			static shader_t load_shader(const char* vertex_source, const char* fragment_source, const char* geometery_source = nullptr) {
				return shader_t(load_vertex_shader(vertex_source), load_fragment_shader(fragment_source), geometery_source == nullptr ? invalid_shader : load_geometry_shader(geometery_source));
			}
			static shader_t load_shader_from_path(const filesystem::path& vertex_path, const filesystem::path& fragment_path, const filesystem::path& geometry_path) {
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
				std::string geometry_source = load_file(geometry_source);
				return load_shader(vertex_source.empty() ? nullptr : vertex_source.c_str(),
					fragment_source.empty() ? nullptr : fragment_source.c_str(),
					geometry_source.empty() ? nullptr : geometry_source.c_str());
			}
		};
	}
}
std::mutex foton::shader::shader_t::_master_shader_mutex;