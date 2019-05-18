#pragma once
#include "../types.hpp"
#include "../mutex.hpp"
#include "../containers/job_queue.hpp"
namespace foton {
	struct draw_call_t {
		virtual void draw() = 0;
	};
	class draw_queue_t {		
		job_queue_t<draw_call_t, true> _queue;
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
