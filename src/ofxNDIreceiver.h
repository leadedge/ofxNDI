/*
	NDI Receiver

	using the NDI SDK to receive frames from the network

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

	16.10.16 - common buffer copy utilities

*/
#pragma once
#ifndef __ofxNDIreceiver__
#define __ofxNDIreceiver__

#include <windows.h>
#include <string>
#include <vector>
#include <intrin.h> // for _movsd
#include <emmintrin.h> // for SSE2
#include <gl\GL.h>
#include "Processing.NDI.Lib.h" // NDI SDK
#pragma comment(lib, "Winmm.lib") // for timegettime
#include "ofxNDIutils.h" // buffer copy utilities

class ofxNDIreceiver {

public:

	ofxNDIreceiver();
    ~ofxNDIreceiver();

	bool CreateReceiver(int index = -1);
	bool CreateReceiver(NDIlib_recv_color_format_e colorFormat, int index = -1);
	void ReleaseReceiver();
	bool ReceiveImage(unsigned char *pixels, 
					  unsigned int &width, unsigned int &height, 
					  bool bSwapRB = false, bool bInvert = false);

	void CreateFinder();
	void ReleaseFinder();
	int  FindSenders();
	int  RefreshSenders(DWORD dwTimeout);

	void SetSenderIndex(int index); // Set current sender index in the sender list
	int  GetSenderIndex(); // Get current index in the sender list
	bool GetSenderIndex(char *sendername, int &index); // Get the index of a sender name
	bool GetSenderName(char *sendername, int index = -1); // Get the name of a sender index
	int  GetSenderCount(); // Number of senders
	bool SenderSelected(); // Has the user changed the sender index

private:

	const NDIlib_source_t* p_sources;
	//DWORD no_sources; 
	uint32_t no_sources;
	NDIlib_send_create_t NDI_send_create_desc;
	NDIlib_find_instance_t pNDI_find;
	NDIlib_recv_instance_t pNDI_recv; 
	NDIlib_video_frame_t video_frame;

	unsigned int m_Width, m_Height;
	std::vector<std::string> NDIsenders; // List of sender names
	int nsenders;// Sender count
	int senderIndex; // Current sender index
	bool bNDIinitialized; // Is NDI initialized properly
	bool bSenderSelected; // Sender index has been changed by the user
	DWORD dwStartTime; // For timing delay
	DWORD dwElapsedTime;

};


#endif

