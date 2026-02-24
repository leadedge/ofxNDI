#pragma once

#include "ofMain.h"
#include "ofxNDI.h"    // ofxNDI classes
#include "AudioFile.h" // Audio support using miniaudio

class ofApp : public ofBaseApp{

	public:

		void setup();
		void update();
		void draw();
		void exit();

		// NDI
		ofxNDIsender ndiSender;        // NDI sender
		std::string senderName;        // Sender name
		unsigned int senderWidth = 0;  // Width of the sender output
		unsigned int senderHeight = 0; // Height of the sender output
		bool bInitialized = false;
		ofFbo m_fbo;                   // Fbo used for sending
		float videoFps = 30.0f;        // Video frame rate

		// Video
		ofVideoPlayer video;

		// Audio
		std::vector<int> audiosamples;   // Sequence of sample numbers per frame
		int nSamples = 0;                // Current sample number per frame
		int audioindex = 0;              // Audio frame counter

		AudioFile audiofile;             // Miniaudio functions
		int nChannels = 0;               // Decoder channel number
		int sampleRate = 0;              // Decoder sample rate
		std::vector<float> planarBuffer; // Interleaved audio to planar
		void DrawAudio();                // Audio waveform
		vector<float> lAudio;
		vector<float> rAudio;

};
