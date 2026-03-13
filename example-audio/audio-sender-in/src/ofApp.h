#pragma once

#include "ofMain.h"
#include "ofxNDI.h"    // ofxNDI classes

class ofApp : public ofBaseApp{

	public:

		void setup();
		void update();
		void draw();
		void exit();
		void audioIn(ofSoundBuffer & input);

		// NDI
		ofxNDIsender ndiSender;        // NDI sender
		std::string senderName;        // Sender name
		unsigned int senderWidth = 0;  // Width of the sender output
		unsigned int senderHeight = 0; // Height of the sender output
		ofFbo m_fbo;                   // Fbo used for sending
		float videoFps = 30.0f;        // Video frame rate

		// Graphics
		ofImage textureImage;          // Texture image for the 3D cube graphics
		float rotX = 0.0f;
		float rotY = 0.0f;             // Cube rotation increment
		void DrawGraphics();           // Rotating cube draw

		// Audio
		std::vector<int> audiosamples; // Sequence of sample numbers per frame
		int nSamples = 0;              // Current sample number per frame
		int audioindex = 0;            // Audio frame counter
		int nChannels = 0;             // Decoder channel number
		int sampleRate = 0;            // Decoder sample rate

		ofSoundStream soundStream;     // To get sound to speakers
		std::mutex audioMutex;
		int bufferSize = 0;            // Buffer size

		// Draw audio waveform
		void DrawAudio();
		bool bAudioReceived = false;
		vector<float> lAudio;
		vector<float> rAudio;
		// Copies for mutex use
		vector<float> lCopy;
		vector<float> rCopy;


};
