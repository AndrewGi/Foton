#pragma once
#include <mutex>
#include <functional>
#include <vector>
#include <iostream>
#include <algorithm>
#include "drawer.hpp"
#include "glew/glew.h"
#include "GLFW/glfw3.h"
namespace foton {
	static void init_glfw() {
		auto err = glfwInit();
		if (err != GL_TRUE) {
			std::cerr << "initalization error: glfw returned " << err << '\n';
			return;
		}
	}
	static void init_glew() {
		glewExperimental = true;
		auto err = glewInit();
		if (err != GLEW_OK) {
			std::cerr << "initalization error: glew returned " << glewGetErrorString(err) << '\n';
			return;
		}
		std::cout << "renderer : " << glGetString(GL_RENDERER) << '\n';
		std::cout << "opengl version : " << glGetString(GL_VERSION) << '\n';
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
	}
	class window_t {
		struct glfw_context_lock_t {
			static std::mutex _gl_lock;
			std::unique_lock<std::mutex> guard;
			glfw_context_lock_t(GLFWwindow* window) : guard(_gl_lock) {
				glfwMakeContextCurrent(window);
			}
		};
		glfw_context_lock_t aquire_glfw_lock() {
			return glfw_context_lock_t(_glfw_window);
		}
	public:
		window_t(const char* title, int width, int height) {
			static std::once_flag glfw_init_flag; //will only call glfwInit once during the duration of the program
			static std::once_flag glew_init_flag; //will only call glfwInit once during the duration of the program
			std::call_once(glfw_init_flag, init_glfw); //this line does nothing if glfw_init_flag was called
			_glfw_window = glfwCreateWindow(width, height, title, nullptr, nullptr);
			glfwSetKeyCallback(_glfw_window, _glfw_on_key_cb);
			glfwSetWindowFocusCallback(_glfw_window, _glfw_on_focus_cb);
			if (!_glfw_window) { //couldn't create a window for some reason
				glfwTerminate();
				throw std::logic_error("couldn't create glfw window");
			}
			glfwSetWindowUserPointer(_glfw_window, this); //set the user pointer to 'this' so we can access 'this' inside callbacks
			auto cl = aquire_glfw_lock();
			std::call_once(glew_init_flag, init_glew);
		}
		window_t(const window_t&) = delete;
		window_t(window_t&& other) {
			*this = std::move(other);
		}
		window_t& operator=(const window_t&) = delete;
		window_t&& operator=(window_t&& other) {
			_glfw_window = other._glfw_window;
			glfwSetWindowUserPointer(_glfw_window, this); //Update the userpoint to the new object
			other._glfw_window = nullptr;
			_on_focus_cb = other._on_focus_cb;
			_on_loss_focus_cb = other._on_loss_focus_cb;
		}
		//returns width, height
		std::pair<int, int> get_dimensions() {
			int width = 0;
			int height = 0;
			glfwGetWindowSize(_glfw_window, &width, &height);
			return { width, height };
		}
		void resize(int width, int height) {
			glfwSetWindowSize(_glfw_window, width, height);
		}
		void hide() {
			glfwHideWindow(_glfw_window);
		}	
		void show() {
			glfwShowWindow(_glfw_window);
		}
		
		void rename(const char* new_name) {
			if (new_name == nullptr)
				throw std::invalid_argument("window rename called with nullptr");
			glfwSetWindowTitle(_glfw_window, new_name);
		}
		void set_on_focus_cb(std::function<void(window_t&)> callback) {
			_on_focus_cb = callback;
		}
		void set_on_loss_focus_cb(std::function<void(window_t&)> callback) {
			_on_loss_focus_cb = callback;
		}
		bool should_close() {
			return glfwWindowShouldClose(_glfw_window);
		}
		void close() {
			glfwSetWindowShouldClose(_glfw_window, true);
		}
		void set_clear_color(float r, float g, float b) {
			auto cl = aquire_glfw_lock();
			glClearColor(r, g, b, 1.0f);
		}
		void add_drawer(drawer_t* drawer_p) {
			auto cl = aquire_glfw_lock(); //Don't want to add anything while we drawing
			_drawers.push_back(drawer_p);
		}
		void render() {
			auto cl = aquire_glfw_lock();
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			/*
				For now, just a for each through all the renderable objects

				Later: hoping to have this multithreaded or more advance in some way
			*/
			std::for_each(_drawers.cbegin(), _drawers.cend(), [](drawer_t* drawer) {
				drawer->draw();
			});

			glfwSwapBuffers(_glfw_window);
			glfwPollEvents();
		}
		using keyboard_key_t = int;
		using keyboard_action_t = int;
		using keyboard_mods_t = int;
		void set_key_callback(std::function<void(window_t&, keyboard_key_t, keyboard_action_t, keyboard_mods_t)> callback) {
			_on_key_cb = callback;
		}
	private:
		static void _glfw_on_focus_cb(GLFWwindow* glfw_window, int state) {
			window_t& window = *static_cast<window_t*>(glfwGetWindowUserPointer(glfw_window));
			if (state == GL_TRUE) { //focus gained
				window._on_focus_cb(window);
			}
			else if (state == GL_FALSE) { //focus lost
				window._on_loss_focus_cb(window);
			}
		}
		static void _glfw_on_key_cb(GLFWwindow* glfw_window, int key, int scancode, int action, int mods) {
			window_t& window = *static_cast<window_t*>(glfwGetWindowUserPointer(glfw_window));
			window._on_key_cb(window, key, action, mods);
		}
		std::vector<drawer_t*> _drawers = {}; //maybe shouldn't use raw pointer?
		//TODO: should_close callback
		GLFWwindow* _glfw_window = nullptr;
		std::function<void(window_t&)> _on_focus_cb;
		std::function<void(window_t&)> _on_loss_focus_cb;
		std::function<void(window_t&, keyboard_key_t, keyboard_action_t, keyboard_mods_t)> _on_key_cb;
	};
}
std::mutex foton::window_t::glfw_context_lock_t::_gl_lock;