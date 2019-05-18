#pragma once
#include "../types.hpp"
#include "../mutex.hpp"
#include "../graphics/gl/shader.hpp"
#include "../containers/stack.hpp"
namespace foton {
	struct draw_call_t {

	};
	class draw_call_stack_t {
		struct stack_call_t {

		};
		stack_t<draw_call_t, true> _queue;
		static thread_mutex_t _mutex;
	public:
		std::unique_lock<thread_mutex_t> lock_draw_context() {
			return std::unique_lock<thread_mutex_t>(_mutex);
		}

		void draw() {
			auto l = lock_draw_context();
			_queue.consume(&draw_call_t::draw);
		}
	};
}
