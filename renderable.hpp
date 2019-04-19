#pragma once
namespace foton {
	struct renderable_t {
		virtual void draw() const = 0;
	};

}