/*
	OpenFrameworks NDI sender example

	using the NDI SDK to send the frames via network

	http://NDI.NewTek.com

	Copyright (C) 2016-2024 Lynn Jarvis.

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
#include "ofxNDI.h" // ofxNDI classes

class ofApp : public ofBaseApp {
	public:

		void setup();
		void update();
		void draw();
		void exit();
		void keyPressed(int key);

		ofxNDIsender ndiSender;        // NDI sender
		std::string senderName;        // Sender name
		unsigned int senderWidth = 0;  // Width of the sender output
		unsigned int senderHeight = 0; // Height of the sender output
		bool bInitialized = false;
		ofFbo m_fbo;                   // Fbo used for graphics and sending
		ofImage textureImage;          // Texture image for the 3D cube graphics
		float rotX = 0.0f;
		float rotY = 0.0f;             // Cube rotation increment
		ofImage ndiImage;              // Test image for sending
		void DrawGraphics();           // Rotating cube draw
		
		bool bReadback = true;
		bool bAsync = true;
		double framerate = 60.0;

};
