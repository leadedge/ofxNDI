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
		float videoFps = 30.0f;        // NDI video frame rate
		bool bInitialized = false;

		// Graphics
		ofImage textureImage;           // Texture image for the 3D cube graphics
		ofFbo m_fbo;                    // Fbo used for sending
		float rotX = 0.0f;
		float rotY = 0.0f;              // Cube rotation increment
		void DrawGraphics();            // Rotating cube draw

		// Audio
		std::vector<int> audiosamples;  // Sequence of sample numbers per frame
		int nSamples = 0;               // Current sample number per frame
		int audioindex = 0;             // Audio frame counter

		AudioFile audiofile;             // Miniaudio functions
		int nChannels = 0;               // Decoder channel number
		int sampleRate = 0;              // Decoder audio sample rate
		void DrawAudio();                // Audio waveform
		vector<float> lAudio;            // Audio to draw
		vector<float> rAudio;

};
