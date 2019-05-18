#pragma once
#include "helpers.hpp"
#include "dynamic_vector.hpp"
namespace foton {
	template<class T, bool use_mutex = false, class ContainerT = dynamic_vector_t<T, false>>
	struct unordered_queue_t {
		using index_t = ContainerT::index_t;
	private:
		ContainerT _container;
	public:
		ContainerT swap_container(ContainerT new_container) {
			auto l = container().write_lock();
			std::swap(new_container, _container);
			return new_container;
		}
		ContainerT& container() {
			return _container;
		}
		void push(T&& object) {
			container().push_back(std::move(object));
		}
		template<class... Args>
		void emplace(Args&& ... args) {
			push(T(std::forward<Args>(args)...));
		}
		template<class F>
		void consume(F&& for_each_function) {
			auto l = container().read_lock();

		}
	private:
		
	};
}
