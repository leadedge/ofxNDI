#pragma once

#include "ofMain.h"
#include "ofxNDI.h"    // ofxNDI classes

class ofApp : public ofBaseApp{

	public:

		void setup();
		void update();
		void draw();
		void exit();
		void audioOut(ofSoundBuffer &buffer);

		// NDI
		ofxNDIsender ndiSender;        // NDI sender
		std::string senderName;        // Sender name
		unsigned int senderWidth = 0;  // Width of the sender output
		unsigned int senderHeight = 0; // Height of the sender output

		// Graphics
		ofImage textureImage; // Texture image for the 3D cube graphics
		ofFbo m_fbo;          // Fbo used for sending
		float rotX = 0.0f;
		float rotY = 0.0f;    // Cube rotation increment
		void DrawGraphics();  // Rotating cube draw

		// Audio
		int nSamples = 0;          // Sample number per channel
		int nChannels = 2;         // Stereo
		int sampleRate = 48000;    // 48khz audio
		ofSoundStream soundStream; // To get sound to speakers
		vector<float> audioBuffer; // Buffer for the audio data
		float frequency = 0.0f;    // tone frequency
		float phase = 0.0f;        // for tone generation 

		// Audio waveform dsiplay
		void DrawAudio();
		std::mutex audioMutex;
		vector<float> lAudio;
		vector<float> rAudio;
		vector<float> lCopy;
		vector<float> rCopy;

};
