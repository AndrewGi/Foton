#pragma once
#include <vector>
#include "texture.hpp"
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
					texture.activate(index);
				});
				GL::texture_t::activate_texture_unit(0);
			}
		}
	private:
		void enable_normals() {

		}
		void draw_visable(const drawer_context_t& context) override {
			activate_textures();

		}
	};
}