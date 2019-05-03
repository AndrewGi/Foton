#pragma once
#include "../types.hpp"
	namespace foton {
	struct drawer_t {
		bool is_visable = true;
		void draw(const mat4f& transformation) {
			if (is_visable)
				draw_visable(transformation);
		}
	private:
		virtual void draw_visable(const mat4f& transformation) = 0;
	};
}
