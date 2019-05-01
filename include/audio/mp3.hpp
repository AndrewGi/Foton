#pragma once
#ifdef FOTON_MP3_SUPPORT
#include "sound.hpp"
#include "../libmpg123/mpg123.h"
#pragma comment(lib, "resources/libmpg123.lib")
#include <filesystem>
namespace foton {
	namespace audio {
		struct mp3_sample_generator_t {
			
		};
		struct mp3_t {
			std::filesystem::path path;
			mp3_t(std::filesystem::path path) : path(std::move(path)) {

			}
		};
	}
}

#endif