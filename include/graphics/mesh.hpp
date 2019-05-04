#pragma once
#include <vector>
#include "texture.hpp"
#include "gl/gl_objects.hpp"
namespace foton {
	struct mesh_t {
		using index_t = uint32_t;
		std::vector<vertex_t> vertices;
		std::vector<index_t> indices;
		std::vector<texture_t> textures;
		GL::vao_t vao;
	private:
		void enable_normals() {

		}
		void draw_visable(const drawer_context_t& context) override {

		}
	};
}