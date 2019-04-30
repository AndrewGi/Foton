#pragma once
namespace foton {
	struct drawer_t {
		bool is_visable = true;
		virtual void draw() = 0;
	};

}
