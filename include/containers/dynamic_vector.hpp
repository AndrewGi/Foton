#pragma once
#include "helpers.hpp"

namespace foton {
	template<class T, bool _use_mutex = false, class Allocator = std::allocator<T>>
	struct dynamic_vector_t : _maybe_mutex_t<_use_mutex> {
		using index_t = uint32_t;
		using alloc_traits = std::allocator_traits<Allocator>;
		using pointer_t = alloc_traits::pointer;
		static constexpr index_t DEFAULT_CAPACITY = 8;
	private:
		Allocator _allocator = {};
		pointer_t _data = nullptr;
		index_t _capacity = 0;
		index_t _size = 0;
	public:
		dynamic_vector_t(index_t capacity, Allocator allocator)
			: _allocator(std::move(allocator)) {
			expand_to(capacity);
		}
		dynamic_vector_t(Allocator allocator) : dynamic_vector_t(DEFAULT_CAPACITY, std::move(allocator)) {
		}
		dynamic_vector_t() {}
		
		~dynamic_vector_t() {
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
		static constexpr bool uses_mutex() const {
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
		T* end() {
			return &data[size()];
		}
		T* start() {
			return &data[0];
		}
		template<class... Args>
		T& emplace_back(Args&& ... args) {
			auto l = write_lock();
			if (size() >= capacitity())
				grow();
			T* ptr = &data()[size()];
			alloc_traits::construct(_allocator, ptr, std::forward<Args>(args)...);
			_size++;
			return *ptr;
		}
		template<class... Args>
		T& emplace(Args&& ... arg) {
			auto l = write_lock();
			if (size() >= capacitity())
				grow();
			auto l = write_lock();
			alloc_traits::construct(_allocator, &data()[size()], std::forward<Args>(args)...);
			_size++;
			for (index_t i = size() - 1; i > 1; i--) {
				std::swap(data()[i], data()[i - 1]);
			}
			return *data();
		}
		T pop_back() {
			auto l = write_lock();
			check_empty();
			T out = std::move(last());
			_size--;
			return out;
		}
		T pop() {
			auto l = write_lock();
			check_empty();
			for (index_t i = 0; i < size() - 1; i++) {
				std::swap(data()[i], data()[i + 1]);
			}
			return pop_back();
		}
		void resize(index_t new_capacity) {
			auto l = write_lock();
			expand_to(new_capacity);
		}
		private:
			void expand_to(index_t new_capacity) {
				//WARNING: THESE DON'T LOCK THEMSELVES
				if (new_capacity < capacitity())
					throw std::underflow_error("new_capacity smaller than current");
				pointer_t new_mem = alloc_traits::allocate(_allocator, new_capacity);
				if (data()) {
					if (size())
						std::move(start(), end(), new_mem);
					if constexpr (!std::is_trivially_destructible_v<T>()) {
						//Not trivally destructible so we gotta destroy all of them our selfs
						std::for_each(start(), end(), [](T& t) {t.~T(); });
					}
					alloc_traits::deallocate(_allocator, data(), capacitity());
				}
				_data = new_mem;
				_capacity = new_capacity;
			}
			void grow() {
				//WARNING: THESE DON'T LOCK THEMSELVES
				expand_to(capacitity() * 2); //double in size
			}
	};
}