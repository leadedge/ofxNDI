/*
	OpenFrameworks NDI sender example

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
	08.12.20 - Change from sprintf to std::string for on-screen display
	03.01.22 - Revise for ofxNDI with YUV sending option

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

	cout << ndiSender.GetNDIversion() << " (https://www.ndi.tv/)" << endl;

	// Set the dimensions of the sender output here
	// This is independent of the display window size.
	// 4K is set as the starting resolution to help
	// assess performance with different options.
	// It can be changed using the 'S' key.
	senderWidth  = 3480;
	senderHeight = 2160;

	// Create an RGBA fbo for collection of data
	m_fbo.allocate(senderWidth, senderHeight, GL_RGBA);

	// Option : set readback
	// Pixel data extraction from fbo or texture
	// is optimised using two OpenGL pixel buffers (pbo's)
	// Note that the speed can vary with different CPUs
	ndiSender.SetReadback();

	// Option : set the framerate
	// NDI sending will clock at the set frame rate 
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
	//
	// Note that the NDI sender frame rate should match the render rate
	// so that it's displayed smoothly with NDI Studio Monitor.
	//
	// ndiSender.SetFrameRate(30); // Disable this line for default 60 fps.

	// Option : set NDI asynchronous sending
	// If disabled, the render rate is clocked to the sending framerate. 
	ndiSender.SetAsync();

	// Create a sender with RGBA output format
	ndiSender.CreateSender(senderName, senderWidth, senderHeight);

	// 3D drawing setup for the demo graphics
	glEnable(GL_DEPTH_TEST); // enable depth comparisons and update the depth buffer
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); // Really Nice Perspective Calculations
	ofDisableAlphaBlending(); // Or we can get trails with the rotating cube

	// ofDisableArbTex is needed to create a texture with
	// normalized coordinates for bind in DrawGraphics
	ofDisableArbTex(); 
	textureImage.load("NDI_Box.png");

	// Back to default pixel coordinates
	ofEnableArbTex();

	// Workaround for mirrored texture with ofDrawBox and ofBoxPrimitive for Openframeworks 10.
#if OF_VERSION_MINOR >= 10
	textureImage.mirror(false, true);
#endif

	// Cube rotation
	rotX = 0;
	rotY = 0;

	// Image for pixel sending examples
	// Loads as RGB but is converted to RGBA by sending functions
	ndiImage.load("Test_Pattern.jpg");

	// Make it the same size as the sender
	ndiImage.resize(senderWidth, senderHeight);

}

//--------------------------------------------------------------
void ofApp::update() {


}

//--------------------------------------------------------------
void ofApp::draw() {

	ofBackground(0);
	ofSetColor(255, 255, 255, 255);

	// Option 1 : Send ofFbo
	DrawGraphics();
	ndiSender.SendImage(m_fbo);

	// Option 2 : Send ofTexture
	// DrawGraphics();
	// ndiSender.SendImage(m_fbo.getTexture());

	// Option 3 Send ofImage
	// ofImage is converted to RGBA if not already
	// ndiImage.draw(0, 0, ofGetWidth(), ofGetHeight());
	// ndiSender.SendImage(ndiImage);

	//
	// Send pixels
	//
	// GPU download from a texture is not necessary
	// because pixel data is already in CPU memory.
	// GPU Readback and YUV Format options are not used.
	// There is no change if these options are selected.
	// Performance may be optimised by Async sending.
	//

	// Option 4 Send ofPixels
	// ofPixels is converted to RGBA if not already
	// ndiImage.draw(0, 0, ofGetWidth(), ofGetHeight());
	// ndiSender.SendImage(ndiImage.getPixels());

	// Option 5 Send char buffer
	// Pixels must be rgba
	// if (ndiImage.getImageType() != OF_IMAGE_COLOR_ALPHA)
	 	// ndiImage.setImageType(OF_IMAGE_COLOR_ALPHA);
	// ndiImage.draw(0, 0, ofGetWidth(), ofGetHeight());
	// ndiSender.SendImage(ndiImage.getPixels().getData(), senderWidth, senderHeight);
	
	//
	// Show what it's sending
	//

	if (ndiSender.SenderCreated()) {

		std::string str;
		str = "Sending as : ["; str += senderName; str += "] (";
		str += to_string(senderWidth); str += "x"; str += to_string(senderHeight); str += ")";
		ofDrawBitmapString(str, 20, 25);
		str = "fps : "; str += to_string((int)ofGetFrameRate());
		ofDrawBitmapString(str, ofGetWidth() - 140, 25);

		// Show sending options
		ofDrawBitmapString("Sending options", 20, 48);

		str = " NDI fps  (""F"") : ";
		framerate = ndiSender.GetFrameRate();
		char dstr[8];
		sprintf_s(dstr, 8, "%4.2f", framerate);
		str += dstr;
		ofDrawBitmapString(str, 20, 66);

		str = " Async    (""A"") : ";
		str += to_string((int)ndiSender.GetAsync());
		ofDrawBitmapString(str, 20, 82);

		str = " Readback (""P"") : ";
		str += to_string((int)ndiSender.GetReadback());
		ofDrawBitmapString(str, 20, 98);

		str = " Format (""Y""""/""""R"") : ";
		if (ndiSender.GetFormat() == NDIlib_FourCC_video_type_UYVY)
			str += "YUV ";
		else
			str += "RGBA ";
		ofDrawBitmapString(str, 20, 114);

		str = " Size     (""S"") : ";
		str += to_string((int)senderWidth);
		str += "x";
		str += to_string((int)senderHeight);
		ofDrawBitmapString(str, 20, 130);

	}

}

void ofApp::DrawGraphics() {

	// Draw graphics into an fbo used for the examples
	m_fbo.begin();
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
	m_fbo.end();

	// Rotate the cube
	rotX += 0.75;
	rotY += 0.75;

	// Draw the fbo result fitted to the display window
	m_fbo.draw(0, 0, ofGetWidth(), ofGetHeight());

}

//--------------------------------------------------------------
void ofApp::exit() {
	// The sender must be released 
	// or NDI sender discovery will still find it
	ndiSender.ReleaseSender();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

	std::string str;
	framerate = ndiSender.GetFrameRate(); // update global fps value
	char dstr[8]; // for display
	double fps = framerate; // for entry
	int width, height = 0; // for entry

	switch(key)
	{

	case 'f':
	case 'F':
		sprintf_s(dstr, 8, "%4.2f", framerate);
		str = ofSystemTextBoxDialog("Frame rate", dstr);
		if (!str.empty()) {
			fps = stod(str);
			if (fps <= 60.0 && fps >= 10.0) {
				framerate = fps;
				ndiSender.SetFrameRate(fps);
			}
		}
		break;

	case 'a':
	case 'A':
		bAsync = !bAsync;
		ndiSender.SetAsync(bAsync);
		break;

	case 'p':
	case 'P':
		bReadback = !bReadback;
		ndiSender.SetReadback(bReadback);
		break;

	case 'y':
	case 'Y':
		ndiSender.SetFormat(NDIlib_FourCC_video_type_UYVY);
		break;

	case 'r':
	case 'R':
		ndiSender.SetFormat(NDIlib_FourCC_video_type_RGBA);
		break;

	case 's':
	case 'S':
		str = ofSystemTextBoxDialog("Image size", to_string(senderWidth) + "x" + to_string(senderHeight));
		if (!str.empty()) {
			width = height = 0;
			if (!str.empty()) {
				std::size_t pos = str.find("x");
				std::string w = str.substr(0, pos);
				width = stoi(w);
				std::string h = str.substr(pos + 1, str.length());
				height = stoi(h);
				if (width > 99 && width < 4100 && height > 100 && height <= 4100) {
					std::string name = ndiSender.GetSenderName();
					senderWidth = (unsigned int)width;
					senderHeight = (unsigned int)height;
					// Resize the drawing fbo to the same size as the sender
					m_fbo.allocate(senderWidth, senderHeight, GL_RGBA);
					// And the demo image
					ndiImage.resize(senderWidth, senderHeight);
					// Adapt NDI sender to the size change
					if (ndiSender.SenderCreated())
						ndiSender.UpdateSender(senderWidth, senderHeight);
				}
			}
		}
		break;
	}

	// Show the main window
	BringWindowToTop(ofGetWin32Window());

}

