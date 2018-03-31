/*

	OpenFrameworks NDI receiver 
	using the NewTek NDI SDK to receive frames via network

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

*/
#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {

	ofBackground(0);
	ofSetColor(255);

	// Set the window title to show that it is a receiver
	ofSetWindowTitle("Openframeworks NDI receiver");

	#ifdef _WIN64
	cout << "\nofxNDI example receiver - 64 bit" << endl;
	#else // _WIN64
	cout << "\nofxNDI example receiver - 32 bit" << endl;
	#endif // _WIN64

	cout << "NDI SDK copyright NewTek (http:\\NDI.NewTek.com)" << endl;
	cout << "Press 'SPACE' to list NDI senders" << endl;
	
	senderName[0] = 0;    // The sender name used for display
	nSenders = 0;         // Total number of NDI senders
	senderWidth = 0;      // Sender width
	senderHeight = 0;     // Sender height
	bNDIreceiver = false; // Receiver creation

	// Create an intial receiving image
	ndiImage.allocate(ofGetWidth(), ofGetHeight(), OF_IMAGE_COLOR_ALPHA);

	// For received frame fps calculations - independent of the rendering rate
	startTime = lastTime = frameTime = 0;
	fps = frameRate = 1; // starting value

} // end setup

//--------------------------------------------------------------
void ofApp::update() {

}

//--------------------------------------------------------------
void ofApp::draw() {

	char str[256];
	unsigned int width = 0;
	unsigned int height = 0;

	// Update the NDI sender list to find new senders
	// There is no delay if no new senders are found
	nSenders = ndiReceiver.FindSenders();
	
	if(nSenders > 0) {

		// Has the user changed the sender index ?
		if(ndiReceiver.SenderSelected()) {
			// Release the current receiver.
			// A new one is then created from the selected sender index.
			ndiReceiver.ReleaseReceiver();
			bNDIreceiver = false;
		}

		// Create a new receiver if one does not exist.
		// We don't know the sender dimensions until a frame is received.
		if(!bNDIreceiver) {

			// The receiver will detect which format a sender is using and convert
			// the pixel buffer to RGBA for return. Using BGRA is more efficient than YUV
			// due to optimized copy functions.
			bNDIreceiver = ndiReceiver.CreateReceiver(NDIlib_recv_color_format_BGRX_BGRA);

			// ==========================================================================
			//
			// Note for RGBA Version 3
			//
			// We can specify that RGBA is the preferred format for NDI
			// to provide to the receiver.
			//
			// bNDIreceiver = ndiReceiver.CreateReceiver(NDIlib_recv_color_format_RGBX_RGBA);
			//
			// With Version 3, RGBA format causes the application to crash.
			// Newtek has been advised and a patch provided that fixes the problem
			// but, as at Version 3.1.0.10, the patch has not been included in the SDK.
			// Subsequent versions may have the patch included. However, the pixel data
			// still has to be copied to the Openframeworks buffer so performance
			// difference is marginal.
			//
			// Frame 1920x1080 : Vsync off
			// Normal : RGBA 20.0 msec : BGRA 20.2 msec
			// Async  : RGBA 16.7 msec : BGRA 17.2 msec
			//
			// ==========================================================================

			//
			// A receiver is created from an index into a list of sender names.
			// The current user selected index is saved in the NDIreceiver class
			// and is used to create the receiver unless you specify a particular index.
			//
			// The name of the sender can also be retrieved if you need it.
			// If you specified a particular sender index to create the receiver
			// use that index to retrieve it's name.
			//
			// In this application we use it to display the sender name.
			//
			if (bNDIreceiver) {
				ndiReceiver.GetSenderName(senderName);
				cout << "Created NDI receiver for " << senderName << endl;
			}

			// Reset the starting values for frame rate calculations
			fps = frameRate = 1;

		}

	}

	// Receive an image from the NDI sender
	if(bNDIreceiver) {

		// If the NDI sender uses BGRA format, the received buffer is converted to rgba by ReceiveImage.
		// Optionally you can flip the image if necessary.
		if(ndiReceiver.ReceiveImage(ndiImage.getPixelsRef().getPixels(), width, height, false)) {  // receives as rgba

			// ----------------------------
			// Calculate received frame fps
			lastTime = startTime;
			startTime = ofGetElapsedTimeMicros();
			frameTime = (startTime - lastTime)/1000000; // seconds

			if( frameTime  > 0.01) {
				frameRate = floor(1.0/frameTime + 0.5);
				// damping from a starting fps value
				fps *= 0.95;
				fps += 0.05*frameRate;
			}
			// ----------------------------

			// Have the NDI sender dimensions changed ?
			if(senderWidth != width || senderHeight != height) {

				// Update the sender dimensions
				senderWidth  = width;
				senderHeight = height;
				
				// Update the receiving image size
				ndiImage.allocate(senderWidth, senderHeight, OF_IMAGE_COLOR_ALPHA);

				return; // get pixel data on the next frame
				
			}

			ndiImage.update();
		}

		//
		// Draw the current image.
		//
		// If receiveimage fails, the connection could be down so keep waiting for it to come back up.
		// Or the frame rate of the NDI sender can be less than the receiver draw cycle.
		//
		if(bNDIreceiver) 
			ndiImage.draw(0, 0, ofGetWidth(), ofGetHeight());

		// Show fps etc.
		if(nSenders > 0) {
			if(bNDIreceiver) {
				if(senderWidth > 0 && senderHeight > 0)
					sprintf_s(str, 256, "[%s] (%dx%d) - fps %2.0f", senderName, senderWidth, senderHeight, fps);
				else // connected but nothing received yet so the frame size is unknown
					sprintf_s(str, 256, "Connected to [%s]", senderName);
				ofDrawBitmapString(str, 20, 30);
			}

			if(nSenders == 1) {
				ofDrawBitmapString("1 network source", 20, ofGetHeight()-20);
			}
			else {
				sprintf_s(str, 256, "%d network sources", nSenders);
				ofDrawBitmapString(str, 20, ofGetHeight()-40);
				ofDrawBitmapString("'SPACE' to list senders", 20, ofGetHeight()-20);
			}
		}
	}
	else {
		ofDrawBitmapString("Connecting . . .", 20, 30);
	}


}


//--------------------------------------------------------------
void ofApp::keyPressed(int key) {

	char name[256];
	int index = key-48;

	nSenders = ndiReceiver.GetSenderCount();

	if(key == ' ') {
		// List all the senders
		if(nSenders > 0) {
			cout << "Number of NDI senders found: " << nSenders << endl;
			for (int i = 0; i < nSenders; i++) {
				ndiReceiver.GetSenderName(name, i);
				cout << "    Sender " << i << " [" << name << "]" << endl;
			}
			if(nSenders > 1)
				cout << "Press key [0] to [" << nSenders-1 << "] to select a sender" << endl;
		}
		else 
			cout << "No NDI senders found" << endl;
	}
	else if(index >= 0 && index < nSenders) {
		// Update the receiver with the returned index
		// "SenderSelected" will then return true in Draw() to update the receiver
		ndiReceiver.SetSenderIndex(index);
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

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
