// Foton.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include "window.hpp"
#include "2D.hpp"
#include "gl/gl_objects.hpp"
#include "gl/shader.hpp"
#include <iostream>
#include <chrono>
using namespace std::chrono_literals;
using vec2f = foton::gfx_2D::vec2f;
using vec3f = Eigen::Vector3f;

static_assert(sizeof(vec2f) == 2 * sizeof(float));


int main()
{
	using namespace foton;
	window_t main_window("foton test", 1920, 1080);
	main_window.set_clear_color(0.5f, 0.25f, 0.1f);
	auto shader_with_paths = shader::shader_with_paths_t::guess_filetypes({ "resources/shaders/test1.frag", "resources/shaders/test1.vert"});
	auto& shader = shader_with_paths.shader();
	shader::uniform_t<float> time_uniform = shader.get_uniform<float>("time");
	foton::GL::vao_t vao;
	vao.bind().emplace_vbo<vec3f>(0, 0, std::initializer_list<vec3f>{ {-.75,-.75, 0 }, {0, .75, 0}, {.75, -.75, .75} }, static_cast<GLenum>(GL_TRIANGLES), true);
	auto wrapper_vao = shader.wrap(vao);
	main_window.add_drawer(&wrapper_vao);
	auto current_time = std::chrono::high_resolution_clock::now;
	auto fps_display_period = 100ms;
	auto fps_spike_period = 2s;
	auto last_fps_time = current_time();
	double frame_time = 1; //not 0 because fps = 1/frame_time and if frame_time == 0.0, thats division by zero
	double last_frame_time_spike = 1;
	auto last_frame_time_spike_time = last_fps_time; //great variable names 
	auto print_fps = [&]() {
		GLenum err = glGetError();
		if (err != GL_NO_ERROR) {
			std::cout << "\rGL ERROR: (" << std::hex << err << ") " << glewGetErrorString(err) << '\n';

		}
		std::cout << "\rframe_time:" << std::right << std::setw(7) << std::setprecision(3) << std::fixed << frame_time << "ms  spike_time:" 
			<< std::right << std::setw(7) << std::setprecision(3) << std::fixed << last_frame_time_spike << "ms       "; //extra spaces to clear and extra characters
		std::cout.flush(); //force it so its deterministic (can probably remove this)
	};
	auto as_miliseconds = [](auto duration) -> double {
		return std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(duration).count();
	};
	auto last_shader_reload_time = current_time();
	main_window.set_key_callback([&] (window_t&, window_t::keyboard_key_t key, window_t::keyboard_action_t action, window_t::keyboard_mods_t mods){
		if (key == GLFW_KEY_R && action == GLFW_PRESS) {
			if (last_shader_reload_time + 500ms > current_time()) //Timeout on shader creation
				return;
			std::cout << "reloading shaders!\n";
			shader_with_paths.reload_shader();
			GL::check_gl_errors("after reloading shaders");
			std::cout << "done!\n";
		}
	});
	auto game_loop_start_time = current_time();
	std::cout << "start!"; //This gets overwritten by the fps_counter
	while (!main_window.should_close()) {
		GL::check_gl_errors("start rendering");
		auto frame_start_time = current_time();
		if (last_fps_time + fps_display_period < frame_start_time) {
			print_fps();
			//reset frametime so print_fps() doesn't affect the render fps
			last_fps_time = current_time();
			frame_start_time = last_fps_time;
		}
		GL::check_gl_errors("before rendering");
		main_window.render();
		GL::check_gl_errors("after rendering");

		frame_time = as_miliseconds(current_time() - frame_start_time);
		//check for frame_time spikes 
		//we subtract from last_frame_time_spike inorder to more smoothly show frame spikes
		if (frame_time > last_frame_time_spike - std::max(0.0, as_miliseconds(frame_start_time - fps_spike_period - last_frame_time_spike_time))) {
			//we have a frame_time spike so update the variables
			last_frame_time_spike = frame_time;
			last_frame_time_spike_time = frame_start_time;
		}
		time_uniform = static_cast<float>(as_miliseconds(frame_start_time - game_loop_start_time)/1000.0); //update uniform

		GL::check_gl_errors("end rendering");
	}
}
