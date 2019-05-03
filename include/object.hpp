#pragma once
#include <vector>
#include "graphics/drawer.hpp"
#include "graphics/gl/shader.hpp"
#include "model.hpp"
namespace foton {
	struct object_t : drawer_t {
		struct optional_shader_t {
			shader::shader_t shader;
			shader::uniform_t<mat4f> transform_uniform;
			optional_shader_t(shader::shader_t in_shader)
				: shader(std::move(in_shader)),
				transform_uniform(get_transform_uniform()) {}
			optional_shader_t& operator=(shader::shader_t&& new_shader) {
				shader.update_from(std::move(new_shader));
				transform_uniform = get_transform_uniform();
			}
			shader::uniform_t<mat4f> get_transform_uniform() {
				return shader.get_uniform<mat4f>("transform", false);
			}
		};
		using mat4f = Eigen::Matrix4f;
		std::vector<model::model_t> models;
		std::vector<drawer_t*> _drawers;
		std::unique_ptr<optional_shader_t> default_shader = nullptr;
		vec3f position;
		quatf rotation;
		void draw_visable(const mat4f& transformation) override {
			mat4f mat = transformation * object_mat();
			for (drawer_t* drawer : _drawers) {
				if (default_shader) {
					if (auto * shader_drawer = dynamic_cast<shader::shader_t::shader_drawer_t*>(drawer)) {
						drawer->draw(mat);
					}
					else {
						auto use = default_shader->shader.use();
						default_shader->transform_uniform = mat;
						drawer->draw(mat);
					}
				}
				else {
					drawer->draw(mat);
				}
			}
		}
		void set_shader(shader::shader_t&& shader) {
			if (default_shader) {
				*default_shader = std::move(shader);
			}
			else {
				default_shader = std::make_unique<optional_shader_t>(std::move(shader));
			}
		}
		mat4f object_mat() const {
			aff3f out;
			out.translate(position);
			out.rotate(rotation);
			return out.matrix;
		}
	};
}