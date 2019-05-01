// Foton.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include "windows/window.hpp"
#include "2D.hpp"
#include "graphics/gl/gl_objects.hpp"
#include "graphics/gl/shader.hpp"
#include "model.hpp"
#include "audio/sound.hpp"
#define FOTON_MP3_SUPPORT
#include "audio/mp3.hpp"
#include "utility/fps_counter.hpp"
#include <iostream>
#include <chrono>
using namespace std::chrono_literals;
using vec2f = foton::gfx_2D::vec2f;
using vec3f = Eigen::Vector3f;

static_assert(sizeof(vec2f) == 2 * sizeof(float));
static_assert(sizeof(vec3f) == 3 * sizeof(float));


auto print_fps = [](foton::fps_counter_t& counter) {
	std::cout << "\rframe_time:" << std::right << std::setw(7) << std::setprecision(3) << std::fixed << std::chrono::duration<double, std::milli>(counter.frame_time()).count() << "ms";
		//<< std::right << std::setw(7) << std::setprecision(3) << std::fixed << last_frame_time_spike << "ms       "; //extra spaces to clear and extra characters
	//std::cout.flush(); //force it so its deterministic (can probably remove this)
};

int main()
{
	using namespace foton;
	window_t main_window("foton test", 1920, 1080);
	main_window.set_clear_color(0.1f, 0.1f, 0.1f);
	main_window.fps_counter = fps_counter_t(250ms, print_fps);
	auto shader_with_paths = shader::shader_with_paths_t::guess_filetypes({ "resources/shaders/test1.frag", "resources/shaders/test1.vert"});
	auto& shader = shader_with_paths.shader();
	shader::uniform_t<float> time_uniform = shader.get_uniform<float>("time");
	shader::shader_transform_uniforms_t stu(shader);
	stu.model_mat = mat4f();

	foton::GL::vao_t vao;
	vao.bind().emplace_vertex_attribute<vec3f>(0, 0, 0, std::initializer_list<vec3f>{ {-.75,-.75, 0 }, {0, .75, 0}, {.75, -.75, .75} });

	auto wrapper_vao = shader.wrap(vao);
	main_window.add_drawer(&wrapper_vao);

	audio::initalize();
	std::cout << "Output devices:\n";
	for (const auto& device : audio::get_output_devices()) {
		std::cout << device.name() << '\n';
	}

	auto current_time = std::chrono::high_resolution_clock::now;
	auto last_shader_reload_time = current_time();

	main_window.set_key_callback([&] (window_t& window, window_t::keyboard_key_t key, window_t::keyboard_action_t action, window_t::keyboard_mods_t mods){
		if (key == GLFW_KEY_R && action == GLFW_PRESS) {
			if (last_shader_reload_time + 500ms > current_time()) //Timeout on shader creation
				return;
			std::cout << "reloading shaders!\n";
			shader_with_paths.reload_shader();
			GL::check_gl_errors("after reloading shaders");
			std::cout << "done!\n";
		}
		if (key == GLFW_KEY_S && action == GLFW_PRESS) {
			window.camera().view.position + vec3f(0, 0, -.1f);
		}
	});
	auto game_loop_start_time = current_time();
	std::cout << "start!"; //This gets overwritten by the fps_counter
	while (!main_window.should_close()) {
		main_window.render();
		time_uniform = std::chrono::duration<float, std::ratio<1>>(main_window.fps_counter.runtime()).count(); //update uniform
		glfwPollEvents();
	}
}
