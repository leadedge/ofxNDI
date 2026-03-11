#include "ofApp.h"

/*

	Openframeworks ofxNDI audio example

	Demonstrates creating an audio tone
	and sending together with graphics

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
	senderName = "ofxNDI audio output sender";
	ofSetWindowTitle(senderName); // show it on the title bar

	// NDI sender dimensions
	senderWidth  = 1280;
	senderHeight =  720;

	// Set up NDI and soundtream to match with the audio that is generated.
	// In this example it's a tone but it could be specific to the application.

	// Video frame rate
	// 60, 30, 29.97 etc
	float videofps = 30;
	ndiSender.SetFrameRate(videofps);

	// Set up soundstream for audioOut
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
	auto devices = soundStream.getDeviceList();
	if (!devices.empty()) {
		// Select the device number as required by your system
		settings.setOutDevice(devices[0]); // Speakers
		settings.setOutListener(this);
		settings.sampleRate = sampleRate;
		settings.numOutputChannels = nChannels;
		settings.numInputChannels = 0;
		settings.bufferSize = nSamples;
		if (soundStream.setup(settings)) {
			// Make sure the NDI sender and soundstream
			// use the same sample number per channel
			nSamples = soundStream.getBufferSize();
			printf("\nSoundstream setup\n");
			printf("  nSamples     = %d\n", soundStream.getBufferSize());
			printf("  Sample rate  = %d\n", soundStream.getSampleRate());
			printf("  N channels   = %d\n", soundStream.getNumOutputChannels());
		}
		else {
			printf("Soundstream setup failed\n");
		}
	}

	audioBuffer.assign(nSamples*2, 0.0); // Number of audio samples per frame
	lAudio.assign(nSamples, 0.0); // Number of audio samples per channel
	rAudio.assign(nSamples, 0.0);

	// NDI sender
	ndiSender.SetAudio(true); // Allow audio
	ndiSender.SetAudioSampleRate(sampleRate);
	ndiSender.SetAudioChannels(nChannels);
	// Audio samples per channel matching soundstream
	// (interleaved audio data)
	ndiSender.SetAudioSamples(nSamples);
	// Audio data is float interleaved
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
	// to use draw together with audioOut
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

	// Draw graphics into the fbo
	DrawGraphics();

	// Draw the fbo result fitted to the display window
	m_fbo.draw(0, 0, ofGetWidth(), ofGetHeight());

	// Draw the audio waveform
	DrawAudio();

	// Send video
	ndiSender.SendImage(m_fbo);

	// SendAudio is independent in AudioOut

}

//--------------------------------------------------------------
void ofApp::DrawGraphics()
{
	// Rotating cube
	m_fbo.begin();
	ofEnableDepthTest();
	ofClear(10, 100, 140, 255);
	ofPushMatrix();
	ofTranslate((float)senderWidth / 2.0, (float)senderHeight / 2.0, 0);
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
	// Local copy of vectors to minimize mutex lock time
	{
		// Mutex lock for lAudio and rAudio
		// shared with the audioOut thread
		std::unique_lock<std::mutex> lock(audioMutex);
		// lAudio/rAudio vectors must be assigned
		if (lAudio.empty() || rAudio.empty())
			return;
		lCopy = lAudio;
		rCopy = rAudio;
	}

	// Audio data is -1.0 - +1.0
	// increase to +- 1/4 the window height
	// Maximum height of the waveform graph
	float height = (float)(ofGetHeight()/4);
	float ypos  = 0.0f;
	float lasty = 0.0f;
	float xpos  = 0.0f;
	float lastx = 0.0f;

	// Audio is interleaved : L R L R L R .....
	// nSamples spaced over the window width for one channel
	float spacing = (float)ofGetWidth()/(int)lAudio.size();
	float y = (float)(ofGetHeight()/2); // Centre of the window
	for (int i=0; i < (int)lCopy.size(); i++) {
		xpos = (float)i*spacing;
		// Average of left and right channels
		ypos = y + ((lCopy[i]*height) + (rCopy[i]*height))/2;
		if (xpos > lastx) ofDrawLine(lastx, lasty, xpos, ypos);
		lastx = xpos;
		lasty = ypos;
	}

}

//--------------------------------------------------------------
void ofApp::audioOut(ofSoundBuffer &buffer)
{
	// Mutex lock for lAudio and rAudio
	// shared with DrawAudio in the draw thread
	std::unique_lock<std::mutex> lock(audioMutex);

	// Tone generation
	frequency = 440.0f;
	float phaseIncrement = TWO_PI*frequency/sampleRate;
    for(size_t i = 0; i < nSamples; i++) { // Samples per video frame
        float sample = sin(phase); // -1 to +1
        phase += phaseIncrement;
        if(phase > TWO_PI)
			phase -= TWO_PI;
	    // Same signal for left and right channels
   		audioBuffer[i*nChannels  ] = sample;
		audioBuffer[i*nChannels+1] = sample;
		//
		// Enable these lines to play the tone through the speakers
		// Disable if the NDI receiver plays the audio.
		//
		// buffer[i*nChannels  ] = sample;
		// buffer[i*nChannels+1] = sample;
		//
	}

	// Send the interleaved audio data to NDI
	if (ndiSender.SenderCreated() && audioBuffer.data()) {
		ndiSender.SetAudioData(audioBuffer.data());
		ndiSender.SendAudio();
	}

	//
	// Fill left and right channel audio vectors
	// for the waveform graph in DrawAudio
	// Samples are interleaved
	// L R L R L R L R L R L R L R .. etc
	for (size_t i = 0; i < nSamples*2; i+=2) { // Samples per video frame
		lAudio[i/2] = audioBuffer[i]; // Samples per channel
		rAudio[i/2] = audioBuffer[i+1];
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
