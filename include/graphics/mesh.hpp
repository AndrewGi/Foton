#pragma once
#include <vector>
#include "gl/texture.hpp"
#include "drawer.hpp"
namespace foton {
	struct mesh_t {
		using index_t = uint32_t;
		std::vector<vertex_t> vertices;
		std::vector<index_t> indices;
		std::vector<GL::texture_t> textures;
		
		GL::vao_t vao;
	private:
	};
}