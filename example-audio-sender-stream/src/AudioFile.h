#pragma once

#include "ofMain.h"  // for Openframeworks functions
#include <string>
#include "miniaudio.h" // Audio file functions

// Suppress warning 4006 : second definition ignored
#pragma warning (disable : 4006)

class AudioFile {

	public:

	AudioFile() { };
	~AudioFile();

	// Miniaudio
	ma_decoder audio_decoder{};
	bool InitializeDecoder(std::string audiofile, int &audioformat, int &nchannels, int &samplerate);
	int64_t ReadAudioFrames(float* frames, int samples_per_frame);
	int GetChannels();

	// FFmpeg
	std::string CreateAudioFile(std::string videofile);
	double GetVideoFps(std::string videofile);

};

