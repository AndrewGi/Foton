#pragma once
#include "soundio_cpp.hpp"
#include <vector>
namespace foton {
	namespace audio {
		using sample_t = soundio::sample_t;
		soundio::soundio_t sound_io;
		struct stream_t {
		};
		struct out_stream_t : stream_t {
			soundio::out_stream_t stream;
			out_stream_t(soundio::out_stream_t soundio_stream) : stream(std::move(soundio_stream)) {};
		};
		struct in_stream_t : stream_t {
			in_stream_t() = delete; // not implemented
		};
		struct device_t {
			device_t(soundio::device_t sio) : sio(std::move(sio)) {}
			const char* name() const {
				return sio.d_io->name;
			}
			soundio::device_t sio;
		};
		struct output_device_t : device_t {
			output_device_t(soundio::device_t sio) : device_t(std::move(sio)) {}
			out_stream_t stream() {
				return out_stream_t(soundio::out_stream_t(sio));
			}
		};
		struct input_device_t : device_t {
			input_device_t(soundio::device_t sio) : device_t(std::move(sio)) {}
			
		};
		struct buffered_out_stream_t {
			buffered_out_stream_t(out_stream_t stream, size_t count) : _out(std::move(stream)){
				resize(count);
			}
			sample_t* samples() {
				return _samples.get();
			}
			const sample_t* samples() const {
				return _samples.get();
			}
			void resize(size_t sample_count) {
				sample_count = sample_count;
				_samples = std::unique_ptr<sample_t[]>(new sample_t[sample_count]);
			}
			size_t count() const {
				return _sample_count;
			}
			void fill(sample_t value) {
				for (size_t i = 0; i < count(); i++) {
					samples()[i] = value;
				}
			}
			void clear() {
				fill({});
			}
			sample_t& operator[](size_t index) {
				return _samples[index];
			}
			out_stream_t& stream() {
				return _out;
			}
		private:
			std::unique_ptr<sample_t[]> _samples = nullptr;
			size_t _sample_count = 0;
			out_stream_t _out;
		};
		std::vector<output_device_t> get_output_devices() {
			std::vector<output_device_t> devices;
			sound_io.force_device_scan();
			const size_t num_of_devices = sound_io.output_count();
			devices.reserve(num_of_devices);
			for (uint32_t i = 0; i < num_of_devices; i++) {
				devices.emplace_back(sound_io.get_output_device(i));
			}
			return devices;
		}
		std::vector<input_device_t> get_input_devices() {
			std::vector<input_device_t> devices;
			sound_io.force_device_scan();
			const size_t num_of_devices = sound_io.input_count();
			devices.reserve(num_of_devices);
			for (uint32_t i = 0; i < num_of_devices; i++) {
				devices.emplace_back(sound_io.get_input_device(i));
			}
			return devices;
		}
	}
}