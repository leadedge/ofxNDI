/*

	OpenFrameworks ofxNDI video/audio receiver example

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

	09.02.26 - Start based on video/audio sender example
	23.02.26 - Working version
	

*/
#include "ofApp.h"

//
// This example demonstrates receiving audio along with video.
//
//
//--------------------------------------------------------------
void ofApp::setup(){

	ofBackground(0);
	ofSetColor(255);

	// Set the window title to show that it is a receiver
	ofSetWindowTitle("ofxNDI video/audio receiver");

	// Received sender dimensions
	senderWidth  = (unsigned char)ofGetWidth();
	senderHeight = (unsigned char)ofGetHeight();

	// Pre-allocate texture
	ndiTexture.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);

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
		if (ndiAudioData) {

			// Number of audio channels
			nChannels = ndiReceiver.GetAudioChannels();
			// Number of audio samples per channel
			nSamples = ndiReceiver.GetAudioSamples();
			// Audio sample rate
			sampleRate = ndiReceiver.GetAudioSampleRate();
			// stride in bytes
			nStride = ndiReceiver.GetAudioDataStride();

			// Set up soundstream to maych with the sender audio
			if (!bSoundStream) {

				std::string sname = ndiReceiver.GetSenderName();

				if (sname.find("Test Pattern") != std::string::npos) {
					// TODO - test
					nSamples = 512;
					// Reduce channel number from 4 to 2 for stereo 
					nChannels = 2;
				}

				// Next power of 2 for soundstream
				bufferSize = std::pow(2.0, std::ceil(std::log2(nSamples)));

				ofSoundStreamSettings settings;
				settings.numOutputChannels = nChannels;
				settings.sampleRate = sampleRate;
				settings.bufferSize = bufferSize;
				settings.setOutListener(this);
				soundStream.setup(settings);

				// For receiving audio
				bufferCapacity = sampleRate*nChannels*2; // 2 second stereo
				audioBuffer.resize(bufferCapacity);
				writeIndex = 0;
				readIndex  = 0;

				bSoundStream = true;
			}

			std::unique_lock<std::mutex> lock(audioMutex);

			//
			// Testing indicates that NDI senders
			// always produce planar audio data
			//

			// Get the left and right channels
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
			for (int i = 0; i < nSamples; i++) {
				latestAudio[i * 2]     = left[i];
				latestAudio[i * 2 + 1] = right[i];
			}

		} // endif audio data

	} // endif receiveimage failed

	// Draw the received texture
	ndiTexture.draw(0, 0, ofGetWidth(), ofGetHeight());

	// Draw the audio waveform
	DrawAudio();

	// Show what it's receiving
	ShowInfo();

	return;

}

void ofApp::DrawAudio()
{
	// Copy so that the mutex is locked only briefly
	std::vector<float> copyBuffer;
    {
        std::lock_guard<std::mutex> lock(audioMutex);
		if(latestAudio.empty())
			return;
        copyBuffer = latestAudio; // local copy for draw
    }

	ofSetColor(255);

    int yLeft  = ofGetHeight()/4; // Centre top half
    int yRight = ofGetHeight()*3/4; // Centre bottom half

	// Audio data is -1.0 - +1.0
	// increase to +- quarter height
	float h = (float)(ofGetHeight()/4);

	// Samples spaced over the window width
    float xStep = (float)ofGetWidth()/nSamples*2;

	float sample = 0.0f;
	float x = 0;
    float y = 0;
	float lastx = 0;
	float lasty = 0;
	ofSetLineWidth(2);

    // Left channel
	for (int i = 0; i < nSamples; i+=2)
    {
        sample = copyBuffer[i];
		x = i * xStep;
        y = yLeft + sample * h;
		if (x > lastx) ofDrawLine(lastx, lasty, x, y);
		lastx = x;
		lasty = y;
    }

    // Right channel
	x = y = lastx = lasty = 0;
	for (int i = 0; i < nSamples; i+=2)
	{
        sample = copyBuffer[i];
		x = i*xStep;
        y = yRight + sample * h;
		if (x > lastx) ofDrawLine(lastx, lasty, x, y);
		lastx = x;
		lasty = y;
    }

}

//--------------------------------------------------------------
void ofApp::audioOut(ofSoundBuffer& outBuffer)
{
	// Test for soundstream setup
	if(!bSoundStream)
		return;

	std::lock_guard<std::mutex> lock(audioMutex);
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
			ofDrawBitmapString("1 network source", 20, ofGetHeight() - 20);
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
		}
		else
			std::cout << "No NDI senders found" << std::endl;
	}

}

//--------------------------------------------------------------
void ofApp::exit() {
	ndiReceiver.ReleaseReceiver();
}

