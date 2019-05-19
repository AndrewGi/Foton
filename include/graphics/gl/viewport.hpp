#pragma once
#include "../../types.hpp"
#include "../../glew/glew.h"
namespace foton::GL {
	struct viewport_t {
		GLint x = 0;
		GLint y = 0;
		GLsizei width = 0;
		GLsizei height = 0;
		void apply() {
			glViewport(x, y, width, height);
		}
	};
}