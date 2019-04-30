#pragma once
#include "../soundio/soundio.h"
#include <utility>
#include <stdexcept>
//TODO: maybe unique_ptr?
namespace foton {
	namespace audio {
		namespace soundio {
			using sample_t = float;
			static const char* str_err(int err) {
				return soundio_strerror(err);
			}
			struct soundio_error_t : std::runtime_error {
				soundio_error_t(int err) : std::runtime_error(str_err(err)) {};
			};
			static bool err_check(int err) {
				if (err != SoundIoErrorNone) {
					throw soundio_error_t(err);
					return true;
				}
				return false;
			}
			struct device_t {
				SoundIoDevice* d_io = nullptr;
				device_t(SoundIoDevice* d_io) : d_io(d_io) {
					soundio_device_ref(d_io);
				}
				device_t(const device_t& other) : device_t(other.d_io) {}
				device_t(device_t&& other) : d_io(other.d_io) {
					other.d_io = nullptr;
				}
				device_t& operator=(device_t&& other) {
					destroy();
					d_io = other.d_io;
					other.d_io = nullptr;
				}
				device_t& operator=(const device_t& other) {
					return *this = std::move(device_t(other));
				}
				~device_t() {
					destroy();
				}
			private:
				void destroy() {
					if (d_io != nullptr) {
						soundio_device_unref(d_io);
						d_io = nullptr;
					}
				}
			};
			struct out_stream_t {
				struct write_area_t {
					const uint32_t channel_index;
					uint32_t samples_left;
					SoundIoChannelArea* area;
					/**
					returns the number of samples not written
					*/
					size_t write(const sample_t* samples, size_t amount) {
						while (amount > 0 && samples_left > 0) {
							*reinterpret_cast<sample_t*>(area->ptr) = *samples;
							*area->ptr += area->step;
							samples_left--;
							amount--;
						}
						return amount;
					}
				};
				SoundIoOutStream* stream_io = nullptr;
				out_stream_t(const device_t& device) : stream_io(soundio_outstream_create(device.d_io)) {
					stream_io->userdata = this;
					stream_io->name = "foton";
				}
				out_stream_t(const out_stream_t&) = delete;
				out_stream_t(out_stream_t&& other) : stream_io(other.stream_io) {
					other.stream_io = nullptr;
				}
				out_stream_t& operator=(out_stream_t&& other) {
					destroy();
					stream_io = other.stream_io;
					other.stream_io = nullptr;
				}
				out_stream_t& operator=(const out_stream_t& other) = delete;
				~out_stream_t() {
					destroy();
				}
				void open() {
					err_check(soundio_outstream_open(stream_io));
				}
				void start() {
					err_check(soundio_outstream_start(stream_io));
				}
				void pause(bool do_pause = true) {
					err_check(soundio_outstream_pause(stream_io, do_pause));
				}
				void resume() {
					pause(false);
				}
				void clear() {
					err_check(soundio_outstream_clear_buffer(stream_io));
				}
				void set_volume(double volume) {
					err_check(soundio_outstream_set_volume(stream_io, volume));
				}
				double get_volume() {
					return static_cast<double>(stream_io->volume);
				}
				size_t sample_rate() const {
					return stream_io->sample_rate;
				}
				uint32_t samples_per_frame() const {
					return stream_io->bytes_per_frame / stream_io->bytes_per_sample;
				}
				uint32_t channels_count() const {
					return stream_io->layout.channel_count;
				}
				uint32_t samples_per_area() const {
					return samples_per_frame() / channels_count();
				}
				void* user_data = nullptr;
				void(*user_write_callback)(out_stream_t& stream, write_area_t& area);
			private:
				uint32_t _frames_per_callback = 1;
				static void _write_callback(SoundIoOutStream* out_stream, int frame_count_min, int frame_count_max) {
					out_stream_t& self = *static_cast<out_stream_t*>(out_stream->userdata);
					if (self.user_write_callback == nullptr)
						return;
					SoundIoChannelArea* areas;
					int frame_count = self._frames_per_callback;
					soundio_outstream_begin_write(out_stream, &areas, &frame_count);
					for (int i = 0; i < frame_count; i++) {
						write_area_t area { static_cast<uint32_t>(i), self.samples_per_frame(), &areas[i] };
						self.user_write_callback(self, area);
					}
					soundio_outstream_end_write(out_stream);
				}
				void destroy() {
					if (stream_io) {
						soundio_outstream_destroy(stream_io);
						stream_io = nullptr;
					}
						
				}
					
			};
			struct soundio_t {
				SoundIo* s_io;
				soundio_t() : s_io(soundio_create()) {
					if (s_io == nullptr) {
						throw soundio_error_t(0); //soundio is null
					}
				}
				void flush() {
					soundio_flush_events(s_io);
				}
				void connect(SoundIoBackend backend = SoundIoBackendNone) {
					if (backend == SoundIoBackendNone) {
						soundio_connect(s_io);
					}
					else {
						soundio_connect_backend(s_io, backend);
					}
					flush();
				}
				void disconnect() {
					soundio_disconnect(s_io);
				}
				size_t output_count() {
					flush();
					int count = soundio_output_device_count(s_io);
					return static_cast<size_t>(count < 0 ? 0 : count);
				}
				size_t input_count() {
					flush();
					int count = soundio_input_device_count(s_io);
					return static_cast<size_t>(count < 0 ? 0 : count);
				}
				device_t get_output_device(uint32_t index) {
					return device_t(soundio_get_output_device(s_io, static_cast<int>(index)));
				}
				device_t get_input_device(uint32_t index) {
					return device_t(soundio_get_input_device(s_io, static_cast<int>(index)));
				}
				void force_device_scan() {
					soundio_force_device_scan(s_io);
				}
				~soundio_t() {
					destory();
				}
			private:
				void destory() {
					if (s_io) {
						flush();
						soundio_destroy(s_io);
						s_io = nullptr;
					}
				}
			};
		}
	}
}