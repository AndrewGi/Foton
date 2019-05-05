#pragma once
#include <mutex>
/**

	Foton mutex is current just an alias to std::mutex

	std::mutex is not the faster mutex implementation so down the line,
	a better mutex can be implemented here without having to go rename all the mutexs
	in the rest of the files

	I am not planning on working on a mutex implementation until the mutexs turn into the bottle neck

*/
namespace foton {
	using mutex_t = std::mutex;
	struct thread_mutex_t {
		struct invalid_thread_t : std::runtime_error {
			invalid_thread_t() : std::runtime_error("attempting to lock/unlock mutex in wrong thread") {}
		};
		using thread_id_t = size_t;
		void lock() {
			if (locked_by_this_thread())
				throw invalid_thread_t();
			_mutex.lock();
			_current_locking_thread = get_thread_id();
		}
		void unlock() {
			if (_current_locking_thread != get_thread_id())
				throw invalid_thread_t();
			_current_locking_thread = 0;
			_mutex.unlock();
		}
		bool try_lock() {
			if (locked_by_this_thread()) 
				throw invalid_thread_t();
			if (_mutex.try_lock()) {
				_current_locking_thread = get_thread_id();
				return true;
			}
			return false;
		}
		bool locked() const {
			return _current_locking_thread != 0;
		}
		bool locked_by_this_thread() const {
			return _current_locking_thread == get_thread_id();
		}
		static thread_id_t get_thread_id() {
			return std::hash<std::thread::id>{}(std::this_thread::get_id());
		}
	private:
		mutex_t _mutex;
		volatile thread_id_t _current_locking_thread = 0;
	};
}