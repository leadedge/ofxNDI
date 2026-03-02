/*

	Openframeworks ofxNDI audio example

	Demonstrates receiving NDI audio and graphics

	Uses Openframeworks ofSoundStream
	https://openframeworks.cc/documentation/sound/ofSoundStream/

	NDI SDK https://ndi.video

	Copyright (C) 2026 Lynn Jarvis.
	https://github.com/leadedge/ofxNDI
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
#include "ofApp.h"

//
//--------------------------------------------------------------
void ofApp::setup(){

	ofBackground(0);
	ofSetColor(255);

	// Set the window title to show that it is a receiver
	ofSetWindowTitle("ofxNDI audio graphics receiver");

	// Received sender dimensions
	senderWidth  = (unsigned char)ofGetWidth();
	senderHeight = (unsigned char)ofGetHeight();

	// Pre-allocate a receiving RGBA texture
	ndiTexture.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);

	// Asynchronous upload of pixels to texture
	ndiReceiver.SetUpload(true);

	//
	// Set to receive audio
	//
	// Follow up with IsAudioFrame() if ReceiveImage fails.
	// Query using GetAudioChannels, GetAudioSamples, GetAudioSampleRate
	// and GetAudioData() to receive the audio data.
	ndiReceiver.SetAudio(true);

	// soundstream is set up to match the NDI sender in audioIn

}


//--------------------------------------------------------------
void ofApp::update() {

}

//--------------------------------------------------------------
void ofApp::draw()
{

	// Receive ofTexture
	if (ndiReceiver.ReceiveImage(ndiTexture)) {
		// Draw ofTexture below
	}
	else {
		//
		// If ReceiveImage fails, query for an audio frame
		//
		ndiAudioData = ndiReceiver.GetAudioData();
		if (ndiAudioData && ndiReceiver.GetFrameType() == NDIlib_frame_type_audio) {

			// Audio sample rate
			sampleRate = ndiReceiver.GetAudioSampleRate();
			// Number of audio channels
			nChannels = ndiReceiver.GetAudioChannels();
			// Number of audio samples per channel
			nSamples = ndiReceiver.GetAudioSamples();
			// Audio data stride in bytes
			nStride = ndiReceiver.GetAudioDataStride();

			// Set up soundstream to match with the sender audio
			if (!bSoundStream) {
				SetupSoundStream();
				// Return for the next frame
				return;
			}


			//
			// Get the left and right channels
			//
			float* left  = ndiAudioData;
			float* right = ndiAudioData + nStride / sizeof(float);
			for (int i = 0; i < nSamples; i++) {
				// left channel
				audioBuffer[writeIndex] = left[i];
				writeIndex = (writeIndex + 1) % bufferCapacity;
				// right channel
				audioBuffer[writeIndex] = right[i];
				writeIndex = (writeIndex + 1) % bufferCapacity;
				availableSamples += 2;
			}

			// For DrawAudio
			latestAudio.resize(nSamples*2);

			// Half width for more than 2 channels
			if (nChannels > 2) {
				// Every second pair of samples
				for (int i = 0; i < nSamples; i++) {
					latestAudio[i*2]     = left[i];
					latestAudio[i*2 + 1] = right[i];
				}
			}
			else {
				// Full width
				for (int i = 0; i < nSamples*2-2; i++) {
					latestAudio[i]     = left[i];
					latestAudio[i + 1] = right[i];
				}
			}

		} // endif received audio data
		else {

			// Mutex lock for shared variable - availableSamples
			std::lock_guard<std::mutex> lock(audioMutex);
			if (bAudioReceived && availableSamples == 0) {

				//
				// No audio data - sender closed
				//

				// Release the current NDI receiver
				// another one is created from the selected index
				ndiReceiver.ReleaseReceiver();

				// Stop and close soundstream
				if (bSoundStream) {
					soundStream.stop();
					soundStream.close();
					// Re-set soundstream
					bSoundStream = false;
					bSoundStreamPlaying = false;
				}

				// Clear audio data
				audioBuffer.clear();
				latestAudio.clear();
				writeIndex = 0;
				readIndex = 0;
				availableSamples = 0;
				bAudioReceived = false;

			} // endif NDI sender closed

		} // endif no audio data received

	} // endif receiveimage failed

	// Draw the received texture whether received or not
	ndiTexture.draw(0, 0, ofGetWidth(), ofGetHeight());

	// Draw the audio waveform
	DrawAudio();

	// Show what it's receiving
	ShowInfo();

	return;

}

//--------------------------------------------------------------
bool ofApp::SetupSoundStream()
{
	// Stop and close if already playing
	if (bSoundStream) {
		soundStream.stop();
		soundStream.close();
		bSoundStream = false;
		bSoundStreamPlaying = false;
	}

	// Clear audio data
	audioBuffer.clear();
	latestAudio.clear();
	writeIndex = 0;
	readIndex  = 0;
	bAudioReceived = false;

	// Reduce the sample number for stereo speakers
	// if more than 2 channels
	if (nChannels > 2) {
		// Channel number could be 4, 5, 6, 7 etc
		float reduce = ceil((float)nChannels/2.0f);
		nSamples /= (int)reduce;
	}

	// Buffer size is the next power of 2 for soundstream
	bufferSize = std::pow(2.0, std::ceil(std::log2(nSamples)));

	// m_Channels etc is established from the connected sender
	ofSoundStreamSettings settings;
	settings.numOutputChannels = 2; // Stereo
	settings.sampleRate = sampleRate;
	settings.bufferSize = bufferSize;
	settings.setOutListener(this);
	if (soundStream.setup(settings)) {
		// For receiving audio
		bufferCapacity =sampleRate*4; // 2 second stereo for 2 channels
		audioBuffer.resize(bufferCapacity);
		latestAudio.clear();
		writeIndex = 0;
		readIndex  = 0;
		bSoundStream = true;
		bSoundStreamPlaying = true;
		return true;
	}

	printf("Soundstream setup failed\n");

	return false;

}

//--------------------------------------------------------------
void ofApp::DrawAudio() {

	// Copy so that the mutex is locked only briefly
	std::vector<float> copyBuffer;
    {
        std::lock_guard<std::mutex> lock(audioMutex);
		if(latestAudio.empty())
			return;
        copyBuffer = latestAudio; // local copy for draw
    }

	ofSetColor(255);
	ofSetLineWidth(2);

	//
	// Draw the waveform
	//

	int yPos = ofGetHeight()/2; // Centre of the window

	// Audio data is -1.0 - +1.0
	// increase to +- third window height
	float h = (float)(ofGetHeight()/3);

	// Copybuffer has left and right channel data interleaved
	// Samples spaced over the window width
	float xStep = (float)ofGetWidth()/(copyBuffer.size()/2);

	float sample = 0.0f;
	float x = 0.0f;
	float y = 0.0f;
	float lastx = 0.0f;
	float lasty = 0.0f;

	// Interleaved audio
	// L R L R L R L R L R ----
	// L   L   L   L   L
	//   R   R   R   R   R
	for (int i = 0; i < copyBuffer.size()/2-2; i+=2)
	{
		// Average of left and right channels
		sample = (copyBuffer[i]+copyBuffer[i+1])/2.0f;
		x = i*xStep;
		y = yPos + sample*h;
		if (x > lastx) ofDrawLine(lastx, lasty, x, y);
		lastx = x;
		lasty = y;
	}

}

//--------------------------------------------------------------
void ofApp::audioOut(ofSoundBuffer& outBuffer)
{
	std::lock_guard<std::mutex> lock(audioMutex);

	// Test for soundstream setup and playing
	if(!bSoundStream || !bSoundStreamPlaying)
		return;

	for (size_t i = 0; i < outBuffer.size(); i++) {
		if (availableSamples > 0) {
			outBuffer[i] = audioBuffer[readIndex];
			readIndex = (readIndex + 1) % bufferCapacity;
			availableSamples--;
		}
		else {
			outBuffer[i] = 0.0f;
		}
	}

}


void ofApp::ShowInfo() {

	std::string str;

	int nsenders = ndiReceiver.GetSenderCount();

	if (nsenders > 0) {
		if (ndiReceiver.ReceiverCreated()) {
			if (ndiReceiver.ReceiverConnected()) {
				// Show received sender information and received fps
				str = "Receiving [";
				str += ndiReceiver.GetSenderName();
				str += "]";
				ofDrawBitmapString(str, 20, 20);

				// Sender video type UYVY or RGBA
				NDIlib_FourCC_video_type_e type = ndiReceiver.GetVideoType();
				if(type == NDIlib_FourCC_video_type_UYVY)
					str = "YUV ";
				else if(type == NDIlib_FourCC_video_type_BGRA)
					str = "BGRA ";
				else if(type == NDIlib_FourCC_video_type_RGBA)
					str = "RGBA ";

				str += std::to_string(ndiReceiver.GetSenderWidth()); str += "x";
				str += std::to_string(ndiReceiver.GetSenderHeight()); str += " ";

				// Sender frame rate
				float fps = ndiReceiver.GetSenderFps();
				str += std::to_string((int)fps); str += ".";
				str += std::to_string((int)(fps * 100) - (int)fps * 100); str += " fps - receiving at ";

				// Actual received frame rate
				fps = ndiReceiver.GetFps();
				str += std::to_string((int)fps); str += ".";
				str += std::to_string((int)(fps * 100) - (int)fps * 100);
				str += " fps";

				ofDrawBitmapString(str, 20, 40);
			}
		}

		if (nsenders == 1) {
			ofDrawBitmapString("1 network source - 'SPACE' to list senders", 20, ofGetHeight() - 20);
		}
		else {
			str = std::to_string(nsenders);
			str += " network sources";
			ofDrawBitmapString(str, 20, ofGetHeight() - 40);
			ofDrawBitmapString("'SPACE' to list senders", 20, ofGetHeight() - 20);
		}
	}
	else {
		ofDrawBitmapString("Connecting . . .", 20, 30);
	}
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {

	char name[256];
	int index = key - 48;

	int nsenders = ndiReceiver.GetSenderCount();

	if (key == ' ') {
		// List all the senders
		if (nsenders > 0) {
			std::cout << "Number of NDI senders found: " << nsenders << std::endl;
			for (int i = 0; i < nsenders; i++) {
				ndiReceiver.GetSenderName(name, 256, i);
				std::cout << "    Sender " << i << " [" << name << "]" << std::endl;
			}
			if (nsenders > 1)
				std::cout << "Press key [0] to [" << nsenders - 1 << "] to select a sender" << std::endl;
		}
		else
			std::cout << "No NDI senders found" << std::endl;
	}
	else if (nsenders > 0 && index >= 0 && index < nsenders) {
		// Update the receiver with the returned index
		// Returns false if the current sender is selected
		if (ndiReceiver.SetSenderIndex(index)) {
			std::cout << "Selected [" << ndiReceiver.GetSenderName(index) << "]" << std::endl;
			// Release the current NDI receiver
			// another one is created from the selected index by ReceiveImage
			ndiReceiver.ReleaseReceiver();
			// Stop and close soundstream
			if (bSoundStream) {
				soundStream.stop();
				soundStream.close();
				// Re-set soundstream to match the sender audio data
				bSoundStream = false;
				bSoundStreamPlaying = false;
			}
			// Clear audio data
			audioBuffer.clear();
			latestAudio.clear();
			writeIndex = 0;
			readIndex = 0;
			availableSamples = 0;
			bAudioReceived = false;

		}
		else {
			std::cout << "Same sender" << std::endl;
		}
	}

}


//--------------------------------------------------------------
void ofApp::exit() {
	// Stop and close soundstream
	soundStream.stop();
	soundStream.close();
	bSoundStream = false;
	bSoundStreamPlaying = false;
	// Release the receiver
	ndiReceiver.ReleaseReceiver();
}

