#pragma once
#include <string>
#include <string_view>
#include <fstream>
#include <filesystem>
#include "types.hpp"
namespace foton {
	namespace model {
		namespace filesystem = std::filesystem;
		using index_t = uint32_t;
		static constexpr index_t INVALID_INDEX = static_cast<index_t>(-1);
		struct model_t {
			std::vector<vec3f> vertices;
			std::vector<vec2f> texture_coords;
			std::vector<vec3f> normals;
			std::vector<uint32_t> indices;
		};
		class multiindex_model_t {
		public:
			struct model_loading_error_t : std::runtime_error {
				model_loading_error_t(std::string message) : std::runtime_error(message) {};
			};
			struct multiindex_face_t {
				struct vertex_indices_t {
					size_t vertex_i, texture_coords_i, normals_i;
				};
				vertex_indices_t v1, v2, v3;

			};
			std::vector<vec3f> vertices;
			std::vector<vec3f> normals;
			std::vector<vec2f> texture_coords;
			std::vector<multiindex_face_t> faces;
			index_t add_vertex_indices(model_t& model, multiindex_face_t::vertex_indices_t v) {
				model.vertices.push_back(vertices[v.vertex_i]);
				model.texture_coords.push_back(texture_coords[v.texture_coords_i]);
				model.normals.push_back(normals[v.normals_i]);
				return static_cast<index_t>(model.vertices.size() - 1);
			}
			index_t find_similar_vertex(model_t& model, vec3f vertex, vec2f uv, vec3f normal) {
				for (index_t i = 0; i < model.vertices.size(); i++) {
					if (model.vertices[i] == vertex
						&& model.texture_coords[i] == uv
						&& model.normals[i] == normal)
						return i;
				}
				return INVALID_INDEX;
			}
			model_t make_model() {
				model_t out;
				auto process_vertex_indices = [&](multiindex_face_t::vertex_indices_t v) {
					if (const index_t i = find_similar_vertex(out, vertices[v.vertex_i], texture_coords[v.texture_coords_i],
						normals[v.normals_i]); i != INVALID_INDEX) {
						out.indices.push_back(i);
					}
					else {
						out.indices.push_back(add_vertex_indices(out, v));
					}
				};
				auto process_multiindex_face = [&](multiindex_face_t face) {
					process_vertex_indices(face.v1);
					process_vertex_indices(face.v2);
					process_vertex_indices(face.v3);
				};
				std::for_each(faces.begin(), faces.end(), process_multiindex_face);
			}
		};

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
		std::istream& operator>>(std::istream& is, foton::model::multiindex_model_t::multiindex_face_t::vertex_indices_t& out) {
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
		std::istream& operator>>(std::istream& is, foton::model::multiindex_model_t::multiindex_face_t& out) {
			return is >> out.v1 >> out.v2 >> out.v3;
		}
		class OBJ_model_t : public multiindex_model_t {
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
						multiindex_face_t face;
						in >> face;
						faces.push_back(face);
					}
				}
			}
			static OBJ_model_t from_path(const filesystem::path& path) {
				std::ifstream stream = std::ifstream(path);
				return OBJ_model_t(stream);
			}

		};
	}

}