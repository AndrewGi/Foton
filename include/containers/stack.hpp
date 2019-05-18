#pragma once
#include "helpers.hpp"
#include "dynamic_vector.hpp"
namespace foton {
	template<class T, bool use_mutex = false, class ContainerT = dynamic_vector_t<T, false>>
	struct stack_t : _maybe_mutex_t<use_mutex> {
		using index_t = ContainerT::index_t;
	private:
		static_assert(!ContainerT::uses_mutex(), "locking is handled by stack, not by ContainerT");
		ContainerT _container;
	public:
		ContainerT swap_container(ContainerT new_container) {
			auto l = write_lock();
			std::swap(new_container, _container);
			return new_container;
		}
		ContainerT& container() {
			return _container;
		}
		const ContainerT& container() const {
			return _container;
		}
		bool empty() const {
			return container().empty();
		}
		void push(T&& object) {
			auto l = write_lock();
			container().push_back(std::move(object));
		}
		T pop() {
			auto l = write_lock();
			return container().pop_back();
		}
		template<class... Args>
		void emplace(Args&& ... args) {
			push(T(std::forward<Args>(args)...));
		}
		index_t size() {
			return container().size();
		}
		template<class F>
		ContainerT consume(F&& for_each_func) {
			ContainerT old_container;
			old_container.resize(container().capacitity());
			swap_container(old_container);
			std::for_each(old_container.begin(), old_container.end(), for_each_func);
			return old_container;
		}
	private:
		
	};
}
