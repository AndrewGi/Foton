#pragma 
#include <mutex>
#include <functional>
#include <vector>
#include <algorithm>
#include "drawer.hpp"
#include "GL/glew.h"
#include "GLFW/glfw3.h"
namespace foton {
	class window_t {
		struct global_draw_lock_t {
			static std::mutex _gl_lock;
			std::unique_lock<std::mutex> guard;
			global_draw_lock_t(GLFWwindow* window) : guard(_gl_lock) {
				glfwMakeContextCurrent(window);
			}
		};
		global_draw_lock_t aquire_draw_lock() {
			return global_draw_lock_t(_glfw_window);
		}
	public:
		
		window_t(const char* title, int width, int height) {
			static std::once_flag glfw_init_flag; //will only call glfwInit once during the duration of the program
			std::call_once(glfw_init_flag, glfwInit); //this line does nothing if glfw_init_flag was called
			_glfw_window = glfwCreateWindow(width, height, title, nullptr, nullptr);
			if (!_glfw_window) { //couldn't create a window for some reason
				glfwTerminate();
				throw std::logic_error("couldn't create glfw window");
			}
			glfwSetWindowUserPointer(_glfw_window, this); //set the user pointer to 'this' so we can access 'this' inside callbacks
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
		//Called before making and gl* calls
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
			auto cl = aquire_draw_lock();
			glClearColor(r, g, b, 1.0f);
		}
		void add_drawer(const drawer_t* drawer_p) {
			auto cl = aquire_draw_lock(); //Don't want to add anything while we drawing
			_drawers.push_back(drawer_p);
		}
		void render() {
			auto cl = aquire_draw_lock();
			glClear(GL_COLOR_BUFFER_BIT);

			/*
				For now, just a for each through all the renderable objects

				Later: hoping to have this multithreaded or more advance in some way
			*/
			std::for_each(_drawers.cbegin(), _drawers.cend(), [](const drawer_t* drawer) {
				drawer->draw();
			});

			glfwSwapBuffers(_glfw_window);
			glfwPollEvents();
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

		std::vector<const drawer_t*> _drawers; //maybe shouldn't use raw pointer?
		//TODO: should_close callback
		GLFWwindow* _glfw_window = nullptr;
		std::function<void(window_t&)> _on_focus_cb;
		std::function<void(window_t&)> _on_loss_focus_cb;
	};
}
std::mutex foton::window_t::global_draw_lock_t::_gl_lock;