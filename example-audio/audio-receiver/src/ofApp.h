/*

	OpenFrameworks ofxNDI audio graphics receiver example

	Using the NDI SDK
	https://ndi.video

	Copyright (C) 2026 Lynn Jarvis.
	http://www.spout.zeal.co

	=========================================================================
	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU Lesser General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
	=========================================================================
*/
#pragma once

#include "ofMain.h"
#include "ofxNDI.h" // NDI classes
#include <cmath> // for std::pow

class ofApp : public ofBaseApp{

	public:

		void setup();
		void update();
		void draw();
		void exit();
		void keyPressed(int key);
		void audioOut(ofSoundBuffer &output);

		void ShowInfo();

		// NDI receiver
		ofxNDIreceiver ndiReceiver;   // NDI receiver
		std::string m_SenderName;
		ofTexture ndiTexture;         // Texture to receive
		unsigned int senderWidth = 0; // sender width and height needed to receive char pixels
		unsigned int senderHeight = 0;

		// Audio
		float* ndiAudioData = nullptr; // NDI audio pointer
		int nChannels = 0;
		int nSamples = 0;
		int nStride = 0;
		int sampleRate = 0;
		size_t writeIndex = 0;
		size_t readIndex = 0;
		size_t bufferCapacity = 0;
		size_t availableSamples = 0;
		float lastAudio[4]{};

		ofSoundStream soundStream;  // Audio stream to send sound to speakers
		bool SetupSoundStream();
		vector<float> audioBuffer;  // Buffer for the audio data
		std::mutex audioMutex;
		int bufferSize = 0;         // Buffer size (change with sample number)
		bool bSoundStream = false;
		bool bSoundStreamPlaying = false;
		bool bAudioReceived = false;

		// For drawing the audio db bar graph and waveform
		std::vector<float> latestAudio;
		vector<float> lAudio;
		vector<float> rAudio;
		void DrawAudio();
		void drawGradientBar(float x, float y, float width, float height);


};
