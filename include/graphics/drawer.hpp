#pragma once
#include "../types.hpp"
#include "gl/viewport.hpp"
namespace foton {
	struct drawable_t {
		struct context_t {
			const GL::viewport_t viewport;
			//TODO: any drawing context info gets stored here
		};
		virtual void draw_call(const context_t&) = 0;
	};
}
