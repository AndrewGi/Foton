#pragma once
#include <memory>
#include <stdexcept>
#include <shared_mutex>
namespace foton {
	namespace {
		template<bool>
		class _maybe_mutex {
			bool write_lock() {
				return false;
			}
			bool read_lock() {
				return false;
			}
		};
		template<>
		struct _maybe_mutex<true> {
			std::shared_lock<std::shared_mutex> read_lock() {
				return std::shared_lock<std::shared_mutex>{_mutex};
			}
			std::unique_lock<std::shared_mutex> write_lock() {
				return std::unique_lock<std::shared_mutex>{_mutex};
			}
		private:
			mutable std::shared_mutex _mutex;
		};
	}
	template<class T, uint32_t _capacitity, bool _use_mutex=false, class Allocator = std::allocator<T>>
	struct static_vector_t : _maybe_mutex<_use_mutex> {
		using index_t = uint32_t;
		using alloc_traits = std::allocator_traits<Allocator>;
		using pointer_t = alloc_traits::pointer;
	private:
		Allocator _allocator;
		pointer_t _data;
		index_t _size;
	public:
		static_vector_t(Allocator allocator)
			: _allocator(std::move(allocator)), _data(alloc_traits::allocate(_allocator, capacitity())), _size(0) {
		
		}

		~static_vector_t() {
			alloc_traits::deallocate(_allocator, data(), capacitity());
		}
		T* data() {
			return static_cast<T*>(_data);
		}
		const T* data() const {
			return static_cast<const T*>(_data);
		}
		index_t size() const {
			return _size;
		}
		bool empty() const {
			return size() == 0;
		}
		T& at(index_t index) {

			if (index >= size())
				throw std::out_of_range("static_vector at");
			return data()[index];
		}
		T& operator[](index_t index) {
			return data()[index];
		}
		const T& at(index_t index) const {
			if (index >= size())
				throw std::out_of_range("static_vector const at");
			return data()[index];
		}
		const T& operator[](index_t index) const {
			return data()[index];
		}
		static constexpr index_t capacitity() {
			return _capacitity;
		}
		static constexpr bool use_mutex() const {
			return _use_mutex;
		}
		void check_empty() const {
			if (empty())
				throw std::out_of_range("static_vector empty");
		}
		T& first() {
			check_empty();
			return *data();
		}
		T& last() {
			check_empty();
			return data()[size() - 1];
		}
		template<class... Args>
		T& emplace_back(Args&& ... args) {
			auto l = write_lock();
			if (size() >= capacitity())
				throw std::out_of_range("static_vector emplace_back");
			T* ptr = &data()[size()];
			alloc_traits::construct(_allocator, ptr, std::forward<Args>(args)...);
			_size++;
			return *ptr;
		}
		template<class... Args>
		T& emplace(Args&& ... arg) {
			auto l = write_lock();
			if (size() >= capacitity())
				throw std::out_of_range("static_vector emplace_back");
			alloc_traits::construct(_allocator, &data()[size()], std::forward<Args>(args)...);
			_size++;
			for (index_t i = size() - 1; i > 1; i--) {
				std::swap(data()[i], data()[i - 1]);
			}
			return *data();
		}
		T pop_back() {
			check_empty();
			auto l = write_lock();
			T out = std::move(last());
			_size--;
			return out;
		}
		T pop() {
			check_empty();
			auto l = write_lock();
			for (index_t i = 0; i < size() - 1; i++) {
				std::swap(data()[i], data()[i + 1]);
			}
			return pop_back();
		}
	};
}