/*
	OpenFrameworks NDI webcam sender example
	using the NewTek NDI SDK to send the frames via network

	http://NDI.NewTek.com

	Copyright (C) 2016-2022 Lynn Jarvis.

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
	22.02.17 - Update to Openframeworks 0.9.8
	01.04.18 - Change to ofxNDI for NDI SDK Version 3
	06.08.18 - Update to Openframeworks 10
			   Revise for ofxNDI for NDI SDK Version 3.5
	10.11.19 - Revise for ofxNDI for NDI SDK Version 4.0
	10.11.19 - Revise for ofxNDI for NDI SDK Version 5.1
	05.01.21 - Allow user selection of webcam

*/
#include "ofApp.h"
#include <conio.h> // for getch

//--------------------------------------------------------------
void ofApp::setup(){

	ofSetColor(255);

	ofSetWindowTitle("Openframeworks NDI webcam"); // show the sender name on the title bar
#ifdef _WIN64
	cout << "\nofxNDI example webcam sender - 64 bit" << endl;
#else // _WIN64
	cout << "\nofxNDI example webcam sender - 32 bit" << endl;
#endif // _WIN64	
	cout << ndiSender.GetNDIversion() << " (https://www.ndi.tv/)" << endl;

	// Save webcam devices to show the user
	vector<ofVideoDevice> devices = vidGrabber.listDevices();

	// User webcam selection
	printf("Select the index of the device required (0 - %d)\n", devices.size() - 1);
	int index = _getch() - 48;
	if (index > (int)devices.size()-1 || index < 0)
		index = 0;

	// Set up webcam
	vidGrabber.setDeviceID(index); // The first or selected webcam
	vidGrabber.setup(640, 480); // try to grab at this size.

	// Set NDI asynchronous sending for best performance
	ndiSender.SetAsync();

	// Create a new sender - default RGBA for ofPixels
	ndiSender.CreateSender("Openframeworks NDI webcam", (unsigned int)vidGrabber.getWidth(), (unsigned int)vidGrabber.getHeight());
	cout << "Created NDI sender [Openframeworks NDI webcam] (" << vidGrabber.getWidth() << "x" << vidGrabber.getHeight() << ")" << endl;

}

//--------------------------------------------------------------
void ofApp::update() {

	vidGrabber.update();

}

//--------------------------------------------------------------
void ofApp::draw() {

	vidGrabber.draw(0, 0, ofGetWidth(), ofGetHeight());
	if (vidGrabber.isFrameNew())
		ndiSender.SendImage(vidGrabber.getPixels());

}


