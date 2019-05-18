#pragma once
#include <string>
#include <stdexcept>

namespace foton {
	namespace exceptions {
		using namespace std::string_literals;
		struct exception_t : std::exception {

		};
		struct out_of_range_t : exception_t, std::out_of_range {
			enum class over_or_under_t {
				neither,
				overflow,
				underflow
			};
			static constexpr const char* over_or_under_name(over_or_under_t ovu) {
				switch (ovu) {
				case over_or_under_t::neither:
					return "neither";
				case over_or_under_t::overflow:
					return "overflow";
				case over_or_under_t::underflow:
					return "underflow";
				}
			}
			const size_t bound;
			const size_t actual;
			const over_or_under_t over_or_under;
			const std::string message;
			out_of_range_t(over_or_under_t over_or_under, size_t bound, size_t actual, std::string message)
				: bound(bound), actual(actual), over_or_under(over_or_under),
				message("out_of_range: "s + over_or_under_name(over_or_under)
					+ ", actual: " + std::to_string(actual) + ", bound: " + std::to_string(bound)),
				std::out_of_range(message), exception_t() {
			}

			struct gl_error_t : std::runtime_error {
				gl_error_t(GLenum err, std::string msg) : std::runtime_error(std::string(
					"glError: (") + std::to_string(err) + ") " + msg
				) {}
			};
		};
	}
}