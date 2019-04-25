#pragma once
#include <string>
#include <string_view>
#include <sstream>
#include "2D.hpp"
#include "3D.hpp"
namespace foton {
	using namespace gfx_3D;
	using namespace gfx_2D;
	namespace model {
		class model_t {
		public:
			struct model_loading_error_t : std::runtime_error {
				model_loading_error_t(std::string message) : std::runtime_error(message) {};
			};
			struct face_t {
				struct vertex_indices_t {
					size_t vertex_i, texture_coords_i, normals_i;
				};
				vertex_indices_t v1, v2, v3;
			};
			std::vector<vec3f> vertices;
			std::vector<vec2f> texture_coords;
			std::vector<vec3f> normals;
			std::vector<face_t> faces;
		};
		class OBJ_model_t : public model_t {
			std::string obj_file_name;
			OBJ_model_t(std::istream& in) {
				std::string operation;
				auto pass = [&] {
					while (in.get() != '\n') {};
				};
				while (in >> operation) {
					if (operation == "#") { //Comment
						pass();
						continue;
					}
					else if (operation == "o") { //Object name
						in >> obj_file_name;
						continue;
					}
					else if (operation == "mtllib") { //Unused
						pass();
						continue;
					}
					else if (operation == "v") {
						vec3f vertex;
						in >> vertex;
						vertices.push_back(vertex);
					}
					else if (operation == "vt") {
						vec2f texture_coord;
						in >> texture_coord;
						texture_coords.push_back(texture_coord);
					}
					else if (operation == "vn") {
						vec3f normal;
						in >> normal;
						normals.push_back(normal);
					}
					else if (operation == "f") {
						face_t face;
						in >> face;
						faces.push_back(face);
					}
				}
			}

		};
	}

}
std::istream& operator>>(std::istream& is, foton::vec3f& out) {
	foton::vec3f::value_type x, y, z;
	is >> x >> y >> z;
	out = foton::vec3f{ x, y, z };
	return is;
}
std::istream& operator>>(std::istream& is, foton::vec2f& out) {
	foton::vec2f::value_type x, y;
	is >> x >> y;
	out = foton::vec2f{ x, y };
	return is;
}
std::istream& operator>>(std::istream& is, foton::model::model_t::face_t::vertex_indices_t& out) {
	is >> out.vertex_i;
	if (is.peek() != '/')
		return is;
	is.get();
	if (is.peek() != '/')
		is >> out.texture_coords_i;
	is.get();
	if (is.peek() != '/')
		return is;
	is >> out.normals_i;
	return is;
}
std::istream& operator>>(std::istream& is, foton::model::model_t::face_t& out) {
	return is >> out.v1 >> out.v2 >> out.v3;
}