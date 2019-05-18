#pragma once
#include <vector>
#include "gl/gl_objects.hpp"
namespace foton {
	struct mesh_t {
		using index_t = uint32_t;
		std::vector<vertex_t> vertices;
		std::vector<index_t> indices;
		std::vector<GL::texture_t> textures;
		
		GL::vao_t vao;
		void activate_textures() {
			if (!textures.empty()) {
				std::for_each_n(textures.begin(), textures.end(),
					[](GL::texture_t texture, GLsizei index) {
					texture.activate(index).dont_unbind();
				});
				GL::texture_t::texture_bind_t::activate_unit(0);
			}1
		}
	private:
		void draw_call() override {
			auto b = vao.bind();
			activate_textures();
			vao.draw_call();

		}
	};
}