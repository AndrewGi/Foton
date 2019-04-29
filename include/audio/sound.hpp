#pragma once
#include "soundio_cpp.hpp"
#include <vector>
namespace foton {
	namespace audio {
		soundio::soundio_t sound_io;
		struct output_device_t : device_t {

		};
		struct input_device_t : device_t {

		};
		struct device_t {
			const char* name() const {
				return sio.d_io->name;
			}
			
		private:
			soundio::device_t sio;
		};
		std::vector<output_device_t> get_output_devices() {
			std::vector<output_device_t> devices;
			sound_io.force_device_scan();
			const size_t num_of_devices = sound_io.output_count();
			devices.reserve(num_of_devices);
			for (size_t i = 0; i < num_of_devices; i++) {
				devices.emplace_back(sound_io.get_output_device(i));
			}
			return devices;
		}
		std::vector<output_device_t> get_input_devices() {
			std::vector<output_device_t> devices;
			sound_io.force_device_scan();
			const size_t num_of_devices = sound_io.input_count();
			devices.reserve(num_of_devices);
			for (size_t i = 0; i < num_of_devices; i++) {
				devices.emplace_back(sound_io.get_input_device(i));
			}
			return devices;
		}
	}
}