#pragma once 
namespace foton {
	template<bool, class X>
	struct condi {
		struct type {};
	};

	template<class X>
	struct conditional_apply_t<true, X> {
		using type = X;
	};
}