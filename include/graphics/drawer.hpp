#pragma once
#include "../types.hpp"
	namespace foton {
		struct drawer_context_t {
			const mat4f& view_matrix;
			const mat4f& projection_matrix;
		};
		struct drawer_t {
			bool is_visable = true;
			void draw(const drawer_context_t& context) {
				if (is_visable)
					draw_visable(context);
			}
		private:
			virtual void draw_visable(const drawer_context_t& context) = 0;
	};
}
