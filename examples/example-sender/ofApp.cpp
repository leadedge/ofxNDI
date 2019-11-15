/*
	OpenFrameworks NDI sender example

	using the NewTek NDI SDK to send the frames via network

	http://NDI.NewTek.com

	Copyright (C) 2016-2018 Lynn Jarvis.

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

	16.10.16 - ofxNDI sender example
	03.11.16 - corrected CreateSender dimensions
			 - initialize pixel buffers
			 - BGRA conversion by OpenGL during glReadPixels
	07.11.16 - Included PBO for data read
	09.02.17 - Updated to ofxNDI with Version 2 NDI SDK
			 - Added changes by Harvey Buchan to optionally
			   specify RGBA with Version 2 NDI SDK
	22.02.17 - Updated to Openframeworks 0.9.8
	01.04.17 - update for ofxNDI with NDI Version 3
	06.11.17 - update for ofxNDI with NDI Version 3.5
			 - minor changes
	08.07.18 - Update for Openframeworks 10
			 - Include all SendImage option examples
	10.11.19 - Revise for ofxNDI for NDI SDK Version 4.0

*/
#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

	strcpy_s(senderName, 256, "Openframeworks NDI Sender"); // Set the sender name
	ofSetWindowTitle(senderName); // show it on the title bar

	#ifdef _WIN64
	cout << "\nofxNDI example sender - 64 bit" << endl;
	#else // _WIN64
	cout << "\nofxNDI example sender - 32 bit" << endl;
	#endif // _WIN64

	cout << ndiSender.GetNDIversion() << " (http://ndi.tv/)" << endl;

	// Set the dimensions of the sender output here
	// This is independent of the size of the display window
	senderWidth  = 1920;
	senderHeight = 1080;

	// Create an RGBA fbo for collection of data
	ndiFbo.allocate(senderWidth, senderHeight, GL_RGBA);

	// Optionally set fbo readback using OpenGL pixel buffers
	ndiSender.SetReadback(); // Change to false to compare

	// Optionally set NDI asynchronous sending
	// instead of clocked at the specified frame rate (60fps default)
	ndiSender.SetAsync();

	// Optionally set the framerate
	// Sending will clock at the set frame rate 
	// and over-ride asynchronous sending.
	// The application cycle will also be clocked at that rate
	//
	// Can be set as a whole number, e.g. 60, 30, 25 etc
	// ndiSender.SetFrameRate(30);
	//
	// Or as a decimal number e.g. 29.97
	// ndiSender.SetFrameRate(29.97);
	//
	// Or as a fraction numerator and denominator
	// as specified by the NDI SDK - e.g. 
	// NTSC 1080 : 30000, 1001 for 29.97 fps
	// NTSC  720 : 60000, 1001 for 59.94fps
	// PAL  1080 : 30000, 1200 for 25fps
	// PAL   720 : 60000, 1200 for 50fps
	// ndiSender.SetFrameRate(30000, 1001);

	// Create a sender with default RGBA output format
	ndiSender.CreateSender(senderName, senderWidth, senderHeight);

	cout << "Created NDI sender [" << senderName << "] (" << senderWidth << "x" << senderHeight << ")" << endl;

	// 3D drawing setup for the demo graphics
	glEnable(GL_DEPTH_TEST); // enable depth comparisons and update the depth buffer
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); // Really Nice Perspective Calculations
	ofDisableArbTex(); // needed for textures to work
	textureImage.load("NDI_Box.png"); // Load a texture image for the demo
	// Workaround for mirrored texture with ofDrawBox and ofBoxPrimitive for Openframeworks 10.
#if OF_VERSION_MINOR == 10
	textureImage.mirror(false, true);
#endif

	rotX = 0; // Cube rotation
	rotY = 0;

	// Image for pixel sending examples
	ndiImage.load("Test_Pattern.jpg");

}

//--------------------------------------------------------------
void ofApp::update() {


}

//--------------------------------------------------------------
void ofApp::draw() {

	ofBackground(0);
	ofColor(255);

	// Option 1 : Send ofFbo
	// ofFbo and ofTexture must be RGBA
	// Pixel data extraction from fbo or texture
	// is optimised using ndiSender.SetReadback();
	DrawGraphics();
	ndiSender.SendImage(ndiFbo);

	// Option 2 : Send ofTexture
	// DrawGraphics();
	// ndiSender.SendImage(ndiFbo.getTexture());

	// Option 3 Send ofImage
	// ofImage or ofPixels are converted to RGBA if not already
	// ndiImage.draw(0, 0, ofGetWidth(), ofGetHeight());
	// ndiSender.SendImage(ndiImage);

	// Option 4 Send ofPixels
	// ndiImage.draw(0, 0, ofGetWidth(), ofGetHeight());
	// ndiSender.SendImage(ndiImage.getPixels());

	// Option 5 Send char buffer
	// The pixels must be rgba
	// if (ndiImage.getImageType() != OF_IMAGE_COLOR_ALPHA)
	   // ndiImage.setImageType(OF_IMAGE_COLOR_ALPHA);
	// ndiImage.draw(0, 0, ofGetWidth(), ofGetHeight());
	// ndiSender.SendImage(ndiImage.getPixels().getData(), senderWidth, senderHeight);

	// Show what it is sending
	if (ndiSender.SenderCreated()) {
		char str[256];
		sprintf_s(str, 256, "Sending as : [%s] (%dx%d)", senderName, senderWidth, senderHeight);
		ofDrawBitmapString(str, 20, 30);
		// Show fps
		sprintf_s(str, 256, "fps: %3.3d (%4.2f)", (int)ofGetFrameRate(), ndiSender.GetFps());
		ofDrawBitmapString(str, ofGetWidth() - 140, 30);
	}

}

void ofApp::DrawGraphics() {

	ofDisableAlphaBlending(); // Or we can get trails with the rotating cube

	// Draw graphics into an fbo for the example
	ndiFbo.begin();

	// ======== your application draw goes here ========
	ofEnableDepthTest();
	ofClear(13, 25, 76, 255);

	ofPushMatrix();
	ofTranslate((float)senderWidth / 2.0, (float)senderHeight / 2.0, 0);
	ofRotateYDeg(rotX);
	ofRotateXDeg(rotY);
	textureImage.getTexture().bind();
	ofDrawBox(0.4*(float)senderHeight);
	ofPopMatrix();

	ofDisableDepthTest();
	rotX += 0.6;
	rotY += 0.6;
	// ============ end application draw ==============

	// End the fbo
	ndiFbo.end();

	// Draw the fbo result fitted to the display window
	ndiFbo.draw(0, 0, ofGetWidth(), ofGetHeight());
}

//--------------------------------------------------------------
void ofApp::exit() {
	// The sender must be released 
	// or NDI sender discovery will still find it
	ndiSender.ReleaseSender();
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

