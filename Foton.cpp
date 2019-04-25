// Foton.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include "window.hpp"
#include "shader.hpp"
#include "2D.hpp"
#include "gl_buffer.hpp"
#include <iostream>

int main()
{
	foton::window_t main_window("foton test", 640, 480);
	main_window.set_clear_color(0.5f, 0.25f, 0.1f);
	auto shader = foton::shader::shader_t::load_shader_from_paths({ "shapes.frag", "shapes.vert", "shapes.geom" });
	while (!main_window.should_close()) {
		main_window.render();
	}
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
