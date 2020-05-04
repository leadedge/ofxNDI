/*

	OpenFrameworks NDI receiver

	using the NewTek NDI SDK to receive frames via network

	http://NDI.NewTek.com

	Copyright (C) 2016-2020 Lynn Jarvis.

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

	13.10.16 - Addon receiver example created
	14.10.16 - Included received frame rate
	03.11.16 - Receive into image pixels directly
			 - Add a sender selection dialog
	05.11.16 - Note - dialog is Windows only
			   the ofxNDIdialog class is used separately by the application
			   and can be omitted along with resources
	09.02.17 - Updated to ofxNDI with Version 2 NDI SDK
			 - Added changes by Harvey Buchan to optionally
			   specify preferred pixel format in CreateReceiver
	22.02.17 - updated to Openframeworks 0.9.8
			 - corrected reallocate on size change
	03.11.17 - update for ofxNDI with NDI Version 3
			 - remove sender dialog
			 - change NDI deprecated functions to Vers 3
	01.04.18 - Revise for NDI Version 3
			 - RGBA format not used due to SDK problem
	11.06.18 - Updated for NDI vers 3.5
			   ReceiveImage without memory copy to a pixel buffer
	12.07.18 - ReceiveImage ofFbo/ofTexture/ofImage
			 - All size change checks in ofxDNIreceiver class
	06.08.18 - Include all receiving options in example
	27.03.19 - Add example of using ReceiverCreated, ReceiverConnected and GetSenderFps
	10.11.19 - Revise for ofxNDI for NDI SDK Version 4.0
	28.02.20 - Remove initial texture clear.
			   Add received fps to on-screen display

*/
#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

	ofBackground(0);
	ofSetColor(255);

	// Set the window title to show that it is a receiver
	ofSetWindowTitle("Openframeworks NDI receiver");

#ifdef _WIN64
	cout << "\nofxNDI example receiver - 64 bit" << endl;
#else // _WIN64
	cout << "\nofxNDI example receiver - 32 bit" << endl;
#endif // _WIN64

	cout << ndiReceiver.GetNDIversion() << " (http://ndi.tv/)" << endl;
	cout << "Press 'SPACE' to list NDI senders" << endl;

	// ofFbo
	ndiFbo.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);

	// Clear the fbo so the first frame draw is black
	ndiFbo.begin();
	ofClear(0, 0, 0, 0);
	ndiFbo.end();

	// ofTexture
	ndiTexture.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);

	// ofImage
	ndiImage.allocate(ofGetWidth(), ofGetHeight(), OF_IMAGE_COLOR_ALPHA);

	// ofPixels
	ndiPixels.allocate(ofGetWidth(), ofGetHeight(), OF_IMAGE_COLOR_ALPHA);

	// unsigned char pixels
	ndiChars = new unsigned char[senderWidth*senderHeight * 4];

	// Sender dimensions and fps are not known yet
	senderWidth = (unsigned char)ofGetWidth();
	senderHeight = (unsigned char)ofGetHeight();

}


//--------------------------------------------------------------
void ofApp::update() {

}

//--------------------------------------------------------------
void ofApp::draw(){

	// Receive ofTexture
	ndiReceiver.ReceiveImage(ndiTexture);
	ndiTexture.draw(0, 0, ofGetWidth(), ofGetHeight());

	// Receive ofFbo
	// ndiReceiver.ReceiveImage(ndiFbo);
	// ndiFbo.draw(0, 0, ofGetWidth(), ofGetHeight());

	// Receive ofImage
	// ndiReceiver.ReceiveImage(ndiImage);
	// ndiImage.draw(0, 0, ofGetWidth(), ofGetHeight());

	// Receive ofPixels
	// ndiReceiver.ReceiveImage(ndiPixels);
	// ndiImage.setFromPixels(ndiPixels);
	// ndiImage.draw(0, 0, ofGetWidth(), ofGetHeight());

	/*
	// Receive unsigned char pixel image
	// ndiChars is the buffer to receive the pixels
	// buffer size must be managed if there is a sender size change
	unsigned int width = (unsigned int)ofGetWidth();
	unsigned int height = (unsigned int)ofGetHeight();
	if (ndiReceiver.ReceiveImage(ndiChars, width, height)) {
		if (width != senderWidth || height != senderHeight) {
			// Update sender dimensions
			senderWidth = width;
			senderHeight = height;
			// Reallocate the receiving buffer
			delete ndiChars;
			ndiChars = new unsigned char[senderWidth*senderHeight * 4];
			// Reallocate the image we use for display
			ndiImage.allocate(senderWidth, senderHeight, OF_IMAGE_COLOR_ALPHA);
		}
		else {
			// Update the display image
			ndiImage.getPixels().setFromExternalPixels(ndiChars, senderWidth, senderHeight, 4);
			ndiImage.update();
		}
	}
	// Draw whether received or not
	ndiImage.draw(0, 0, ofGetWidth(), ofGetHeight());
	*/

	// Show what it is receiving
	ShowInfo();

}

void ofApp::ShowInfo() {

	char str[256];

	int nsenders = ndiReceiver.GetSenderCount();
	if (nsenders > 0) {

		if (ndiReceiver.ReceiverCreated()) {
			if (ndiReceiver.ReceiverConnected()) {
				// Show received sender information and received fps
				sprintf_s(str, 256, "[%s] (%dx%d/%4.2fp) (fps %2.0f)", ndiReceiver.GetSenderName().c_str(), ndiReceiver.GetSenderWidth(), ndiReceiver.GetSenderHeight(), ndiReceiver.GetSenderFps(), ndiReceiver.GetFps());
			}
			else {
				// Nothing received
				sprintf_s(str, 256, "Connecting to [%s]", ndiReceiver.GetSenderName().c_str());
			}
		}
		ofDrawBitmapString(str, 20, 30);

		if (nsenders == 1) {
			ofDrawBitmapString("1 network source", 20, ofGetHeight() - 20);
		}
		else {
			sprintf_s(str, 256, "%d network sources", nsenders);
			ofDrawBitmapString(str, 20, ofGetHeight() - 40);
			ofDrawBitmapString("'SPACE' to list senders", 20, ofGetHeight() - 20);
		}
	}
	else {
		ofDrawBitmapString("Connecting . . .", 20, 30);
	}

}

//--------------------------------------------------------------
void ofApp::keyPressed  (int key){

	char name[256];
	int index = key - 48;

	int nsenders = ndiReceiver.GetSenderCount();

	if (key == ' ') {
		// List all the senders
		if (nsenders > 0) {
			cout << "Number of NDI senders found: " << nsenders << endl;
			for (int i = 0; i < nsenders; i++) {
				ndiReceiver.GetSenderName(name, 256, i);
				cout << "    Sender " << i << " [" << name << "]" << endl;
			}
			if (nsenders > 1)
				cout << "Press key [0] to [" << nsenders - 1 << "] to select a sender" << endl;
		}
		else
			cout << "No NDI senders found" << endl;
	}
	else if (nsenders > 0 && index >= 0 && index < nsenders) {
		// Update the receiver with the returned index
		// Returns false if the current sender is selected
		if (ndiReceiver.SetSenderIndex(index))
			cout << "Selected [" << ndiReceiver.GetSenderName(index) << "]" << endl;
		else
			cout << "Same sender" << endl;
	}

	
}

//--------------------------------------------------------------
void ofApp::keyReleased  (int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

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
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

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
