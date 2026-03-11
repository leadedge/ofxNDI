#include "ofApp.h"

/*

	Openframeworks ofxNDI audio input example

	Demonstrates reading system audio in audioIn
	and sending together with graphics

	Play an audio file with a media player
	so that audio data is available in audioIn

	Uses Openframeworks ofSoundStream
	https://openframeworks.cc/documentation/sound/ofSoundStream/

	NDI SDK https://ndi.video

	Copyright (C) 2026 Lynn Jarvis.
	https://github.com/leadedge/ofxNDI
	http://www.spout.zeal.co

	- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
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
	- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
*/

//--------------------------------------------------------------
void ofApp::setup()
{

	ofBackground(0);

	// NDI sender name
	senderName = "ofxNDI audio input sender";
	ofSetWindowTitle(senderName); // show it on the title bar

	// NDI sender dimensions
	senderWidth  = 1280;
	senderHeight =  720;

	// Video frame rate
	// 60, 30, 29.97 etc
	// 30 fps NDI video frame rate
	// For soundstream audio rate of 48000 hz
	float videofps = 30;
	ndiSender.SetFrameRate(videofps);

	//
	// Set up soundstream for audioIn
	//
	nChannels = 2; // Stereo

	// Audio rate for NDI and Soundstream
	// 48000 or 44100
	sampleRate = 48000;

	// Stereo at 48000/30fps requires 1600 samples per channel per frame.
	// Soundstream rounds up to the next power of 2 (2048)
	//   48000/60fps -  800 (1024)
	//   44100/30fps - 1470 (2048)
	//   44100/60fps -  735 (1024)
	nSamples = sampleRate/videofps;
	
	ofSoundStreamSettings settings;
	auto alldevices = soundStream.getDeviceList();
	// Select the device number as required by your system
	settings.setInDevice(alldevices[0]); // Speakers
	settings.setInListener(this);
	settings.sampleRate = sampleRate;
	settings.numInputChannels = nChannels;
	settings.bufferSize = nSamples;
	if (soundStream.setup(settings)) {
		// Make sure the NDI sender and soundstream
		// use the same sample number
		nSamples = soundStream.getBufferSize();

		printf("\nSoundstream setup\n");
		printf("  nSamples     = %d\n", soundStream.getBufferSize());
		printf("  Sample rate  = %d\n", soundStream.getSampleRate());
		printf("  N channels   = %d\n", soundStream.getNumInputChannels());

		bufferCapacity = nSamples*sampleRate*2; // 2 seconds audio buffer
		audioBuffer.resize(bufferCapacity); // Buffer for audio data
		lAudio.assign(nSamples, 0.0); // Number of audio samples per channel
		rAudio.assign(nSamples, 0.0);
		writeIndex = 0; // index for writing in audioIn
		readIndex  = 0; // index for reading in draw
		bSoundStream = true; // Soundstream is initialized
	}
	else {
		printf("Soundstream setup failed\n");
	}

	// NDI sender
	ndiSender.SetAudio(true); // Allow audio
	ndiSender.SetAudioSampleRate(sampleRate);
	ndiSender.SetAudioChannels(nChannels);
	// Audio samples per channel matching soundstream
	ndiSender.SetAudioSamples(nSamples);
	// Soundstream audio data is float interleaved
	ndiSender.SetAudioType(audio_frame_interleaved_32f_t);

	// Create an NDI sender with YUV output format for best speed
	ndiSender.SetFormat(NDIlib_FourCC_video_type_UYVY);
	if(ndiSender.CreateSender(senderName.c_str(), senderWidth, senderHeight))
		printf("Created sender [%s]\n", senderName.c_str());
	else
		printf("CreateSender failed\n");

	// Allocate an RGBA fbo for texture draw
	m_fbo.allocate(senderWidth, senderHeight, GL_RGBA);

	// 3D drawing setup for the demo graphics
	glEnable(GL_DEPTH_TEST); // Depth comparisons are needed
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	ofDisableAlphaBlending(); // To prevent trails with the rotating cube
	ofDisableArbTex(); // To create a texture with normalized coordinates
	textureImage.load("SpoutBox.jpg");
	ofEnableArbTex();

	// Important - disable vertical sync lock
	// to use draw together with audioIn
	ofSetVerticalSync(false);

}

//--------------------------------------------------------------
void ofApp::update()
{

}

//--------------------------------------------------------------
void ofApp::draw()
{
	ofBackground(0);
	ofSetColor(255);

	// Check success of CreateSender
	if (!ndiSender.SenderCreated())
		return;

	// Mutex lock for variables shared with audioIn
	// (availableSamples and audioBuffer)
	{
		std::unique_lock<std::mutex> lock(audioMutex);
		// Soundstream audio is interleaved, nSamples*2 per frame
		// L R L R L R L R L R ...
		if (availableSamples >= nSamples*2) {

			// Send audio frames to NDI
			if (ndiSender.SenderCreated()) {
				ndiSender.SetAudioData(audioBuffer.data()+readIndex);
				ndiSender.SendAudio();
			}

			// Get left and right channel audio data for DrawAudio
			for (int i = 0; i <nSamples; i++) {
				// left channel
				lAudio[i] = audioBuffer[readIndex];
				// right channel
				rAudio[i] = audioBuffer[readIndex + 1];
				readIndex = (readIndex + 2) % bufferCapacity;
				availableSamples -= 2;
			}
		}
	}

	// Draw graphics into an fbo
	DrawGraphics();

	// Send video
	ndiSender.SendImage(m_fbo);

	// Draw the fbo result fitted to the display window
	m_fbo.draw(0, 0, ofGetWidth(), ofGetHeight());

	// Draw the audio waveform
	DrawAudio();

}

//--------------------------------------------------------------
void ofApp::DrawGraphics()
{
	// Rotating cube
	m_fbo.begin();
	ofEnableDepthTest();
	ofClear(10, 100, 140, 255);
	ofPushMatrix();
	ofTranslate((float)senderWidth/2.0, (float)senderHeight/2.0, 0);
	ofRotateYDeg(rotX);
	ofRotateXDeg(rotY);
	textureImage.getTexture().bind();
	ofDrawBox(0.4*(float)senderHeight);
	textureImage.getTexture().unbind();
	ofPopMatrix();
	ofDisableDepthTest();
	m_fbo.end();

	// Rotate the cube (best for 30 fps)
	rotX += 1.5;
	rotY += 1.5;

}

//--------------------------------------------------------------
//
// Audio waveform graph
//
void ofApp::DrawAudio()
{
	if(lAudio.empty() || rAudio.empty())
		return;

	// Audio data is -1.0 - +1.0
	// increase to +- 1/4 the window height
	// Maximum height of the waveform graph
	float height = (float)(ofGetHeight()/4);
	float ypos  = 0.0f;
	float lasty = 0.0f;;
	float xpos  = 0.0f;
	float lastx = 0.0f;;

	// nSamples spaced over the window width
	float spacing = (float)ofGetWidth()/(int)lAudio.size();
	float y = (float)(ofGetHeight()/2); // Centre of the window
	for (int i=0; i < (int)lAudio.size(); i++) {
		xpos = (float)i*spacing;
		// Average of left and right channels
		ypos = y + ((lAudio[i]*height) + (rAudio[i]*height))/2;
		if (xpos > lastx) ofDrawLine(lastx, lasty, xpos, ypos);
		lastx = xpos;
		lasty = ypos;
	}

}


//--------------------------------------------------------------
void ofApp::audioIn(ofSoundBuffer& input)
{
	// Soundstream must be initialized
	if(!bSoundStream)
		return;

	// Mutex lock for variables shared with draw
	// (availableSamples and audioBuffer)
	std::unique_lock<std::mutex> lock(audioMutex);
	// Soundstream audio is interleaved
	// L R L R L R L R L R ...
	for (size_t i = 0; i <input.size(); i++) {
		audioBuffer[writeIndex] = input[i];
		writeIndex = (writeIndex + 1) % bufferCapacity;
		availableSamples++;
	}

}

//--------------------------------------------------------------
void ofApp::exit()
{
	// Stop and close soundstream
	soundStream.close();
	// Release the sender
	// This releases the audio data buffer
	ndiSender.ReleaseSender();
}
