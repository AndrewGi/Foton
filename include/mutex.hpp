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
	using mutex = std::mutex;
	using lock = std::unique_lock<mutex>;
}