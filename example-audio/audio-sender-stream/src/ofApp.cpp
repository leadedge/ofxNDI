#include "ofApp.h"

/*

	Openframeworks ofxNDI audio example

	Demonstrates sending system audio together with graphics

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
	senderName = "ofxNDI audio stream sender";
	ofSetWindowTitle(senderName); // show it on the title bar

	// NDI sender dimensions
	senderWidth  = 1280;
	senderHeight =  720;

	// Video frame rate
	videoFps = 30.00;
	ndiSender.SetFrameRate(videoFps);

	// Important
	// Decouple openframeworks frame rate from NDI sending
	ndiSender.SetClockVideo(false);

	//
	// Set up soundstream
	//

	nChannels = 2; // Stereo
	sampleRate = 48000; // For NDI and Soundstream

	// Stereo at 48000/30fps requires 1600 samples per frame
	// Soundstream rounds up to the next power of 2 (2048)
	nSamples = 2048;
	
	ofSoundStreamSettings settings;
	auto alldevices = soundStream.getDeviceList();
	// Select the device number as required by your system
	settings.setInDevice(alldevices[0]); // Speakers
	settings.setInListener(this);
	settings.sampleRate = sampleRate;
	settings.numInputChannels = nChannels;
	settings.bufferSize = nSamples;
	if (soundStream.setup(settings)) {
		// printf("\nSoundstream setup\n");
		// printf("  nSamples     = %d --> %d\n", nSamples, soundStream.getBufferSize());
		// printf("  Sample rate  = %d --> %d\n", (int)settings.sampleRate, soundStream.getSampleRate());
		// printf("  N channels   = %d --> %d\n", (int)settings.numInputChannels, soundStream.getNumInputChannels());
		// printf("\n");
		lAudio.assign(nSamples, 0.0);
		rAudio.assign(nSamples, 0.0);
		// Make sure the NDI sender and soundstream use the same sample number
		nSamples = soundStream.getBufferSize();
	}
	else {
		printf("Soundstream setup failed\n");
	}

	// NDI sender
	ndiSender.SetAudio(true); // Allow audio
	ndiSender.SetAudioSamples(nSamples); // Important - must match soundstream
	ndiSender.SetAudioChannels(nChannels);
	ndiSender.SetAudioSampleRate(sampleRate);

	// Soundstream audio data is float interleaved
	ndiSender.SetAudioType(audio_frame_interleaved_32f_t);

	// Create an NDI sender with YUV output format for best speed
	ndiSender.SetFormat(NDIlib_FourCC_video_type_UYVY);
	bInitialized = ndiSender.CreateSender(senderName.c_str(), senderWidth, senderHeight);
	if(bInitialized)
		printf("Created sender [%s]\n", senderName.c_str());
	else
		printf("CreateSender failed\n");

	// Allocate an RGBA fbo for texture draw
	m_fbo.allocate(senderWidth, senderHeight, GL_RGBA);

	// 3D drawing setup for the demo graphics
	glEnable(GL_DEPTH_TEST); // enable depth comparisons and update the depth buffer
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	ofDisableAlphaBlending(); // To prevent trails with the rotating cube
	// ofDisableArbTex is needed to create a texture with
	// normalized coordinates for bind in DrawGraphics
	ofDisableArbTex();
	textureImage.load("SpoutBox.jpg");
	ofEnableArbTex();
	

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
	if (!bInitialized)
		return;

	// Draw graphics
	DrawGraphics();

	// Draw the audio waveform
	DrawAudio();

	// Send video
	ndiSender.SendImage(m_fbo);

	// SendAudio is independent in AudioIn

}

//--------------------------------------------------------------
void ofApp::DrawGraphics()
{
	// Draw graphics into an fbo

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

	// Draw the fbo result fitted to the display window
	m_fbo.draw(0, 0, ofGetWidth(), ofGetHeight());

	// Rotate the cube (30 fps)
	rotX += 1.5;
	rotY += 1.5;

}

//--------------------------------------------------------------
void ofApp::DrawAudio()
{
	// Local copy of vectors to minimize mutex lock time
	{
        std::unique_lock<std::mutex> lock(audioMutex);
		// lAudio/rAudio vectors must be assigned
		if(lAudio.empty() || rAudio.empty())
			return;
        lCopy = lAudio;
        rCopy = rAudio;
    }

	//
	// Calculate volume (interleaved audio data)
	//
	float leftVol = 0.0f;
	float rightVol = 0.0f;
	float count = 0.0f;
	for (int i=0; i < (int)lCopy.size(); i++) {
		leftVol  += lCopy[i]*lCopy[i];
		rightVol += rCopy[i]*rCopy[i];
		count++;
	}
	leftVol  = sqrt(leftVol/count);  // left rms
	rightVol = sqrt(rightVol/count); // right rms

	//
	// DB bar graph
	//

	//
	// Left channel
	//
	// Clamp rms values to avoid log 0 and convert rms to db
	float dB = 20.0f*log10(ofClamp(leftVol, 0.000001f, 1.0f));
	// map to 2/3 window height
	float mapDB = ofMap(dB, -60, 0, 0, ofGetHeight()*2/3, true);
	drawGradientBar(10, ofGetHeight()-mapDB, 7, mapDB);
	//
	// Right channel
	//
	dB = 20.0f*log10(ofClamp(rightVol, 0.000001f, 1.0f));
	mapDB = ofMap(dB, -60, 0, 0, ofGetHeight()*2/3, true);
	drawGradientBar(20, ofGetHeight()-mapDB, 7, mapDB);

	//
	// Audio waveform graph
	//

	// Audio data is -1.0 - +1.0
	// increase to +- 1/3 the window height
	// Maximum height of the waveform graph
	float height = (float)(ofGetHeight()/3);
	float ypos = 0.0f;
	float lasty = ypos;
	float xpos = 0.0f;
	float lastx = xpos;

	// Audio is interleaved : L R L R L R .....
	// nSamples spaced over the window width
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

// Similar to Studio Monitor
void ofApp::drawGradientBar(float x, float y, float width, float height)
{
    ofMesh mesh;
    mesh.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);

    // Bottom color (Blue)
    mesh.addVertex(glm::vec3(x, y+height, 0));
	mesh.addColor(ofColor(0, 64, 255));

    mesh.addVertex(glm::vec3(x+width, y+height, 0));
	mesh.addColor(ofColor(0, 64, 255));

    // Middle color (Green)
    float midY = y+height*0.10f;

    mesh.addVertex(glm::vec3(x, midY, 0));
	mesh.addColor(ofColor(0, 225, 0));

    mesh.addVertex(glm::vec3(x+width, midY, 0));
	mesh.addColor(ofColor(0, 225, 0));

    mesh.draw();
}

//--------------------------------------------------------------
void ofApp::audioIn(ofSoundBuffer& input)
{
	// Soundstream must be initialized
	// and lAudio/rAudio vectors assigned
	if(lAudio.empty() || rAudio.empty())
		return;

	// Send audio independently of graphics in draw
	if (bInitialized && input.getBuffer().data()) {
		ndiSender.SetAudioData(input.getBuffer().data());
		ndiSender.SendAudio();
	}

	// SoundStream samples are interleaved
	// L R L R L R L R L R L R L R .. etc
	for (size_t i = 0; i < input.getNumFrames()*input.getNumChannels(); i+=2){
		lAudio[i/2] = input[i];
		rAudio[i/2] = input[i+1];
	}

}

//--------------------------------------------------------------
void ofApp::exit()
{
	// Stop and close soundstream
	soundStream.stop();
	soundStream.close();

	// Release the sender
	// This releases the audio data buffer
	ndiSender.ReleaseSender();
}
