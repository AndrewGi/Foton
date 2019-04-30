#pragma once
#include <chrono>
#include <functional>
namespace foton {
	using namespace std::chrono_literals;
	struct fps_counter_t {
		using callback_t = std::function<void(fps_counter_t&)>;
		using high_res_clock = std::chrono::high_resolution_clock;
		using duration_t = high_res_clock::duration;
		using time_point_t = high_res_clock::time_point;
		using period_t = high_res_clock::period;
		time_point_t start_time = now();
		time_point_t last_frame_time = now();
		duration_t last_frame_duration;

		time_point_t last_fps_callback = now();
		duration_t fps_callback_period;
		callback_t fps_callback;

		template<class T1, class R1>
		fps_counter_t(std::chrono::duration<T1, R1> fps_callback_period, callback_t fps_callback) 
			: fps_callback(std::move(fps_callback)), fps_callback_period(fps_callback_period) {}

		fps_counter_t() {}
		time_point_t now() const {
			return high_res_clock::now();
		}
		duration_t runtime() const {
			return now() - start_time;
		}
		void frame() {
			time_point_t frame_start = now();
			last_frame_duration = frame_start - last_frame_time;
			last_frame_time = frame_start;
			maybe_call_fps_callback();
		}
		duration_t frame_time() const {
			return last_frame_duration;
		}
		double fps() const {
			if (frame_time().count() == 0)
				return 0.0;
		
			return 1/std::chrono::duration<double, std::ratio<1>>(frame_time()).count();
		}

	private:
		void maybe_call_fps_callback() {
			if (fps_callback_period.count() == 0 || !fps_callback)
				return;
			if (last_frame_time - last_fps_callback > fps_callback_period) {
				fps_callback(*this);
				last_fps_callback = last_frame_time;
			}
		}
	};
}