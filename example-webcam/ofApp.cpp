/*
	OpenFrameworks NDI webcam sender example
	using the NewTek NDI SDK to send the frames via network
	Advanced example using a shader for RGBA-BGRA conversion

	http://NDI.NewTek.com

	Copyright (C) 2016-2017 Lynn Jarvis.

	http://www.spout.zeal.co

	With help from Harvey Buchan

	https://github.com/Harvey3141

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

	13.11.16 - sender example using a webcam
	09.02.17 - Updated to ofxNDI with Version 2 NDI SDK
			 - Added changes by Harvey Buchan to optionally
			   specify RGBA with Version 2 NDI SDK

*/
#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

	ofSetColor(255);

	strcpy(senderName, "Openframeworks NDI webcam"); // Set the sender name
	ofSetWindowTitle(senderName); // show it on the title bar
	cout << "NDI SDK copyright NewTek (http:\\NDI.NewTek.com)" << endl;

	camWidth  = 640; // try to grab at this size. 
	camHeight = 480;
	vidGrabber.setDeviceID(0);
	vidGrabber.setDesiredFrameRate(30); // try to set this frame rate
	vidGrabber.initGrabber(camWidth, camHeight);
	// Check width and height
	camWidth  = vidGrabber.getWidth();
	camHeight = vidGrabber.getHeight();
	cout << "Initialized webcam (" << camWidth << " x " << camHeight << ")" << endl;
	ofSetVerticalSync(true);

	// Set the desired frame rate for NDI
	// This will cause sendImage to clock the video
	// and limit the Openframeworks frame rate as well
	// unless asynchronous sending is selected below.
	//    60    60000 / 1000
	//    29.97 30000 / 1001
	ndiSender.SetFrameRate(30000, 1001); // 29.97 fps

	// Initialize rgba pixel buffers for NDI
	ndiBuffer[0].allocate(camWidth, camHeight, 4);
	ndiBuffer[1].allocate(camWidth, camHeight, 4);

	// Optionally set NDI asynchronous sending instead of clocked at the set frame rate
	ndiSender.SetAsync(false); // change to true for async before creating the sender
	idx = 0; // index used for buffer swapping

	// Create a new sender
	// Specify RGBA format here otherwise the default is BGRA
	ndiSender.CreateSender(senderName, camWidth, camHeight, NDIlib_FourCC_type_RGBA);
	cout << "Created NDI sender [" << senderName << "] (" << camWidth << "x" << camHeight << ")" << endl;

}

//--------------------------------------------------------------
void ofApp::update() {

	ofBackground(100,100,100);
	vidGrabber.update();
	
	if (vidGrabber.isFrameNew()) {

		if (ndiSender.GetAsync())
			idx = (idx + 1) % 2;

		ndiBuffer[idx].setFromPixels(vidGrabber.getPixels(), camWidth, camHeight, OF_IMAGE_COLOR); // RGB from camera
		ndiBuffer[idx].setImageType(OF_IMAGE_COLOR_ALPHA); // RGBA for NDI

	}

}

//--------------------------------------------------------------
void ofApp::draw() {

	vidGrabber.draw(0, 0, ofGetWidth(), ofGetHeight());

	// Send the rgba pixel buffer to NDI
	// If you did not set the sender pixel format to rgba in CreateSender
	// you can convert to bgra within SendImage (specify true for bSwapRB)
	if (ndiSender.SendImage(ndiBuffer[idx].getPixels(), camWidth, camHeight)) {
        // Show the sender name and fps
		char str[256];
        sprintf(str, "Sending as : [%s] (%dx%d)", senderName, camWidth, camHeight);
		ofDrawBitmapString(str, 20, 30);
        sprintf(str, "fps: %3.3d", (int)ofGetFrameRate());
        ofDrawBitmapString(str, ofGetWidth()-120, 30);
	}

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}

