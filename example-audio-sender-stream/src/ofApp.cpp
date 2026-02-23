#include "ofApp.h"

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

	//
	// Clock video
	//
	videoFps = 30.00;
	ndiSender.SetFrameRate(videoFps);

	// Important
	// Decouple openframeworks from NDI sending
	ndiSender.SetClockVideo(false);

	// Stereo at 48000/30fps requires 1600 samples per frame
	// Soundstream rounds up to the next power of 2
	nSamples = 2048;
	nChannels = 2; // Stereo
	sampleRate = 48000; // For NDI and Soundstream

	// NDI sender
	ndiSender.SetAudio(true); // Allow audio
	ndiSender.SetAudioSamples(nSamples); // Important - must match soundstream
	ndiSender.SetAudioChannels(nChannels);
	ndiSender.SetAudioSampleRate(sampleRate);

	// Audio data is  32 bit float interleaved
	ndiSender.SetAudioType(audio_frame_interleaved_32f_t);

	// Create an NDI sender with YUV output format
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
	
	// Set up soundstream
	ofSoundStreamSettings settings;
	auto alldevices = soundStream.getDeviceList();

	printf("\nSoundstream device list\n");
	for (int i=0; i < (int)alldevices.size(); i++) {
		printf("  (%d) %s (ID = %d, API = %d)\n",
			i, alldevices[i].name.c_str(),
			alldevices[i].deviceID,
			alldevices[i].api);
	}

	// Select the device number as required
	// TODO - test in different systems
	settings.setInDevice(alldevices[0]); // Speakers
	settings.setInListener(this);
	settings.sampleRate = sampleRate;
	settings.numInputChannels = nChannels;
	settings.bufferSize = nSamples; // 2048
	soundStream.setup(settings);

	printf("\nSoundstream setup\n");
	printf("  nSamples     = %d --> %d\n", nSamples, soundStream.getBufferSize());
	printf("  Sample rate  = %d --> %d\n", (int)settings.sampleRate, soundStream.getSampleRate());
	printf("  N channels   = %d --> %d\n", (int)settings.numInputChannels, soundStream.getNumInputChannels());
	printf("\n");

	lAudio.assign(nSamples,  0.0);
	rAudio.assign(nSamples, 0.0);

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

}

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

	
	/*
	// Plane for testing
	m_fbo.begin();
	ofClear(10, 100, 140, 255);
	ofPushMatrix();
	ofTranslate((float)senderWidth / 2.0, (float)senderHeight / 2.0, 0);
	ofRotateYDeg(rotX);
	textureImage.getTexture().bind();
	ofDrawPlane(0, 0, (float)senderHeight/3, (float)senderHeight/3);
	textureImage.getTexture().unbind();
	ofPopMatrix();
	m_fbo.end();
	*/

	// Draw the fbo result fitted to the display window
	m_fbo.draw(0, 0, ofGetWidth(), ofGetHeight());

	// Rotate the cube (30 fps)
	rotX += 1.5;
	rotY += 1.5;

}

void ofApp::DrawAudio()
{
	// Soudstream must be initialized
	if(!bSoundStream)
		return;

	// Local copy of vectors to minimize mutex lock time
	{
        std::unique_lock<std::mutex> lock(audioMutex);
		if(lAudio.empty() || rAudio.empty())
			return;
        lCopy = lAudio;
        rCopy = rAudio;
    }

	//
	// Left channel:
	//

	// Audio data is -1.0 - +1.0
	// increase to +- third the window height
	float height = (float)(ofGetHeight()/3);
	float ypos = 0.0f;
	float lasty = ypos;
	float xpos = 0.0f;
	float lastx = xpos;
	ofSetLineWidth(2);

	// nSamples spaced over the window width
	float spacing = (float)ofGetWidth()/(int)lAudio.size()*2; // (float)nSamples*2;
	float y = (float)(ofGetHeight()/4); // Centre of top half
	for (int i=0; i < (int)lAudio.size()*2; i+=2) { // Every second even sample
		xpos = (float)i*spacing;
		ypos = y + (lCopy[i]*height);
		if (xpos > lastx) ofDrawLine(lastx, lasty, xpos, ypos);
		lastx = xpos;
		lasty = ypos;
	}

	//
	// Right channel
	//
	y = (float)(ofGetHeight()*3/4); // Centre of bottom half
	for (int i=1; i < (int)rAudio.size()*2; i+=2) { // Every second odd sample
		xpos = (float)i*spacing;
		ypos = y + (rCopy[i]*height);
		if (xpos > lastx) ofDrawLine(lastx, lasty, xpos, ypos);
		lastx = xpos;
		lasty = ypos;
	}

}

//--------------------------------------------------------------
void ofApp::audioIn(ofSoundBuffer& input)
{
	if (bInitialized && input.getBuffer().data()) {
		ndiSender.SetAudioData(input.getBuffer().data());
		ndiSender.SendAudio();
	}

	// samples are interleaved
	// L R L R L R .. etc
	for (size_t i = 0; i < input.getNumFrames(); i+=2){
		lAudio[i/2] = input[i];
		rAudio[i/2] = input[i+1];
	}

}


//--------------------------------------------------------------
void ofApp::exit()
{
	// Close soundstream
	soundStream.close();

	// Release the sender
	// This releases the audio data buffer
	ndiSender.ReleaseSender();
}
