#pragma once
#include <string>
#include <stdexcept>
#include "GL/glew.h"
namespace foton {
	namespace shader {
		struct shader_error_t : std::logic_error {
			shader_error_t(std::string msg) : std::logic_error(msg) {};
		};
		using vertex_shader_t = GLuint;
		using fragment_shader_t = GLuint;
		vertex_shader_t load_vertex_shader(std::string code) {
			vertex_shader_t vertex_shader = glCreateShader(GL_VERTEX_SHADER);
			const char* p = code.c_str();
			glShaderSource(vertex_shader, 1, &p, nullptr);
			glCompileShader(vertex_shader);
			GLint success = 0;
			glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
			if (!success)
				throw shader_error_t("vertex shader compile failed");
			return vertex_shader;
		}
		fragment_shader_t load_fragment_shader(std::string code) {
			fragment_shader_t fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
			const char* p = code.c_str();
			glShaderSource(fragment_shader, 1, &p, nullptr);
			glCompileShader(fragment_shader);
			GLint success = 0;
			glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
			if (!success)
				throw shader_error_t("fragment shader compile failed");
			return fragment_shader;
		}
	}
}