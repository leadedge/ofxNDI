/*

	Openframeworks ofxNDI sender example

	NDI SDK https://ndi.video

	Copyright (C) 2026 Lynn Jarvis.
	https://github.com/leadedge/ofxNDI
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
	08.12.20 - Change from sprintf to std::string for on-screen display
	02.12,21 - Update pixel receive examples and comments
	04.12.21 - Use Setfromexternalpixels to update display image
	24.04.22 - Update examples for Visual Studio 2022
	04.07.22 - Update with revised ofxNDI. Rebuild x64/MD.
	05-08-22 - Update to NDI 5.5 (ofxNDI and bin\Processing.NDI.Lib.x64.dll)
	20-11-22 - Update to NDI 5.5.2 (ofxNDI and bin\Processing.NDI.Lib.x64.dll)
	27-04-23 - Update to NDI 5.5.4 (ofxNDI and bin\Processing.NDI.Lib.x64.dll)
			   Rebuild example executables x64/MD
	14-12-24 - Add #define BUILDRECEIVER in header for conditional build
	09.05.24 - Update to NDI 6.0.0
	17.05.24 - Update to NDI 6.0.1.0
	19.05.24 - ofxNDI async texture pixel load (LoadTexturePixels) for receiver
	20.05 24 - Add SetUpload to activate async pixel load
			   Extend comments for asynchronous sending
	27.05.24 - ofxNDIsender - SendImage
			     ofTexture - RGBA only
			   ofxNDIreceive - FindSenders
			     check for a name change at the same index and update m_senderName
			   ofxNDIsend - ReleaseSender
			     clear metadata, CreateSender - add sender name to metadata
			   Rebuild example sender/receiver x64/MD
	07.01.25 - Update to NDI 6.1.1.0
	10.01.25 - Sender - try a different sender name if initialization fails.
			   If it fails again, warn and quit.
	16.03.25 - SetUpload option default false for a receiver
			   SetReadback option default false for a sender
	18.03.25 - #ifdef for MessageBox Windows only
			   Console out OpenGL and Openframeworks versions
	11.04.25 - Use ofSystemAlertDialog in place of MessageBox (issue #60)
	21.07.26 - Update to NDI 6.2.0.3
	02.03.26 - Created separate sender and receiver examples
			   Updated ofxNDI with audio
			   Update to NDI 6.3.1

*/
#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

	ofBackground(0);
	ofSetColor(255);

	// Query the OpenGL version string
	const char * version = (const char *)glGetString(GL_VERSION);
	std::cout << "OpenGL (" << version << ")" << std::endl;
	// Get Openframemorks and renderer OpenGL version
	int major = ofGetVersionMajor();
	int minor = ofGetVersionMinor();
	int glmajor = ofGetGLRenderer()->getGLVersionMajor();
	int glminor = ofGetGLRenderer()->getGLVersionMinor();
	std::cout << "Openframeworks " << major << "." << minor << " - OpenGL " << glmajor << "." << glminor << std::endl;
	// Check for :
	// glGenBuffersARB,	glDeleteBuffersARB, glBindBufferARB
	// glBufferDataARB, glMapBufferARB, glUnmapBufferARB
	if (GLEW_ARB_vertex_buffer_object) {
		std::cout << "GL_ARB_vertex_buffer_object is supported\n" << std::endl;
	}
	else {
		std::cout << "GL_ARB_vertex_buffer_object is not supported\n" << std::endl;
	}

	senderName = "Openframeworks NDI Sender";
	ofSetWindowTitle(senderName); // show it on the title bar

#ifdef _WIN64
	std::cout << "\nofxNDI example sender - 64 bit" << std::endl;
#else // _WIN64
	std::cout << "\nofxNDI example sender - 32 bit" << std::endl;
#endif // _WIN64

	// Set the dimensions of the sender output here.
	// This is independent of the display window size.
	// It can be changed using the 'S' key.
	// 4K (3840x2160) can help assess performance of different options.
	senderWidth  = 1920;
	senderHeight = 1080;

	// Create an RGBA fbo for collection of data
	m_fbo.allocate(senderWidth, senderHeight, GL_RGBA);

	// Option : set readback
	// Pixel data extraction from fbo or texture
	// is optimised using two OpenGL pixel buffers (pbo's)
	// Note that the speed can vary with different CPUs
	// Default false
	// ndiSender.SetReadback(true);

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
	// ndiSender.SetFrameRate(30); // Enable this line for 30 fps instead of default 60.

	// Option : set NDI asynchronous sending
	// If disabled, the render rate is clocked to the sending framerate. 
	// Note that when sending is asynchronous, frames can be sent at a higher
	// rate than the receiver can process them and hesitations may be evident.
	// ndiSender.SetAsync(true);

	// Create a sender with RGBA output format
	bInitialized = ndiSender.CreateSender(senderName.c_str(), senderWidth, senderHeight);

	// A Sender with the same name cannot be created.
	// In case the executable has been renamed, and there
	// is already a sender of the same NDI name running,
	// increment the NDI name and try again.
	if (!bInitialized) {
		std::string str = "Could not create sender [";
		str += senderName;	str += "]";
		printf("Could not create %s\n", str.c_str());
		senderName += "_2";
		bInitialized = ndiSender.CreateSender(senderName.c_str(), senderWidth, senderHeight);
		// If that still fails warn the user and quit
		if (!bInitialized) {
			ofSystemAlertDialog(str);
			exit();
		}
	}
	printf("Created sender [%s]\n", senderName.c_str());

	// 
	// 3D drawing setup for the demo graphics
	glEnable(GL_DEPTH_TEST); // enable depth comparisons and update the depth buffer
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); // Really Nice Perspective Calculations
	ofDisableAlphaBlending(); // Or we can get trails with the rotating cube

	// ofDisableArbTex is needed to create a texture with
	// normalized coordinates for bind in DrawGraphics
	ofDisableArbTex();
	textureImage.load("SpoutBox.jpg");

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
	ndiImage.load("NDI_Box.png");

	// Make it the same size as the sender
	ndiImage.resize(senderWidth, senderHeight);

	// If Wait For Vertical Sync is applied by the driver,
	// frame rate will be limited to multiples of the sync interval.
	// Disable it here and use async NDI send for best performance.
	ofSetVerticalSync(false);

	// Limit frame rate using timing instead
	ofSetFrameRate(60);

}


//--------------------------------------------------------------
void ofApp::update() {

}

//--------------------------------------------------------------
void ofApp::draw() {

	// Check success of CreateSender
	if (!bInitialized)
		return;

	//
	// Option 1 : Send ofFbo
	//
	DrawGraphics();
	ndiSender.SendImage(m_fbo);

	//
	// Option 2 : Send ofTexture
	//
	// DrawGraphics();
	// ndiSender.SendImage(m_fbo.getTexture());

	//
	// Option 3 : Send ofImage
	//
	// ofImage is converted to RGBA if not already
	// ndiImage.draw(0, 0, ofGetWidth(), ofGetHeight());
	// ndiSender.SendImage(ndiImage);

	//
	// Option 4 : Send ofPixels
	//
	// GPU download from a texture is not necessary
	// because pixel data is already in CPU memory.
	// GPU Readback and YUV Format options are not used.
	// There is no change if these options are selected.
	// Performance may be optimised by Async sending.
	// ofPixels is converted to RGBA if not already
	// ndiImage.draw(0, 0, ofGetWidth(), ofGetHeight());
	// ndiSender.SendImage(ndiImage.getPixels());

	//
	// Option 5 : Send char buffer
	//
	// Pixels must be rgba
	// if (ndiImage.getImageType() != OF_IMAGE_COLOR_ALPHA)
		// ndiImage.setImageType(OF_IMAGE_COLOR_ALPHA);
	// ndiImage.draw(0, 0, ofGetWidth(), ofGetHeight());
	// ndiSender.SendImage(ndiImage.getPixels().getData(), senderWidth, senderHeight);

	//
	// Show what it's sending
	//
	ShowInfo();

}

void ofApp::ShowInfo() {

	std::string str;
	if (ndiSender.SenderCreated()) {
		
		str = "Sending as : ["; str += senderName; str += "] (";
		str += std::to_string(senderWidth); str += "x"; str += std::to_string(senderHeight); str += ")";
		ofDrawBitmapString(str, 20, 25);
		// Round first to avoid integer truncation
		str = "fps : "; str += std::to_string((int)roundf(ofGetFrameRate()));
		ofDrawBitmapString(str, ofGetWidth() - 140, 25);

		// Show sending options
		ofDrawBitmapString("Sending options", 20, 48);

		str = " NDI fps  (""F"") : ";
		framerate = ndiSender.GetFrameRate();
		str += std::to_string(framerate);
		// Limit fps display to 2 decimal places
		size_t s = str.rfind(".");
		str = str.substr(0, s + 3);
		ofDrawBitmapString(str, 20, 66);

		str = " Async    (""A"") : ";
		str += std::to_string((int)ndiSender.GetAsync());
		ofDrawBitmapString(str, 20, 82);

		str = " Readback (""P"") : ";
		str += std::to_string((int)ndiSender.GetReadback());
		ofDrawBitmapString(str, 20, 98);

		str = " Format (""Y""""/""""R"") : ";
		if (ndiSender.GetFormat() == NDIlib_FourCC_video_type_UYVY)
			str += "YUV ";
		else
			str += "RGBA ";
		ofDrawBitmapString(str, 20, 114);

		str = " Size     (""S"") : ";
		str += std::to_string((int)senderWidth);
		str += "x";
		str += std::to_string((int)senderHeight);
		ofDrawBitmapString(str, 20, 130);

		// NDI version
		str = "NDI version - " + ndiSender.GetNDIversion();
		ofDrawBitmapString(str, 20, ofGetHeight()-10);

	}

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {

	std::string str;
	framerate = ndiSender.GetFrameRate(); // update global fps value
	double fps = framerate; // for entry
	int width, height = 0; // for entry

	switch (key) {

		case 'f':
		case 'F':
		{
			str = std::to_string(framerate);
			size_t s = str.rfind(".");
			str = str.substr(0, s + 3);
			str = ofSystemTextBoxDialog("Frame rate", str);
			if (!str.empty()) {
				fps = stod(str);
				if (fps <= 60.0 && fps >= 10.0) {
					framerate = fps;
					ndiSender.SetFrameRate(fps);
				}
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
			str = ofSystemTextBoxDialog("Image size", std::to_string(senderWidth) + "x" + std::to_string(senderHeight));
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

	} // end switch key

	// Show the main window
	#ifdef TARGET_WIN32
	BringWindowToTop(ofGetWin32Window());
	#endif

}

void ofApp::DrawGraphics() {

	// Draw graphics into an fbo used for the examples
	m_fbo.begin();
	ofEnableDepthTest();
	ofClear(10, 100, 140, 255);
	ofPushMatrix();
	ofTranslate((float)senderWidth / 2.0, (float)senderHeight / 2.0, 0);
	ofRotateYDeg(rotX);
	ofRotateXDeg(rotY);
	textureImage.getTexture().bind();
	ofDrawBox(0.4 * (float)senderHeight);
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
