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
	foton::window_t main_window("foton test", 640, 480);
	main_window.set_clear_color(0.5f, 0.25f, 0.1f);
	auto shader = foton::shader::shader_t::load_shader_from_paths({ "resources/shaders/shapes.frag", "resources/shaders/shapes.vert"});
	auto vbo = foton::GL::vbo_t<vec3f>({ {-.5,-.5, 0 }, {0, .5, 0}, {0,0,0} }, GL_TRIANGLES);
	auto shader_bind = shader.use();
	main_window.add_drawer(&vbo);
	auto current_time = std::chrono::high_resolution_clock::now;
	auto fps_display_period = 100ms;
	auto fps_spike_period = 2s;
	auto last_fps_time = current_time();
	double frame_time = 1;
	double last_frame_time_spike = 1;
	auto last_frame_time_spike_time = last_fps_time; //great variable names 
	auto print_fps = [&]() {
		std::cout << "\rframe_time:" << std::right << std::setw(7) << std::setprecision(3) << std::fixed << frame_time << "ms  spike_time:" 
			<< std::right << std::setw(7) << std::setprecision(3) << std::fixed << last_frame_time_spike << "ms";
		std::cout.flush(); //force it so its deterministic (can probably remove this)
	};
	auto get_miliseconds = [](auto duration) -> double {
		return std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(duration).count();
	};
	std::cout << "start!";
	while (!main_window.should_close()) {
		auto start_time = current_time();
		if (last_fps_time + fps_display_period < start_time) {
			print_fps();
			//reset frametime so print_fps() doesn't affect the render fps
			last_fps_time = current_time();
			start_time = last_fps_time;
		}
		main_window.render();
		frame_time = get_miliseconds(current_time() - start_time);
		//check for frame_time spikes 
		//we subtract from last_frame_time_spike inorder to more smoothly show frame spikes
		if (frame_time > last_frame_time_spike - std::max(0.0, get_miliseconds(start_time - fps_spike_period - last_frame_time_spike_time))) {
			//we have a frame_time spike so update the variables
			last_frame_time_spike = frame_time;
			last_frame_time_spike_time = start_time;
		}
	}
}
