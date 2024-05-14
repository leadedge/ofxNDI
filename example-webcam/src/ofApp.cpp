/*
	OpenFrameworks NDI webcam sender example
	using the NewTek NDI SDK to send the frames via network

	http://NDI.NewTek.com

	Copyright (C) 2016-2024 Lynn Jarvis.

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
	26.04.22 - Update for Visual Studio 2022
	04.07.22 - Update with revised ofxNDI. Rebuild x64/MD.
	05-08-22 - Update to NDI 5.5 (ofxNDI and bin\Processing.NDI.Lib.x64.dll)
	20-11-22 - Update to NDI 5.5.2 (ofxNDI and bin\Processing.NDI.Lib.x64.dll)
	27-04-23 - Added select webcam details on main window
			   Update to NDI 5.5.2 (ofxNDI and bin\Processing.NDI.Lib.x64.dll)
			   Rebuild example executables x64/MD
	12-05-24 - Update to NDI 6.0.0 (ofxNDI and bin\Processing.NDI.Lib.x64.dll)
			   Rebuild example executable x64/MD to match Openframeworks videoInput.lib

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
	cout << camsender.GetNDIversion() << " (https://www.ndi.tv/)" << endl;

	camsendername = "Openframeworks NDI webcam"; // Set the sender name
	ofSetWindowTitle(camsendername); // show it on the title bar

	// Get the webcam list so that we can identify by name
	camdevices = vidGrabber.listDevices();
	printf("Select a webcam by it's index (0-%d)\n\n", (int)camdevices.size() - 1);

	// Use the default webcam (0) or change as required
	camindex = 0;
	vidGrabber.setDeviceID(camindex);
	vidGrabber.setDesiredFrameRate(30); // Try to set desired frame rate
	vidGrabber.setup(640, 480); // Try to grab at desired size
	cout << "Initialized webcam [" << camdevices[camindex].deviceName << "] " << vidGrabber.getWidth() << " x " << vidGrabber.getHeight() << ")" << endl;

	// Try to set this frame rate
	vidGrabber.setDesiredFrameRate(30);
	// Set Openframeworks to send frames at this rate
	ofSetFrameRate(30);
	// Set the NDI sender to this rate
	camsender.SetFrameRate(30.0);

	// Set NDI asynchronous sending for best performance
	camsender.SetAsync();

	// Create a new sender - default RGBA for ofPixels
	camsender.CreateSender(camsendername.c_str(), (unsigned int)vidGrabber.getWidth(), (unsigned int)vidGrabber.getHeight());
	cout << "Created NDI sender [" << camsendername << "]" << vidGrabber.getWidth() << "x" << vidGrabber.getHeight() << endl;

}

//--------------------------------------------------------------
void ofApp::update() {

	vidGrabber.update();

}

//--------------------------------------------------------------
void ofApp::draw() {

	std::string str;
	ofSetColor(255);

	vidGrabber.draw(0, 0, ofGetWidth(), ofGetHeight());
	if (vidGrabber.isFrameNew())
		camsender.SendImage(vidGrabber.getPixels());

	str = "Select a webcam by it's index";
	ofDrawBitmapString(str, 20, 30);

	// Show the webcam list for selection
	int ypos = 50;
	for (int i=0; i<(int)camdevices.size(); i++) {
		str = "("; str+= ofToString(i); str += ") ";
		str += camdevices[i].deviceName;
		ofDrawBitmapString(str, 40, ypos);
		ypos += 15;
	}
	str = "Press 0 to ";
	str += ofToString(camdevices.size()-1);
	str += " to select a webcam";
	ofDrawBitmapString(str, 40, ypos);

}

//--------------------------------------------------------------
void ofApp::exit() {
	// Release the sender on exit
	camsender.ReleaseSender();
}


//--------------------------------------------------------------
void ofApp::keyPressed(int key) {

	int i = key - 48; // Decimal number
	if (key == 32) {
		camdevices = vidGrabber.listDevices();
		printf("Select a webcam by it's index (0-%d)\n", (int)camdevices.size() - 1);
	}
	else if (i >= 0 && i < (int)camdevices.size()) {
		camindex = i;
		vidGrabber.close();
		vidGrabber.setDeviceID(camindex);
		vidGrabber.setDesiredFrameRate(30); // Try to set desired frame rate
		ofSetFrameRate(30); // Set Openframeworks to send frames at this rate
		camsender.SetFrameRate(30.0); // Set the NDI sender to this rate
		if (vidGrabber.setup(640, 480)) {  // Try to grab at desired size
			ofSetWindowShape(vidGrabber.getWidth(), vidGrabber.getHeight());
			// The webcam resolution might have changed. Update the sender.
			camsender.UpdateSender((unsigned int)vidGrabber.getWidth(), (unsigned int)vidGrabber.getHeight());
			cout << "Initialized webcam [" << camdevices[camindex].deviceName << "] (" << vidGrabber.getWidth() << " x " << vidGrabber.getHeight() << ")" << endl;
		}
		else {
			printf("Webcam setup error. Try a different one.\n");
		}
	}

}
