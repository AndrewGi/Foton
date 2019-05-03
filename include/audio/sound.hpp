#pragma once
#include "soundio_cpp.hpp"
#include <vector>
#include <chrono>
namespace foton {
	namespace audio {
		using sample_t = soundio::sample_t;
		using duration_t = std::chrono::duration<double, std::ratio<1, 1>>;
		std::unique_ptr<soundio::soundio_t> sound_io = nullptr;
		void initalize() {
			sound_io = std::make_unique<soundio::soundio_t>();
			sound_io->connect();
		}
		struct stream_t {
			void* user_ptr = nullptr;
		};
		struct out_stream_t : stream_t {
			soundio::out_stream_t stream;
			out_stream_t(soundio::out_stream_t soundio_stream, void* user_ptr = nullptr) : stream(std::move(soundio_stream)), stream_t{ user_ptr } {};
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
				_sample_count = sample_count;
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
		struct sample_generator_t {
			virtual size_t size_hint(size_t max_sample_count) = 0;
			virtual void generate_samples(sample_t* samples, size_t sample_count) = 0;
		};
		struct player_t {
			out_stream_t stream;
			std::unique_ptr<sample_generator_t> generator;
			size_t buffer_size = 0;
			std::unique_ptr<sample_t[]> buffer = nullptr;
			void grow_to(size_t size) {
				if (buffer_size < size) {
					buffer_size = size;
					buffer.reset(new sample_t[size]);
				}
			}
			size_t channel_count() const {
				return 2;
			}
			player_t(out_stream_t stream, std::unique_ptr<sample_generator_t> generator) :
				stream(std::move(stream)), generator(std::move(generator)) {
				stream.stream.user_data = nullptr;
				stream.stream.user_size_hint_callback = _stream_size_hint;
				stream.stream.user_write_callback = _stream_generator;
			}
			static void _stream_generator(soundio::out_stream_t& stream, sample_t* out_samples, size_t sample_count) {
				player_t& player = *static_cast<player_t*>(stream.user_data);
				player.generator->generate_samples(out_samples, sample_count);
				
			}
			static size_t _stream_size_hint(soundio::out_stream_t& stream, size_t max_samples) {
				player_t& player = *static_cast<player_t*>(stream.user_data);
				size_t size = player.generator->size_hint(max_samples);
				return player.generator->size_hint(max_samples);
			}
		};
		template<class GeneratorT, class... Args>
		static player_t make_player(out_stream_t stream, Args&&... args) {
			return { std::move(stream), std::make_unique<GeneratorT>(std::forward<Args>(args)...) };
		}
		std::vector<output_device_t> get_output_devices() {
			std::vector<output_device_t> devices;
			sound_io->force_device_scan();
			const size_t num_of_devices = sound_io->output_count();
			devices.reserve(num_of_devices);
			for (uint32_t i = 0; i < num_of_devices; i++) {
				devices.emplace_back(sound_io->get_output_device(i));
			}
			return devices;
		}
		std::vector<input_device_t> get_input_devices() {
			std::vector<input_device_t> devices;
			sound_io->force_device_scan();
			const size_t num_of_devices = sound_io->input_count();
			devices.reserve(num_of_devices);
			for (uint32_t i = 0; i < num_of_devices; i++) {
				devices.emplace_back(sound_io->get_input_device(i));
			}
			return devices;
		}
	}
}