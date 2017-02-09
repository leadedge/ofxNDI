/*

	OpenFrameworks NDI receiver example

	using the NDI SDK to receive frames via network

	http://NDI.NewTek.com

	Copyright (C) 2016-2017 Lynn Jarvis.

	http://www.spout.zeal.co

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
*/
#pragma once

#include "ofMain.h"
#include "ofxNDI.h" // NDI classes

#include "ofxNDIdialog.h" // for the sender dialog
//
// Also if you want to use the sender selection dialog,
// include in your project from the ofxNDI addon source files :
//     ofxNDIdialog.h
//     ofxNDIdialog.cpp
//     resource.h
//     resource.rc 
// If this conflicts with existing resources, you will need to include
// the code for the dialog within your own resource files and change
// identifiers as necessary.
//

class ofApp : public ofBaseApp {

	public:

		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
	
		ofxNDIreceiver ndiReceiver; // NDI receiver object
		ofxNDIdialog ndiDialog;     // for the sender dialog

		ofImage ndiImage;           // Image used for pixel transfer and display
		char senderName[256];	    // Sender name used by a receiver
		int nSenders;
		unsigned int senderWidth;
		unsigned int senderHeight;
		bool bNDIreceiver;

		// For received frame fps calculations
		double startTime, lastTime, frameTime, frameRate, fps;



};
