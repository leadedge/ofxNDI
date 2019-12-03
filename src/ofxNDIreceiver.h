/*
	NDI Receiver

	using the NDI SDK to receive frames from the network

	http://NDI.NewTek.com

	Copyright (C) 2016-2019 Lynn Jarvis.

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

	08.07.16 - Use ofxNDIreceive class

*/

#pragma once
#ifndef __ofxNDIreceiver__
#define __ofxNDIreceiver__

#include "ofMain.h"

#if defined(TARGET_WIN32)
    #include <windows.h>
    #include <intrin.h> // for _movsd
    #include <gl\GL.h>
    #include <mmsystem.h> // for timegettime if ofMain is included
    #pragma comment(lib, "Winmm.lib") // for timegettime
#elif defined(TARGET_OSX)
    #include <x86intrin.h> // for _movsd
    #include <sys/time.h>
#endif

#include <string>
#include <iostream>
#include <vector>
#include <iostream> // for cout
#include "Processing.NDI.Lib.h" // NDI SDK
#include "ofxNDIreceive.h" // Basic receiver functions
#include "ofxNDIutils.h" // buffer copy utilities

class ofxNDIreceiver {

public:

	ofxNDIreceiver();
	~ofxNDIreceiver();

	// Create a receiver
	// - index | index in the sender list to connect to
	//   -1 - connect to the selected sender
	//        if none selected connect to the first sender
	// Default NDI format is BGRA
	// Default application format is RGBA
	// Data is converted to RGBA during copy from the video frame buffer
	bool CreateReceiver(int index = -1);

	// Create a receiver with preferred colour format
	// - colorFormat | the preferred format
	// - index | index in the sender list to connect to
	//   -1 - connect to the selected sender
	//        if none selected connect to the first sender
	bool CreateReceiver(NDIlib_recv_color_format_e colorFormat, int index = -1);

	// Open the receiver to receive
	bool OpenReceiver();

	// Return whether a receiver has been created
	bool ReceiverCreated();

	// Return whether a receiver has connected to a sender
	bool ReceiverConnected();

	// Close receiver and release resources
	void ReleaseReceiver();

	// Receive a texture
	// - texture re-allocated for changed sender dimensions
	bool ReceiveImage(ofTexture &texture);

	// Receive an fbo
	// - fbo re-allocated for changed sender dimensions
	bool ReceiveImage(ofFbo &fbo);

	// Receive an image
	// - image re-allocated for changed sender dimensions
	bool ReceiveImage(ofImage &image);

	// Receive a pixel buffer
	// - buffer re-allocated for changed sender dimensions
	bool ReceiveImage(ofPixels &pixels);

	// Receive image pixels to a char buffer
	// - Calling application must test width and
	//   height for change with true return
	//   and realocate the receiving buffer.
	// - pixel | received pixel data
	// - width | received image width
	// - height | received image height
	// - bInvert | flip the image
	bool ReceiveImage(unsigned char *pixels,
		unsigned int &width, unsigned int &height,
		bool bInvert = false);

	// Create an NDI finder to find existing senders
	void CreateFinder();

	// Release an NDI finder that has been created
	void ReleaseFinder();

	// Find all current NDI senders
	int FindSenders();

	// Find all current senders and refresh sender list
	// If no timeout specified, return the sources that exist right now
	// For a timeout, wait for that timeout and return the sources that exist then
	// If that fails, return NULL
	int RefreshSenders(uint32_t timeout);

	// Set current sender index in the sender list
	bool SetSenderIndex(int index);

	// Current index in the sender list
	int GetSenderIndex();

	// The index of a sender name
	bool GetSenderIndex(char *sendername, int &index);

	// Name characters of a sender index
	bool GetSenderName(char *sendername);
	bool GetSenderName(char *sendername, int index);
	bool GetSenderName(char *sendername, int maxsize, int index);

	// Name string of a sender index
	std::string GetSenderName(int index = -1);

	// Sender width
	unsigned int GetSenderWidth();

	// Sender height
	unsigned int GetSenderHeight();

	// Sender frame rate
	float GetSenderFps();

	// Number of senders
	int GetSenderCount();

	// Has the user changed the sender index
	bool SenderSelected();

	// Set NDI low banwidth option
	void SetLowBandwidth(bool bLow = true);

	// Received frame type
	NDIlib_frame_type_e GetFrameType();

	// Is the current frame MetaData ?
	// Use when ReceiveImage fails
	bool IsMetadata();

	// The current MetaData string
	std::string GetMetadataString();

	// Is the current frame Audio data ?
	// Use when ReceiveImage fails
	bool IsAudioFrame();

	// Return current audio frame data
	void GetAudioData(float * &output, int &samplerate, int &samples, int &nChannels);

	// The NDI SDK version number
	std::string GetNDIversion();

	// Timed received frame rate
	double GetFps();

	// Basic receiver functions
	ofxNDIreceive NDIreceiver;

private :

	std::string SenderName;
	bool GetPixelData(ofTexture &texture);


};


#endif
