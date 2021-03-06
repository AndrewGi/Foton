#pragma once
#include <memory>
#include <algorithm>
#include <shared_mutex>
#include "../exceptions.hpp"

namespace foton {
	template<bool>
	struct _maybe_mutex_t {
		bool write_lock() const {
			return false;
		}
		bool read_lock() const {
			return false;
		}
	};
	template<>
	struct _maybe_mutex_t<true> {
		std::shared_lock<std::shared_mutex> read_lock() const {
			return std::shared_lock<std::shared_mutex>{_mutex};
		}
		std::unique_lock<std::shared_mutex> write_lock() const {
			return std::unique_lock<std::shared_mutex>{_mutex};
		}
	private:
		mutable std::shared_mutex _mutex;
	};
}