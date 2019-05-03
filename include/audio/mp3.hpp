#pragma once
#ifdef FOTON_MP3_SUPPORT
#include "sound.hpp"
#include "../libmpg123/mpg123.h"
#pragma comment(lib, "libs/x64/libmpg123.lib")
#include <filesystem>
namespace foton {
	namespace audio {
		namespace mp3 {
			void initalize() {
				mpg123_init();
			}
			struct mpg123_t : sample_generator_t {
				int error;
				mpg123_handle* handle;
				mpg123_t(const char* decoder = nullptr) : handle(mpg123_new(decoder, &error)) {
					mpg123_format_none(handle);
					mpg123_format(handle, static_cast<long>(sample_rate()), static_cast<int>(channel_count()), MPG123_ENC_FLOAT_32);
				}
				void open(const char* path) {
					mpg123_open(handle, path);
				}
				void open_feed() {
					mpg123_open_feed(handle);
				}
				size_t read_bytes(byte_t* bytes, size_t size) {
					size_t actual = 0;
					mpg123_read(handle, bytes, size, &actual);
					return actual;
				}
				size_t read_samples(sample_t* samples, size_t sample_count) {
					return read_bytes(reinterpret_cast<byte_t*>(samples), sample_count * sizeof(sample_t))/sizeof(sample_t);
				}
				void feed_bytes(const byte_t* bytes, size_t size) {
					mpg123_feed(handle, bytes, size);
				}
				size_t channel_count() const {
					return MPG123_STEREO;
				}
				size_t frame_size_in_bytes() const {
					return channel_count() * sizeof(sample_t);
				}
				size_t sample_rate() const {
					return 44100;
				}
				void close() {
					mpg123_close(handle);
				}
				off_t frame_position() {
					return mpg123_framepos(handle);
				}
				void seek(off_t frame_position) {
					mpg123_seek_frame(handle, frame_position, SEEK_SET);
				}
				void generate_samples(sample_t* samples, size_t max_samples) override {
					read_samples(samples, max_samples); //TODO: FIX CHANNEL SEPERATION
				}
				off_t frames_per_second() {
					return static_cast<off_t>(sample_rate() * channel_count());
				}
				off_t length() {
					return mpg123_length(handle);
				}
				size_t size_hint(size_t max_samples) override {
					return std::min(mpg123_outblock(handle)/frame_size_in_bytes(), max_samples);
				}
				~mpg123_t() {
					close();
					mpg123_delete(handle);
					handle = nullptr;
				}
			};
		}
		struct mp3_t : player_t {
			mp3::mpg123_t& mpg123() {
				return *reinterpret_cast<mp3::mpg123_t*>(generator.get());
			}
			mp3_t(out_stream_t out_stream, std::filesystem::path path) : path(std::move(path)), player_t(std::move(out_stream), std::make_unique<mp3::mpg123_t>()) {
				if (!path.empty())
					open(path);
			}
			mp3_t(out_stream_t out_stream) : mp3_t(std::move(out_stream), {}) {}
			void open(const std::filesystem::path& path) {
				this->path = path;
				mpg123().open(path.string().c_str());
			}
			void close() {
				mpg123().close();
			}
			duration_t duration() {
				return frames_to_duration(mpg123().length());
			}
			void seek_to(duration_t position) {
				mpg123().seek(duration_to_frames(position));
			}
			off_t duration_to_frames(duration_t duration) {
				return static_cast<off_t>(duration.count() * mpg123().frames_per_second());
			}
			duration_t frames_to_duration(off_t frames) {
				return duration_t ( double(frames) / mpg123().frames_per_second() );
			}
		private:
			std::filesystem::path path;
		};
	}
}

#endif