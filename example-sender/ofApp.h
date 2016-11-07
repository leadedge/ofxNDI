/*
	OpenFrameworks NDI sender example

	using the NDI SDK to send the frames via network

	http://NDI.NewTek.com

	Copyright (C) 2016 Lynn Jarvis.

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

// BGRA definition should be in glew.h
// but define it here just in case it is not
#ifndef GL_BGRA_EXT
#define GL_BGRA_EXT 0x80E1
#endif


class ofApp : public ofBaseApp {
	public:

		void setup();
		void update();
		void draw();
		void exit();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y);
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

		ofxNDIsender ndiSender;    // NDI sender object
		char senderName[256];      // Sender name
		unsigned int senderWidth;  // Width of the sender output
		unsigned int senderHeight; // Height of the sender output
		ofFbo ndiFbo;              // Fbo used for data transfer
		ofPixels ndiBuffer[2];     // Two pixel buffers for async sending
		int idx;                   // Index used for async buffer swapping
		ofImage textureImage;      // Texture image for the 3D cube graphics
		float rotX, rotY;          // Cube rotation increment

		GLuint ndiPbo[2];
		int PboIndex;
		int NextPboIndex;
		bool bUsePBO;
		bool ReadFboPixels(ofFbo fbo, unsigned int width, unsigned int height, unsigned char *data);

};
