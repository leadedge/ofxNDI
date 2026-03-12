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
		float videoFps = 30.0f;        // Video frame rate

		// Graphics
		ofImage textureImage;          // Texture image for the 3D cube
		ofFbo m_fbo;                   // Fbo used for sending
		float rotX = 0.0f;
		float rotY = 0.0f;             // Cube rotation increment
		void DrawGraphics();           // Rotating cube draw

		// Audio
		int nSamples = 0;              // Sample number per frame
		int nChannels = 0;             // Channel number
		int sampleRate = 0;            // Sample rate
		void DrawAudio();
		vector<float> lAudio;
		vector<float> rAudio;

		// soundstream and audioIn
		ofSoundStream soundStream;
		bool bSoundStream = false;
		std::mutex audioMutex;
		vector<float> audioBuffer; // Buffer for the audio data
		size_t writeIndex = 0;
		size_t readIndex = 0;
		size_t bufferCapacity = 0;
		size_t availableSamples = 0;

};
