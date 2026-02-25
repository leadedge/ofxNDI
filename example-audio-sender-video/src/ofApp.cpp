#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

	ofBackground(0);

	//
	// Input video files
	// 30 FPS video, 44.1 Khz Audio
	//
	// std::string inputVideo = ofToDataPath("Audio-Video-Sync-Test.mp4", true);
	// https://www.youtube.com/watch?v=ucZl6vQ_8Uo (Created by DJO)
	//
	// std::string inputVideo = ofToDataPath("Audio-Video-Sync-Test-Card.mp4", true);
	// https://www.youtube.com/watch?v=QzomK1fdSUg
	//
	std::string inputVideo = ofToDataPath("EasyLife-Audio-Video-Sync.mp4", true);
	// https://www.youtube.com/watch?v=K0uxjdcZWyk

	printf("Video file\n%s\n", inputVideo.c_str());
	
	// Create an mp3 audio file from the video file on the first pass.
	// Requires FFmpeg to extract an audio file from the video.
	// Returns the existing path if the file exists.
	std::string inputAudio = audiofile.CreateAudioFile(inputVideo);
	printf("Audio file\n%s\n", inputAudio.c_str());

	// Get the video file fps using ffprobe
	double fps = 30.0; // 30fps default
	if (!inputVideo.empty()) {
		audiofile.GetVideoFps(inputVideo, fps);
		printf("Video fps = %.2f\n", fps);
	}

	// For Video, set frame rate to that of the video
	ndiSender.SetFrameRate(fps);

	// Clock to video for the NDI sender 
	// Audio is submitted at the video frame rate
	ndiSender.SetClockVideo(true);

    // Audio file full path
	if (inputAudio.empty()) {
		ndiSender.SetAudio(false);
	}
	else {
		// Initialize the miniaudio decoder for 32-bit floating point audio
		// Floating point data is required by NDI
		int audioformat = 0;
		if (audiofile.InitializeDecoder(inputAudio, audioformat, nChannels, sampleRate)) {
			//
			// Audio/Video rate is not necessarily a whole number.
			// Use the audio file sample rate and video fps to calculate a sequence
			// For example, 48000hz audio requires 1601.6 samples per frame at 29.97 fps.
			//   1602, 1601, 1602, 1601, 1602
			// and 44100 requires 1471.470
			//   1472, 1471, 1472, 1471, 1471,
			// A sequence length of 5 is usually sufficient
			//
			int samplemax = 0; // Maximum sample number for buffer allocation
			audiosamples = ofxNDIutils::AudioFrameSequence(sampleRate, ndiSender.GetFrameRate(), samplemax, 5);
			if (!audiosamples.empty()) {
				// Set to use audio
				ndiSender.SetAudio(true);
				// MiniAudio returns interleaved 32 bit float data.
				// Set the audio type so that this data is sent using :
				//    "util_send_send_audio_interleaved_32f"
				ndiSender.SetAudioType(audio_frame_interleaved_32f_t);
				// Use the audio file decoder sample rate and number of channels
				ndiSender.SetAudioSampleRate(sampleRate);
				ndiSender.SetAudioChannels(nChannels);
				// This is changed according to the sample sequence
				ndiSender.SetAudioSamples(samplemax);
				// Allocate for the maximum sample number in the sequence
				float* p_data = (float*)malloc(samplemax * nChannels * sizeof(float));
				ndiSender.SetAudioData(p_data);
			}
		}
		else {
			printf("MiniAudio could not decode the audio file\n");
		}
	}

	// NDI sender name
	senderName = "ofxNDI audio video sender";
	ofSetWindowTitle(senderName); // show it on the title bar

	// load the video file
	video.load(inputVideo);

	// Set the dimensions for NDI sender output
	// Resolution affects performance
	// Use 640x360 for low bandwidth WIFI 
	senderWidth  = 1280;
	senderHeight = 720;

	// Allocate an RGBA fbo for texture draw
	m_fbo.allocate(senderWidth, senderHeight, GL_RGBA);

	// Create an NDI sender with YUV output format
	ndiSender.SetFormat(NDIlib_FourCC_video_type_UYVY);
	bInitialized = ndiSender.CreateSender(senderName.c_str(), senderWidth, senderHeight);
	if(bInitialized)
		printf("Created sender [%s]\n", senderName.c_str());
	else
		printf("CreateSender failed\n");

	// Set volume to zero if audio is handled by NDI locally
	// Leave system volume set if sending to another machine
	video.setVolume(0.0f);

	// Start the video
	video.play();

}

//--------------------------------------------------------------
void ofApp::update(){
	video.update(); // advance frames
}

//--------------------------------------------------------------
void ofApp::draw(){

	ofBackground(0);
	ofSetColor(255);

	// Check success of CreateSender
	if (!bInitialized)
		return;

	// Send audio if selected
	if (ndiSender.GetAudio()) {

		// Read data from the audio file using MiniAudio

		// Set the number of audio samples to read for this frame
		// based on the audio sample number sequence and it's size
		nSamples = audiosamples[audioindex % (int)audiosamples.size()];
		ndiSender.SetAudioSamples(nSamples);
		audioindex++;
		// Reset the index if it exceeds the sample size
		if (audioindex > (int)audiosamples.size())
			audioindex = 0;

		// Read the next lot of audio frames from the file
		// GetAudioData() is a pointer to allocated float audio frame data
		float* frames = ndiSender.GetAudioData();
		audiofile.ReadAudioFrames(frames, nSamples);
	
		// Set the new audio data to the sender audio frame
		ndiSender.SetAudioData(frames);

	}

	
	// Video texture format is RGB
	// Draw to an fbo allocated with alpha
	m_fbo.begin();
	video.draw(0, 0, m_fbo.getWidth(), m_fbo.getHeight());
	m_fbo.end();

	// Draw the fbo result fitted to the display window
	m_fbo.draw(0, 0, ofGetWidth(), ofGetHeight());

	// Draw the audio waveform
	DrawAudio();

	// Send audio
	// Video is clocked and audio is submitted at the video frame rate
	ndiSender.SendAudio();

	// Send video
	ndiSender.SendImage(m_fbo);

}

void ofApp::DrawAudio()
{
	if (!ndiSender.GetAudio() || !ndiSender.GetAudioData())
		return;


	// Default planar nSamples spaced over the window width
	float spacing = (float)ofGetWidth()/(float)nSamples;
	int yPos = ofGetHeight()/2;
	float y = (float)(ofGetHeight()/2);
	// Audio data is -1.0 - +1.0
	// increase to +- quarter height
	float height = (float)(ofGetHeight()/4);
	float ypos = 0.0f;
	float lasty = ypos;
	float xpos = 0.0f;
	float lastx = xpos;

	// Yellow lines
	ofSetColor(255, 255, 0, 255);
	ofSetLineWidth(2);

	//
	// Planar audio (default audio type)
	//
	if (ndiSender.GetAudioType() == 0) {

		//
		// Left channel
		//
		// Assign for planar : L L L L L ... R R R R R ...
		float* audiodata = ndiSender.GetAudioData();
		lAudio.assign(audiodata, audiodata + nSamples);
		y = (float)(ofGetHeight()/4); // Centre of top half
		for (int i=0; i < nSamples; i++) {
			xpos = (float)i * spacing;
			ypos = y + (lAudio[i] * height);
			if (xpos > lastx) ofDrawLine(lastx, lasty, xpos, ypos);
			lastx = xpos;
			lasty = ypos;
		}

		//
		// Right channel
		//
		audiodata = ndiSender.GetAudioData() + nSamples;
		rAudio.assign(audiodata, audiodata + nSamples);
		y = (float)(ofGetHeight() * 3 / 4); // Centre of bottom half
		for (int i=0; i < nSamples; i++) {
			xpos = (float)i * spacing;
			ypos = y + (rAudio[i] * height);
			if (xpos > lastx) ofDrawLine(lastx, lasty, xpos, ypos);
			lastx = xpos;
			lasty = ypos;
		}
	}
	//
	// Interleaved audio types
	//
	else {

		// nSamples*2 spaced over the window width
		spacing = (float)ofGetWidth()/(float)nSamples/2;

		//
		// Left channel
		//
		// Assign for interleaved : L R L R L R L R L R ...
		float* audiodata = ndiSender.GetAudioData();
		lAudio.assign(audiodata, audiodata+nSamples*2);
		y = (float)(ofGetHeight()/4); // Centre of top half
		for (int i=0; i < nSamples*2; i+=2) { // Every second even sample
			xpos = (float)i * spacing;
			ypos = y + (lAudio[i] * height);
			if (xpos > lastx) ofDrawLine(lastx, lasty, xpos, ypos);
			lastx = xpos;
			lasty = ypos;
		}

		//
		// Right channel
		//
		// Assign for interleaved starting at tha second channel
		audiodata = ndiSender.GetAudioData()+1;
		rAudio.assign(audiodata, audiodata+nSamples*2);
		y = (float)(ofGetHeight()*3/4); // Centre of bottom half
		for (int i=0; i < nSamples*2; i+=2) { // Every second odd sample
			xpos = (float)i * spacing;
			ypos = y + (rAudio[i] * height);
			if (xpos > lastx) ofDrawLine(lastx, lasty, xpos, ypos);
			lastx = xpos;
			lasty = ypos;
		}

	}

}

//--------------------------------------------------------------
void ofApp::exit() {
	// Release the sender or NDI sender discovery will still find it
	// This also releases the video and audio data buffers
	ndiSender.ReleaseSender();
}
