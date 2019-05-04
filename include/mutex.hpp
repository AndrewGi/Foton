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
		mutex_t mutex;
		void 
	};
}
template<>
struct std::unique_lock<foton::thread_mutex_t> {
	std::unique_lock<std::mutex> _unique_lock;
	foton::thread_mutex_t& _mutex;
	std::unique_lock(foton::thread_mutex_t& mutex) : _mutex(mutex) {}
	bool owns_lock() const {
		_unique_lock.owns_lock();
	}
	bool same_thread() const {

	}
};