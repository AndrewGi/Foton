#pragma 
#include <mutex>
#include <functional>
#include "GLFW/glfw3.h"
namespace foton {
	class window_t {
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
		void make_context_current() {
			glfwMakeContextCurrent(_glfw_window);
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
		void render() {
			make_context_current(); //only need to be called once
			//glClear(GL_COLOR_BUFFER_BIT);
			glfwSwapBuffers(_glfw_window);
			glfwPollEvents();
		}
	private:
		static void _glfw_on_focus_cb(GLFWwindow* glfw_window, int state) {
			window_t& window = *static_cast<window_t*>(glfwGetWindowUserPointer(glfw_window));
			if (state == GL_TRUE) { //focus gained
				window._on_focus_cb(window);
			}
			else if (state == GL_FALSE) { //focus lossed
				window._on_loss_focus_cb(window);
			}
		}
		//TODO: should_close callback
		GLFWwindow* _glfw_window = nullptr;
		std::function<void(window_t&)> _on_focus_cb;
		std::function<void(window_t&)> _on_loss_focus_cb;
	};
}