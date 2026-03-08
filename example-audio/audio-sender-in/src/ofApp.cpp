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
	float videofps = 30;
	ndiSender.SetFrameRate(videofps);

	// 30 fps NDI video frame rate
	// For soundstream audio rate of 48000 hz
	// ndiSender.SetFrameRate(30.00);

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
		// printf("\nSoundstream setup\n");
		// printf("  nSamples     = %d\n", soundStream.getBufferSize());
		// printf("  Sample rate  = %d\n", soundStream.getSampleRate());
		// printf("  N channels   = %d\n", soundStream.getNumOutputChannels());

	}
	else {
		printf("Soundstream setup failed\n");
	}

	audioBuffer.assign(nSamples*2, 0.0); // Number of audio samples per frame
	lAudio.assign(nSamples, 0.0); // Number of audio samples per channel
	rAudio.assign(nSamples, 0.0);

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

	// Draw graphics
	DrawGraphics();

	// Draw the fbo result fitted to the display window
	m_fbo.draw(0, 0, ofGetWidth(), ofGetHeight());

	// Draw the audio waveform
	DrawAudio();

	// Send video
	ndiSender.SendImage(m_fbo);

	// SendAudio is independent in AudioIn

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
		// shared with the audioIn thread
		std::unique_lock<std::mutex> lock(audioMutex);
		if(lAudio.empty() || rAudio.empty())
			return;
        lCopy = lAudio;
        rCopy = rAudio;
    }

	// Audio data is -1.0 - +1.0
	// increase to +- 1/3 the window height
	// Maximum height of the waveform graph
	float height = (float)(ofGetHeight()/3);
	float ypos = 0.0f;
	float lasty = ypos;
	float xpos = 0.0f;
	float lastx = xpos;

	// Audio is interleaved : L R L R L R .....
	// For one channel there are nSamples spaced over the window width
	float spacing = (float)ofGetWidth()/(int)lCopy.size();
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
void ofApp::audioIn(ofSoundBuffer& input)
{
	// Mutex lock for lAudio and rAudio
	// shared with DrawAudio in the draw thread
	std::unique_lock<std::mutex> lock(audioMutex);

	// Soundstream must be initialized
	// and lAudio/rAudio vectors assigned
	if(lAudio.empty() || rAudio.empty())
		return;

	// Send audio frames to NDI
	if (ndiSender.SenderCreated() && input.getBuffer().data()) {
		ndiSender.SetAudioData(input.getBuffer().data());
		ndiSender.SendAudio();
	}

	//
	// Fill the left and right channel audio vectors
	// for draw of the waveform graph in DrawAudio
	//
	// SoundStream samples are interleaved
	// L R L R L R L R L R L R L R .. etc
	// Samples per video frame
	for (size_t i = 0; i < input.getNumFrames()*input.getNumChannels(); i+=2){
		lAudio[i/2] = input[i]; // Samples per channel
		rAudio[i/2] = input[i+1];
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
