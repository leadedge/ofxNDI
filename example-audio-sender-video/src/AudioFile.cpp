//
// MiniAudio functions for NDI
//
// 24.02.26 - Changes by Daandelange
//			    FFmpeg path for OSX
//			    Shellexecute alternatives for OSX and Linux
//			- Change GetVideoFps to bool with fps argument
//			- Use CreateProcess instead of ShellExecute to
//			  wait for FFmpeg to finish creating the audio file
//

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include "AudioFile.h"

// Suppress warning 4006 : second definition ignored
#pragma warning (disable : 4006)

AudioFile::~AudioFile()
{
	// Free the miniaudio decoder
	ma_decoder_uninit(&audio_decoder);
}

bool AudioFile::InitializeDecoder(std::string audiofile, int &audioformat, int &nchannels, int &samplerate)
{
	ma_decoder_config audio_decoder_config;

	// Setup the miniaudio decoder configuration for 32-bit floating point audio
	// Floating point data is required by NDI
	audio_decoder_config = ma_decoder_config_init(ma_format_f32, 0, 0);

	// Read the audio file and retrieve the sample rate and number of channels
	ma_format audio_fmt = (ma_format)0;
	ma_uint32 num_channels = 0;
	ma_uint32 sample_rate = 0;
	if (ma_decoder_init_file(audiofile.c_str(), &audio_decoder_config, &audio_decoder) == MA_SUCCESS) {
		//   Output data formats
		//     ma_format_unknown = 0, Mainly used for indicating an error, but also the default output format
		//     ma_format_u8      = 1,
		//     ma_format_s16     = 2, Seems to be the most widely supported format.
		//     ma_format_s24     = 3, Tightly packed. 3 bytes per sample.
		//     ma_format_s32     = 4,
		//     ma_format_f32     = 5, (this setup)
		//   Number of channels  (specified by the audio file)
		//   Sample rate         (specified by the audio file)
		ma_decoder_get_data_format(&audio_decoder, &audio_fmt, &num_channels, &sample_rate, nullptr, 0);
		// printf("InitializeDecoder\n");
		// printf("  Output data format = %d\n", (int)audio_fmt);    // 5
		// printf("  Number of channels = %d\n", (int)num_channels); // 2
		// printf("  Sample rate        = %d\n", (int)sample_rate);  // 44100
		audioformat = (int)audio_fmt;
		nchannels = (int)num_channels;
		samplerate = (int)sample_rate;
		return true;
	}
	return false;
}

int64_t AudioFile::ReadAudioFrames(float *frames, int samples_per_frame)
{
	ma_uint64 framesRead;
	ma_result ret = ma_decoder_read_pcm_frames(&audio_decoder, frames, samples_per_frame, &framesRead);
	if (ret != MA_SUCCESS) {
		// For end of file, loop back to the beginning of the audio.
		if (ret == MA_AT_END && ma_decoder_seek_to_pcm_frame(&audio_decoder, 0) != MA_SUCCESS)
			return 0;
	}
	return framesRead;
}

int AudioFile::GetChannels()
{
	return audio_decoder.outputChannels;
}

// Create an audio file from the video file
std::string AudioFile::CreateAudioFile(std::string videofile)
{
	std::string audiofile = videofile.substr(0, videofile.rfind('.')) + ".wav";
	if (!ofFile::doesFileExist(audiofile)) {
		// Create the audio file for miniaudio using FFmpeg
#if defined(TARGET_WIN32)
		std::string ffmpegPath = ofToDataPath("ffmpeg/ffmpeg.exe", true);
#elif defined(TARGET_OSX) || defined(TARGET_LINUX)
		std::string ffmpegPath = ofToDataPath("ffmpeg/ffmpeg", true);
#else
		std::string ffmpegPath = ofToDataPath("", true); // todo !
#endif
		if (!ofFile::doesFileExist(ffmpegPath)) {
			// Warning
			printf("\nffmpeg is required.\n");
			printf("* Go to https://github.com/GyanD/codexffmpeg/releases\n");
			printf("* Click on \"Assets\"\n");
			printf("* Choose the \"Essentials\" build.\n");
			printf("* e.g. \"ffmpeg-2026-02-04-git-627da1111c-essentials_build.zip\"\n");
			printf("* Download the archive, unzip it and and copy \"bin/ffmpeg.exe\"\n");
			printf("* to the folder : \"data/ffmpeg/\".\n\n");
			audiofile.clear();
		}
		else {
			// FFmpeg command arguments
			std::string args = "-y -i ";
			args += "\"" + videofile + "\" ";
			args += "-vn -acodec pcm_f32le -ar 48000 -ac 2 ";
			args += "\"" + audiofile + "\"";
#if defined(TARGET_WIN32)

			std::string cmd = "\"" + ffmpegPath + "\" ";
			cmd += args;

			STARTUPINFOA si{};
			PROCESS_INFORMATION pi{};
			si.cb = sizeof(si);
			si.dwFlags = STARTF_USESHOWWINDOW;
			si.wShowWindow = SW_HIDE;
			BOOL success = CreateProcessA(
				NULL, cmd.data(), NULL, NULL, FALSE,
				CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
			if (!success) {
				printf("AudioFile::CreateAudioFile - CreateProcess failed\n");
				audiofile.clear();
			}
			else {
				// Wait for ffmpeg to finish
				WaitForSingleObject(pi.hProcess, INFINITE);
			    // Check FFmpeg exit code
				DWORD exitCode;
				GetExitCodeProcess(pi.hProcess, &exitCode);
				if (exitCode != 0) {
					printf("AudioFile::CreateAudioFile - FFmpeg failed with code %lu\n", exitCode);
					audiofile.clear();
				}
				CloseHandle(pi.hProcess);
				CloseHandle(pi.hThread);
			}

#elif defined(TARGET_OSX) || defined(TARGET_LINUX)
			std::string result = ofSystem(ffmpegPath+" "+args);
			std::cout << "Result=" << result << std::endl;
			if (!result.length()) {
				audiofile.clear();
				printf("AudioFile::CreateAudioFile - ofSystem error\n");
			}
#else
			ofLogError("AudioFile::CreateAudioFile") << "FFMPEG support is not implemented on your platform !";
			audiofile.clear();
#endif
		}
	}
	return audiofile;
}

bool AudioFile::GetVideoFps(std::string videofile, double &videofps)
{
	if(videofile.empty())
		return false;

	// FFprobe path
	std::string ffprobePath = ofToDataPath("ffmpeg/ffprobe.exe", true);
	if (!ofFile::doesFileExist(ffprobePath)) {
		// Warning
		printf("\nffprobe is required.\n");
		printf("* Go to https://github.com/GyanD/codexffmpeg/releases\n");
		printf("* Click on \"Assets\"\n");
		printf("* Choose the \"Essentials\" build.\n");
		printf("* e.g. \"ffmpeg-2026-02-04-git-627da1111c-essentials_build.zip\"\n");
		printf("* Download the archive, unzip it and and copy \"bin/ffprobe.exe\"\n");
		printf("* to the folder : \"data/ffmpeg/\".\n\n");
		return false;
	}

	std::string cmd = ffprobePath;
	cmd += " -v error -select_streams v:0 -show_entries stream=avg_frame_rate -of default=noprint_wrappers=1:nokey=1";
	cmd += " \""; cmd += videofile; cmd += "\"";

	std::string result = ofSystem(cmd); // returns console output
	double fps = 0.0;
	size_t pos = result.find('/');
	if (pos != std::string::npos) {
		double num = std::stod(result.substr(0, pos));
		double den = std::stod(result.substr(pos+1));
		fps = num/den;
	}

	videofps = fps;
	return true;

}

