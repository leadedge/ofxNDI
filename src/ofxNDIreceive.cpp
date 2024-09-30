/*
	NDI Receive

	using the NDI SDK to receive frames from the network

	https://ndi.video

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

	20.06.16 - Added RefreshSenders for Max receiver
			 - Added GetSenderIndex(char *sendername, int &index)
	01.07.16 - Added UNREFERENCED_PARAMETER as required
	02.07.16 - Minor changes to RefreshSenders for NDIwebcam
	07.07.16 - Duplicate of global video_frame in ReceiveImage removed
			   Rebuild with VS2015
	25.07.16 - Timeout removed for NDIlib_recv_capture
	10.10.16 - Updated SSE2 memcpy with intrinsics for 64bit compatibility
			   (NDI applications require minimum SSE3)
			   Added "rgba_bgra_sse2" for rgba <> bgra conversion
			   and bSwapRB and bInvert options to RecieveImage and CopyImage
			   Removed ReceiveTexture - not working
	11.10.16 - Changed CreateReceiver
			     . use only an index rather than return a name as well
				 . do not change the current user selected index
			   Changed GetSenderName to use an optional sender index
	07.11.16 - Added CPU support check
	09.02.17 - include changes by Harvey Buchan for NDI SDK version 2
			 - Added Metadata
			 - Added option to specify low bandwidth NDI receiving mode
			 - Removed bSwapRB option from ReceiveImage - now done internally
			 - Replacement function for deprecated NDIlib_find_get_sources
	17.02.17 - Added GetNDIversion - NDIlib_version
	22.02.17 - cleanup
	31.03.18 - Update to NDI SDK Version 3 - search on "Vers 3"
	         - change functions to _v2
			 - change variable types
	11.06.18 - remove messageboxes and replace with cout
			 - correct strcpy_s in GetSenderName for string length
			   (see https://github.com/ThomasLengeling/ofxNDI/commit/216d8d90f811ba73c02bceed60d9deb3f09b02ef)
			 - change ReceiveImage to use the video frame pointer externally
			   rather than copy to a buffer
			   Added GetVideoData, FreeVideoData
	08.07.18 - Change GetSenderName to include size of char buffer
	         - Change class name to ofxDNIreceive
			 - Set allow_video_fields flag false for CreateReceiver
	12.07.18 - Add senderName
			 - Set sender name in CreateReceiver
			 - Update sender name list only if sender number changes
			 - Update sender index after find sources
	13.07.18 - Add 
				string GetSenderName
				GetSenderWidth
				GetSenderHeight
				GetFps
	16.07.18 - Add GetFrameType
			 - Use existing sender name for GetSenderName(-1)
	30.07.18 - const char for GetSenderIndex(const char *sendername, ..
			 - Added GetSenderIndex(std::string sendername, int &index)
	06.08.18 - SetSenderIndex return false for the same sender

	New functions and changes for 3.5 uodate:

				bool GetSenderName(char *sendername, int maxsize, int index = -1)
				bool ReceiveImage(unsigned int &width, unsigned int &height)
				NDIlib_FourCC_type_e GetVideoType()
				unsigned char *GetVideoData()
				void FreeVideoData()
				NDIlib_frame_type_e GetFrameType()
				std::string GetSenderName(int index = -1)
				unsigned int GetSenderWidth()
				unsigned int GetSenderHeight()
				double GetFps()

	11.08.18 - Add FindSenders overload to return true or false for network change
	30.08.18 - audio receive testing (for Receive image pixels to a buffer)
	24.03.19 - Add float GetSenderFps(), bool ReceiverConnected()
	05.11.18 - Update to NDI Version 4.0
			   Default Receiver is now BGRA
			   Conversion to RGBA during video frame buffer copy in ofxNDIreceiver
			   Add audio functions
	08.11.19 - Arch Linux x64 compatibility
			   https://github.com/hugoaboud/ofxNDI
			   To be tested
	15.11.19 - Change to dynamic load of NDI libraries
			   Add runtime download if load of library failed
	16.11.19 - Protect against loading the NDI dll again
			 - Initialize m_AudioData to nullptr and check null for access
	19.11.19 - Add conditional audio receive
	04.12.19 - Revise for ARM port
			   Check targets for Linux and OSX - not tested
			   Cleanup
	06.12.19 - Change all DWORD to uint32_t
	07.12.19 - Use Openframeworks platform target definitions in ofxNDIplatforms.h
	27.02.20 - Remove !defined(TARGET_OSX) condition for counter variables, functions, strcpy_s and TimeGetTime
	28.02.20 - Move NDIlib_frame_type_none, NDIlib_frame_type_error inside switch
			 - Add NDIlib_frame_type_status_change to switch
	29.02.20 - Move rounding from UpdateFps to GetFps
			 - Change from math floor to std::floor
			 - Change GetFps from double to int
	03.12.20 - Change NULL to nullptr for pointers
			 - Change back from from std::floor to math floor due to compatibility problems
	15.12.20 - Add more checks for p_NDILib
			 - Correct FindGetSources return false to nullptr if not initialized
	16.12.20 - Update to 4.5 : recv_capture_v3, NDIlib_audio_frame_v3_t, recv_free_audio_v3 
	18.12.20 - CreateReceiver : Attempt to prevent EXC_BAD_ACCESS when building for macOS Release
			   (https://github.com/leadedge/ofxNDI/issues/19)
			   by explicitly setting descriptor fields (result was no change)
			   Clean up for update from testing to master
	30.10.21 - Add SetSenderName
			   Modify CreateReceiver to use the set sender name
	31.10.21 - Add more comments to CreateReceiver
	02.12.21 - Intialize global video frame data pointer in constructor
			   Null the video frame data pointer in FreeVideoData()
			   Remove video data check in GetSenderFps
	03.12.21 - Fix audio_frame.p_data free in ReceiveImage
			   Free captured metadata buffer
	26.12.21 - Comment out unused usec_per_msec constant - PR #28 by dimitre
	28.12.21 - Add default case break for NDI_frame_type in ReceiveImage - PR #31 by dimitre
	04.01.22 - Add default case break for video_frame.FourCC in ReceiveImage
	09.03.22 - SetSenderName/SetSenderIndex - release the current sender
	23.04.22 - GetSenderName - return "" instead of nullptr to avoid Visual Studio warning C6387
			   Use size_t cast for malloc to avoid warning C26451: Arithmetic overflow
	30.04.22 - Add GetAudioChannels, GetAudioSamples, GetAudioSampleRate.
			   Add GetAudioData overload to get audio frame data pointer
	14.10.22 - Correct SetSenderNam
	14.12.23 - Add video frame timecode and video frame interval
			 - Add GetVideoTimecode, GetVideoTimestamp, GetVideoFrametime
	16.12.23 - Remove VideoFrametime until further investigation
	11.05.24 - Add GetSenderList, SetFormat and m_Format
			   Add "m_" prefix to globals senderName and senderIndex
	13.05.24 - Remove SenderSelected() and bSenderSelected
			   Add OpenReceiver() and use for both ReceiveImage functions
			   Change global "nsenders" to "m_nSenders"
			   FindSenders :
			      Use "sendercount" instead of "nsenders" to avoid conflict
			      Update "m_Senders"
	15.05.24 - ReceiveImage - add specific cases for unsupported formats to avoid warning
	20.05.24 - UpdateFps - change from damping to the average fps over 200 frames
	27.05.24 - FindSenders - check for a name change at the same index and update m_senderName
	29.05.24 - Return to rolling average for received fps calculation with 0.02 update
			   Add ResetFps to reset starting received frame rate
	01.06.24 - UpdateFps - rolling average damping based on received frame time

*/

#include "ofxNDIreceive.h"
#include <math.h>
// Linux
// https://github.com/hugoaboud/ofxNDI
#if !defined(TARGET_WIN32)
double timeGetTime() {
	struct timeval now;
	gettimeofday(&now, NULL);
	return now.tv_usec / 1000;
}

// Helpful conversion constants.
static const unsigned usec_per_sec = 1000000;
// static const unsigned usec_per_msec = 1000;

// These functions are written to match the win32
// signatures and behavior as closely as possible.
bool QueryPerformanceFrequency(LARGE_INTEGER *frequency)
{
	// Sanity check.
	assert(frequency != NULL);
	// gettimeofday reports to microsecond accuracy.
	frequency->QuadPart = usec_per_sec;
	return true;
}

bool QueryPerformanceCounter(LARGE_INTEGER *performance_count)
{
	struct timeval time;

	// Sanity check.
	assert(performance_count != NULL);

	// Grab the current time.
	gettimeofday(&time, NULL);
	performance_count->QuadPart = time.tv_usec + /* Microseconds. */
		time.tv_sec * usec_per_sec; /* Seconds. */

	return true;
}
#endif

ofxNDIreceive::ofxNDIreceive()
{
	p_NDILib = nullptr;
	pNDI_find = nullptr;
	pNDI_recv = nullptr;
	p_sources = nullptr;
	no_sources = 0;
	bNDIinitialized = false;
	bReceiverCreated = false;
	m_FrameType = NDIlib_frame_type_none;
	m_nSenders = 0;
	m_Width = 0;
	m_Height = 0;
	m_Format = NDIlib_recv_color_format_BGRX_BGRA;
	m_senderIndex = 0;
	m_senderName = "";
	// Audio
	m_AudioData = nullptr;
	m_bAudio = false;
	m_bAudioFrame = false;
	m_nAudioSampleRate = 0;
	m_nAudioSamples = 0;
	m_nAudioChannels = 0;

	// Intialize global video frame data pointer
	video_frame.p_data = nullptr;

	// Initialize video frame timecode and timestamp
	m_VideoTimecode = 0LL;
	m_VideoTimestamp = 0LL;

	// For received frame fps calculations
	startTime = lastTime = (double)timeGetTime();
	m_fps = 30.0; // starting value
	m_frameTimeTotal = 0.0; // averaging
	m_frameTimeNumber = 0.0;

	// NDI documentation :
	// For most uses you should specify NDIlib_recv_bandwidth_highest, which will
	// result in the same stream that is being sent from the up-stream source to you.
	// You may specify NDIlib_recv_bandwidth_lowest, which will provide you with a
	// medium quality stream that takes significantly reduced bandwidth
	// (see SetLowBandwidth)
	m_bandWidth = NDIlib_recv_bandwidth_highest;

	// Find and load the NDI dll
	p_NDILib = libloader.Load();
	if (p_NDILib)
		bNDIinitialized = true;

}


ofxNDIreceive::~ofxNDIreceive()
{
	FreeAudioData();
	if(p_NDILib && pNDI_recv) p_NDILib->recv_destroy(pNDI_recv);
	if(p_NDILib && pNDI_find) p_NDILib->find_destroy(pNDI_find);
	// Library is released in ofxNDIdynloader
}


// Create a finder to look for a sources on the network
void ofxNDIreceive::CreateFinder()
{
	if(!bNDIinitialized) return;

	if (pNDI_find) p_NDILib->find_destroy(pNDI_find);
	const NDIlib_find_create_t NDI_find_create_desc = { true, NULL, NULL }; // Version 2
	pNDI_find = p_NDILib->find_create_v2(&NDI_find_create_desc);
	p_sources = nullptr;
	no_sources = 0;
	m_nSenders = 0;
}

// Release the current finder
void ofxNDIreceive::ReleaseFinder()
{
	if(!bNDIinitialized) return;

	if (pNDI_find) p_NDILib->find_destroy(pNDI_find);
	pNDI_find = nullptr;
	p_sources = nullptr;
	no_sources = 0;

}

// Find all current NDI senders
// Return number of senders
// Replacement for original function
int ofxNDIreceive::FindSenders()
{
	int sendercount = 0;
	FindSenders(sendercount);
	return (int)NDIsenders.size();
}

// Find all current NDI senders
// nSenders - number of senders
// Return true for network change
bool ofxNDIreceive::FindSenders(int &sendercount)
{
	std::string name;
	uint32_t nsources = 0; // New number of sources

	if (!bNDIinitialized) {
		sendercount = 0;
		m_nSenders = 0;
		return false;
	}

	// If a finder was created, use it to find senders on the network
	if (pNDI_find) {

		//
		// This may be called for every frame so has to be fast.
		//

		// Specify a delay so that p_sources is returned only for a network change.
		// If there was no network change, p_sources is NULL and no_sources = 0 
		// and can't be used for other functions, so the sender names as well as 
		// the sender count need to be saved locally.
		p_sources = FindGetSources(pNDI_find, &nsources, 1);
		// Check for a name change at the same index and update m_senderName
		if (p_sources && nsources > 0 && !m_senderName.empty() && p_sources[m_senderIndex].p_ndi_name) {
			if (strcmp(m_senderName.c_str(), p_sources[m_senderIndex].p_ndi_name) != 0) {
				m_senderName = p_sources[m_senderIndex].p_ndi_name;
			}
		}

		// If there are new sources and the number of sources has changed
		if (p_sources && nsources > 0 && nsources != no_sources) {
			// Rebuild the sender name list
			no_sources = nsources;
			NDIsenders.clear();
			if (no_sources > 0) {
				for (int i = 0; i<(int)no_sources; i++) {
					if (p_sources[i].p_ndi_name && p_sources[i].p_ndi_name[0]) {
						NDIsenders.push_back(p_sources[i].p_ndi_name);
					}
				}
			}

			// Update the current sender index
			// because it's position may have changed
			if (!m_senderName.empty()) {

				// If there are no senders left, close the current receiver
				if (NDIsenders.size() == 0) {
					ReleaseReceiver();
					m_senderName.clear();
					m_senderIndex = 0;
					m_nSenders = 0;
					sendercount = 0;
					return true; // return true because the last one just closed
				}

				// Reset the current sender index for a changed name
				if (NDIsenders.size() > 0) {
					m_senderIndex = 0;
					for (unsigned int i = 0; i < (int)NDIsenders.size(); i++) {
						if (m_senderName == NDIsenders.at(i)) {
							m_senderIndex = i;
						}
					}
				}
			}

			// Network change - return new number of senders
			sendercount = (int)NDIsenders.size();
			m_nSenders = sendercount;

			return true;

		}
	}
	else {
		CreateFinder();
	}

	// Always update the number of senders even for no network change
	sendercount = (int)NDIsenders.size();
	m_nSenders = sendercount;

	return false; // no network change

}

// Refresh NDI sender list with the current network snapshot
// No longer used
int ofxNDIreceive::RefreshSenders(uint32_t timeout)
{
	std::string name;
	uint32_t nsources = 0;

	if(!bNDIinitialized) return 0;

	// Release the current finder
	if(pNDI_find) ReleaseFinder();
	if(!pNDI_find) CreateFinder();

	// If a finder was created, use it to find senders on the network
	// Give it a timeout in case of connection trouble.
	if(pNDI_find) {

		dwStartTime = timeGetTime();
		dwElapsedTime = 0;
		do {
			p_sources = p_NDILib->find_get_current_sources(pNDI_find, &nsources);
			dwElapsedTime = timeGetTime() - dwStartTime;
		} while(nsources == 0 && (uint32_t)dwElapsedTime < timeout);
		return nsources;

	}

	return 0;
}

// Set current sender index in the sender list
bool ofxNDIreceive::SetSenderIndex(int index)
{
	if(!bNDIinitialized) 
		return false;

	if (NDIsenders.empty() || NDIsenders.size() == 0)
		return false;

	m_senderIndex = index;
	if (m_senderIndex > (int)NDIsenders.size())
		m_senderIndex = 0;

	// Return for the same sender
	if (NDIsenders.at(m_senderIndex) == m_senderName)
		return false;

	// Different sender so release the current one
	ReleaseReceiver();

	// Update the class sender name
	m_senderName = NDIsenders.at(m_senderIndex);

	return true;

}

// Return the index of the current sender
int ofxNDIreceive::GetSenderIndex()
{
	return m_senderIndex;
}

// Get the index of a sender name
bool ofxNDIreceive::GetSenderIndex(const char *sendername, int &index)
{
	std::string name = sendername;
	return GetSenderIndex(name, index);
}

// Return the number of senders
int ofxNDIreceive::GetSenderCount()
{
	return (int)NDIsenders.size();
}

// Return the list of senders
std::vector<std::string> ofxNDIreceive::GetSenderList()
{
	return NDIsenders;
}


// Return the name characters of a sender index
// For back-compatibility only
// Char functions replaced with string versions
bool ofxNDIreceive::GetSenderName(char *sendername)
{
	// Length of user name string is not known
	// assume 128 characters maximum
	int idx = -1;
	int index = GetSenderIndex(sendername, idx);
	return GetSenderName(sendername, 128, index);
}

bool ofxNDIreceive::GetSenderName(char *sendername, int index)
{
	// Length of user name string is not known
	// assume 128 characters maximum
	return GetSenderName(sendername, 128, index);
}

bool ofxNDIreceive::GetSenderName(char *sendername, int maxsize, int userindex)
{
	int index = userindex;

	if (index > (int)NDIsenders.size() - 1)
		return false;

	// If no index has been specified, use the currently selected index
	if (userindex < 0) {
		// If there is an existing name, return it
		if (!m_senderName.empty()) {
#if !defined(TARGET_WIN32)
			strcpy(sendername, m_senderName.c_str());
#else
			strcpy_s(sendername, maxsize, m_senderName.c_str());
#endif
			return true;
		}
		// Otherwise use the existing index
		index = m_senderIndex;
	}

	if (NDIsenders.size() > 0
		&& (unsigned int)index < NDIsenders.size()
		&& !NDIsenders.empty()
		&& NDIsenders.at(index).size() > 0) {
#if !defined(TARGET_WIN32)
		strcpy(sendername, NDIsenders.at(index).c_str());
#else
		strcpy_s(sendername, maxsize, NDIsenders.at(index).c_str());
#endif
		return true;
	}

	return false;
}

// Get the index of a sender name string
bool ofxNDIreceive::GetSenderIndex(std::string sendername, int &index)
{
	if (sendername.empty()) return false;

	if (NDIsenders.size() > 0) {
		for (int i = 0; i<(int)NDIsenders.size(); i++) {
			if (sendername == NDIsenders.at(i)) {
				index = i;
				return true;
			}
		}
	}
	return false;
}

// Set a sender name to receive from
void ofxNDIreceive::SetSenderName(std::string sendername)
{
	// Return if the same as the current sender
	if (sendername == m_senderName)
		return;

	// Release the current sender and change the name
	ReleaseReceiver();
	m_senderName = sendername;

}

// Get the name string of a sender index
std::string ofxNDIreceive::GetSenderName(int userindex)
{
	int index = userindex;

	if (index > (int)NDIsenders.size() - 1)
		return m_senderName;

	// If no index has been specified, use the currently selected index
	if (userindex < 0) {
		// If there is an existing name, return it
		if (!m_senderName.empty()) {
			return m_senderName;
		}
		// Otherwise use the existing index
		index = m_senderIndex;
	}

	if (NDIsenders.size() > 0
		&& (unsigned int)index < NDIsenders.size()
		&& !NDIsenders.empty()
		&& NDIsenders.at(index).size() > 0) {
		return NDIsenders.at(index);
	}

	return ""; // nullptr generates Visual Studio warning C6387
}

// Return current sender width
unsigned int ofxNDIreceive::GetSenderWidth()
{
	return m_Width;
}

// Return current sender height
unsigned int ofxNDIreceive::GetSenderHeight()
{
	return m_Height;
}

// Return current sender fps
float ofxNDIreceive::GetSenderFps()
{
	float senderfps = 0.0f;
	// Retrieve the current sender fps
	if (video_frame.frame_rate_D > 0)
		senderfps = (float)video_frame.frame_rate_N / (float)video_frame.frame_rate_D;
	return senderfps;
}

//
// Bandwidth
//
// NDIlib_recv_bandwidth_lowest will provide a medium quality stream that takes almost no bandwidth,
// this is normally of about 640 pixels in size on it is longest side and is a progressive video stream.
// NDIlib_recv_bandwidth_highest will result in the same stream that is being sent from the up-stream source
//
void ofxNDIreceive::SetLowBandwidth(bool bLow)
{
	if(bLow)
		m_bandWidth = NDIlib_recv_bandwidth_lowest; // Low bandwidth receive option
	else
		m_bandWidth = NDIlib_recv_bandwidth_highest;

}

// Set preferred format
void ofxNDIreceive::SetFormat(NDIlib_recv_color_format_e format)
{
	m_Format = format;
}


// Return the received frame type
NDIlib_frame_type_e ofxNDIreceive::GetFrameType()
{
	return m_FrameType;
}

// Is the current frame MetaData ?
bool ofxNDIreceive::IsMetadata()
{
	return m_bMetadata;
}

// Return the current MetaData string
std::string ofxNDIreceive::GetMetadataString()
{
	return m_metadataString;
}

// Return the current video frame timestamp
int64_t ofxNDIreceive::GetVideoTimestamp()
{
	return m_VideoTimestamp;
}

// Return the current video frame timecode
// UTC time since the Unix Epoch (1/1/1970 00:00) with 100 ns precision.
int64_t ofxNDIreceive::GetVideoTimecode()
{
	return m_VideoTimecode;
}

// Set to receive Audio
void ofxNDIreceive::SetAudio(bool bAudio)
{
	m_bAudio = bAudio;
	if (!m_bAudio)
		FreeAudioData();
}

// Is the current frame Audio ?
bool ofxNDIreceive::IsAudioFrame()
{
	return m_bAudioFrame;
}

// Number of audio channels
int ofxNDIreceive::GetAudioChannels()
{
	return m_nAudioChannels;
}

// Number of audio samples
int ofxNDIreceive::GetAudioSamples()
{
	return m_nAudioSamples;
}

// Audio sample rate
int ofxNDIreceive::GetAudioSampleRate()
{
	if (m_AudioData) {
		return m_nAudioSampleRate;
	}
	return 0;
}

// Get audio frame data pointer
float * ofxNDIreceive::GetAudioData()
{
	return m_AudioData;
}

// Return audio frame data
// output - the audio data pointer and parameters
void ofxNDIreceive::GetAudioData(float*& output, int& samplerate, int& samples, int& nChannels)
{
	if (m_AudioData) {
		output = m_AudioData;
		samplerate = m_nAudioSampleRate;
		samples = m_nAudioSamples;
		nChannels = m_nAudioChannels;
	}
}

// Test for network change
// Create receiver if not initialized or a new sender has been selected
bool ofxNDIreceive::OpenReceiver()
{
	// Update the NDI sender list to find new senders
	// There is no delay if no new senders are found
	int sendercount = FindSenders();

	// Check the sender count
	if(sendercount > 0) {

		// Return now if receiver already created
		if (ReceiverCreated()) {
			return true;
		}

		// Create a new receiver if not already
		// A receiver is created from an index into a list of sender names.
		// The current user selected index is saved in the NDIreceiver class
		// and is used to create the receiver. The index is changed by 
		// SetSenderIndex or SetSenderName.
		// The receiver is created with default preferred format BGRA
		return CreateReceiver();
	}

	// No senders
	return false;

}


// Create a receiver
// If the NDI format is BGRA and the application format is RGBA,
// data is converted from BGRA to RGBA during copy from the video frame buffer.
bool ofxNDIreceive::CreateReceiver(int userindex)
{
	// NDI 4.0 > VERSION 4.1.3
	// NDI bug to be resolved.
	// 64 bit required for RGBA preferred
	// 32 bit returns BGRA even if RGBA preferred.
	// If you find the received result is BGRA, set the receiver to prefer BGRA
	// TODO - Set format function
	return CreateReceiver(m_Format, userindex);

/*
#if defined(_WIN64)
	return CreateReceiver(NDIlib_recv_color_format_RGBX_RGBA, userindex);
#else
	return CreateReceiver(NDIlib_recv_color_format_BGRX_BGRA, userindex);
#endif
*/

}

// Create a receiver with preferred colour format
bool ofxNDIreceive::CreateReceiver(NDIlib_recv_color_format_e colorFormat , int userindex)
{
	std::string name;

	if (!bNDIinitialized) {
		return false;
	}

	// The index passed in can be the required sender which has been
	// selected by the user from a list of senders currently running
	// or -1 if no index has been selected.
	int index = userindex;

	if (!pNDI_recv) {

		// The continued check in FindSenders is for a network change and
		// p_sources is returned NULL if no change. So we need to find all
		// the sources again to get a pointer to the selected sender.
		// Give it a timeout in case of connection trouble.
		if (pNDI_find) {
			dwStartTime = (unsigned int)timeGetTime();
			do {
				p_sources = p_NDILib->find_get_current_sources(pNDI_find, &no_sources);
				dwElapsedTime = (unsigned int)timeGetTime() - dwStartTime;
			} while (no_sources == 0 && dwElapsedTime < 4000);
		}

		if (p_sources && no_sources > 0) {
			// Quit if the user selected index is greater than the number of sources
			// Unlikely but safety.
			if (userindex > (int)no_sources - 1) {
				return false;
			}

			// If no index has been specified (-1), use the currently set index
			// "senderIndex" is either initialized as "0", the first sender,
			// or it is still set to the previous sender index.
			if (userindex < 0)
				index = m_senderIndex;

			// Rebuild the name list
			NDIsenders.clear();
			if (no_sources > 0) {
				for (int i = 0; i<(int)no_sources; i++) {
					if (p_sources[i].p_ndi_name && p_sources[i].p_ndi_name[0]) {
						NDIsenders.push_back(p_sources[i].p_ndi_name);
					}
				}
			}

			// Release the receiver if not done already
			if (bReceiverCreated) {
				ReleaseReceiver();
			}

			//
			// Check for a user requested sender set by SetSenderName().
			// (the global string "m_senderName" is not cleared by ReleaseReceiver above).
			//
			// Only applies for inital sender connection when the program starts.
			//
			// This check is normally skipped when the program starts, because the name
			// is intially empty and the first sender in the list is selected (index 0).
			//
			// But if a sender has been specified for intial connection, the check is performed.
			//
			//  o If the sender is running at index zero, the check immediately passes.
			//  o If the sender is running but with a different index, that index is retrieved.
			//  o If the sender is not in the name list, the check returns to wait for it.
			//
			// After the required sender is connected, operation is as usual.
			// That is, if the connected sender closes, the program waits for it to open again.
			// This allows for network outages.
			//
			// If the user picks a sender from the list, the name is derived from the selected index.
			//
			if (!m_senderName.empty()) {
				// Check the name against the current index
				if (m_senderName != NDIsenders.at(index)) {
					// If the name is different, does the sender exist ?
					if (!GetSenderIndex(m_senderName, index)) {
						// A sender name name has been specified so wait for it
						return false;
					}
					// If the required sender is running and it's index is now matched.
				}
			}

			// Set the descriptor for the source of the receiving index.
			// We tell it that we prefer the passed format (NDIlib_recv_color_format_e)
			// and bandwidth setting (Changed by SetLowBandwidth, default NDIlib_recv_bandwidth_highest)
			// Do not allow video fields.
			// Vers 3.5
			NDIlib_recv_create_v3_t NDI_recv_create_desc;
			NDI_recv_create_desc.source_to_connect_to = p_sources[index];
			NDI_recv_create_desc.color_format = colorFormat;
			NDI_recv_create_desc.bandwidth = m_bandWidth;
			NDI_recv_create_desc.allow_video_fields = false;
			NDI_recv_create_desc.p_ndi_recv_name = NULL;

			// Create the receiver
			// Deprecated version sets bandwidth to highest and allow fields to true.
			// pNDI_recv = NDIlib_recv_create2(&NDI_recv_create_desc);
			// Vers 3
			// pNDI_recv = NDIlib_recv_create_v2(&NDI_recv_create_desc);
			// Vers 3.5
			// pNDI_recv = NDIlib_recv_create_v3(&NDI_recv_create_desc);
			// Vers 4.0
			pNDI_recv = p_NDILib->recv_create_v3(&NDI_recv_create_desc);
			if (!pNDI_recv) {
				printf("CreateReceiver !pNDI_recv\n");
				return false;
			}

			// Reset the current sender name given the index
			m_senderName = NDIsenders.at(index);

			// Reset the current index value
			m_senderIndex = index;

			// Reset the timestamp, timecode and frame time
			m_VideoTimestamp = 0LL;
			m_VideoTimecode = 0LL;

			// Start the counter for frame fps calculations
			StartCounter();

			// on_program = true, on_preview = false
			const NDIlib_tally_t tally_state = { true, false };
			p_NDILib->recv_set_tally(pNDI_recv, &tally_state);

			// Set class flag that a receiver has been created
			bReceiverCreated = true;

			return true;

		}
	} // end create receiver

	return false;
}

// Return whether the receiver has been created
bool ofxNDIreceive::ReceiverCreated()
{
	return bReceiverCreated;
}

// Return whether a receiver has connected to a sender
bool ofxNDIreceive::ReceiverConnected()
{
	return bReceiverConnected;
}

// Close receiver and release resources
void ofxNDIreceive::ReleaseReceiver()
{
	if(!bNDIinitialized) return;

	if(pNDI_recv) 
		p_NDILib->recv_destroy(pNDI_recv);

	m_Width = 0;
	m_Height = 0;
	m_VideoTimestamp = 0LL;
	m_VideoTimecode = 0LL;

	pNDI_recv = nullptr;
	bReceiverCreated = false;
	bReceiverConnected = false;
	FreeVideoData();
	FreeAudioData();

	// Don't clear the sender name
	// It can be used with SetSenderName 
	// to specify as sender to receive from

}

//
// Receive RGBA image pixels to a buffer
//
bool ofxNDIreceive::ReceiveImage(unsigned char *pixels,
	unsigned int &width, unsigned int &height, bool bInvert)
{
	NDIlib_frame_type_e NDI_frame_type;
	NDIlib_metadata_frame_t metadata_frame;
	// NDIlib_audio_frame_v2_t audio_frame;
	// 4.5
	NDIlib_audio_frame_v3_t audio_frame;
	m_FrameType = NDIlib_frame_type_none;
	bool bRet = false;

	if (!bNDIinitialized) return false;

	// Create receiver if not initialized
	// or a new sender has been selected
	if (!OpenReceiver())
		return false;

	if (pNDI_recv) {

		// NDI_frame_type = p_NDILib->recv_capture_v2(pNDI_recv, &video_frame, &audio_frame, &metadata_frame, 0);
		// Vers 4.5
		NDI_frame_type = p_NDILib->recv_capture_v3(pNDI_recv, &video_frame, &audio_frame, &metadata_frame, 0);

		// Set frame type for external access
		m_FrameType = NDI_frame_type;

		// Clear existing metadata if any
		if (!m_metadataString.empty())
			m_metadataString.clear();
		m_bMetadata = false;

		// Default is a video frame
		// Retain any audio data that has been received
		m_bAudioFrame = false;

		switch (NDI_frame_type) {

			// No data received or the connection lost
			case NDIlib_frame_type_none:
				bRet = false;
				break;

			case NDIlib_frame_type_error:
				bRet = false;
				break;

			// The settings on this input have changed
			case NDIlib_frame_type_status_change:
				bRet = false;
				break;

			case NDIlib_frame_type_metadata:
				if (metadata_frame.p_data) {
					m_bMetadata = true;
					m_metadataString = metadata_frame.p_data;
					// ReceiveImage will return false
					// Use IsMetadata() to determine whether metadata has been received
					// Free the captured buffer
					p_NDILib->recv_free_metadata(pNDI_recv, &metadata_frame);
				}
				break;

			case NDIlib_frame_type_audio:
				if (audio_frame.p_data) {
					if (m_bAudio) {
						// printf("Audio data received (%d samples).\n", audio_frame.no_samples);
						// Copy the audio data to a local audio buffer
						// Allocate only for sample size change
						if (m_nAudioSamples != audio_frame.no_samples
							|| m_nAudioSampleRate != audio_frame.sample_rate
							|| m_nAudioChannels != audio_frame.no_channels) {
							// printf("Creating audio buffer - %d samples, %d channels\n", audio_frame.no_samples, audio_frame.no_channels);
							if (m_AudioData) free((void *)m_AudioData);
							m_AudioData = (float *)malloc((size_t)audio_frame.no_samples * (size_t)audio_frame.no_channels * sizeof(float));
						}
						// printf("Audio data received data = %x, samples = %d\n", (unsigned int)audio_frame.p_data, audio_frame.no_samples);
						m_nAudioChannels = audio_frame.no_channels;
						m_nAudioSamples = audio_frame.no_samples;
						m_nAudioSampleRate = audio_frame.sample_rate;
						if (m_AudioData)
							memcpy((void *)m_AudioData, (void *)audio_frame.p_data, ((size_t)m_nAudioSamples * (size_t)audio_frame.no_channels * sizeof(float)));
						m_bAudioFrame = true;
						// ReceiveImage will return false
						// Use IsAudioFrame() to determine whether audio has been received
						// and GetAudioData to retrieve the sample buffer
					}
					// Vers 4.5
					p_NDILib->recv_free_audio_v3(pNDI_recv, &audio_frame);
				}
				break;

			case NDIlib_frame_type_video:

				if (video_frame.p_data) {

					// The caller can check whether a frame has been received
					bReceiverConnected = true;

					if (m_Width != (unsigned int)video_frame.xres || m_Height != (unsigned int)video_frame.yres) {
						m_Width = (unsigned int)video_frame.xres; // current width
						m_Height = (unsigned int)video_frame.yres; // current height
						// Update the caller dimensions
						width = m_Width;
						height = m_Height;
						// Return received OK for the app to handle changed dimensions
						bRet = true;
					}

					// Otherwise sizes are current - copy the received frame data to the local buffer
					else if (video_frame.p_data && (uint8_t*)pixels) {

						// Video frame type
						switch (video_frame.FourCC) {
							// Note :
							// If the receiver is set up to prefer BGRA or RGBA format,
							// other formats are converted to by the API, and the
							// slower YUV422_to_RGBA conversion function is not used.
							case NDIlib_FourCC_type_UYVY: // YCbCr color space
							// Alpha component of NDIlib_FourCC_type_UYVA not supported
							case NDIlib_FourCC_type_UYVA: // With alpha (not used)
								// CPU conversion
								ofxNDIutils::YUV422_to_RGBA((const unsigned char *)video_frame.p_data, pixels, m_Width, m_Height, (unsigned int)video_frame.line_stride_in_bytes);
								break;
							case NDIlib_FourCC_video_type_P216:	break;
							case NDIlib_FourCC_video_type_PA16:	break;
							case NDIlib_FourCC_type_RGBA: // RGBA
							case NDIlib_FourCC_type_RGBX: // RGBX
								// Do not swap red/green
								ofxNDIutils::CopyImage((const unsigned char *)video_frame.p_data, pixels, m_Width, m_Height, (unsigned int)video_frame.line_stride_in_bytes, false, bInvert);
								break;
							case NDIlib_FourCC_type_BGRA: // BGRA
							case NDIlib_FourCC_type_BGRX: // BGRX
								// Swap red/green : BGRA > RGBA
								ofxNDIutils::CopyImage((const unsigned char *)video_frame.p_data, pixels, m_Width, m_Height, (unsigned int)video_frame.line_stride_in_bytes, true, bInvert);
								break;
							
							// Unsupported formats
							case NDIlib_FourCC_type_NV12:
							case NDIlib_FourCC_type_I420:
							case NDIlib_FourCC_type_YV12:
							case NDIlib_frame_type_max:
							default:
								break;

						} // end switch received format

						// Get the current video frame timecode
						// UTC time since the Unix Epoch (1/1/1970 00:00) with 100 ns precision.
						m_VideoTimecode = video_frame.timecode;

						// Get the current video frame timestamp
						m_VideoTimestamp = video_frame.timestamp;

						// Buffers captured must be freed
						p_NDILib->recv_free_video_v2(pNDI_recv, &video_frame);

						// The caller always checks the received dimensions
						width = m_Width;
						height = m_Height;

						// Update received frame counter
						UpdateFps();

						// return true for successful video frame received
						bRet = true;

					} // endif video frame copy
				} // end video frame type
				break;

			case NDIlib_frame_type_max:	// Not used
			default :
				break;

		} // end switch on received frame type

	} // endif pNDI_recv
	else {
		// No video data - no sender
		bReceiverConnected = false;
	}

	return bRet;

}

// Receive image pixels without a receiving buffer
// The received video frame is then held in ofxReceive class.
// (Used for receiving Openframeworks ofTexture, ofFbo, ofImage and ofPixels)
// Use the video frame data pointer externally with GetVideoData()
// For success, the video frame must be freed with FreeVideoData().
bool ofxNDIreceive::ReceiveImage(unsigned int &width, unsigned int &height)
{
	NDIlib_frame_type_e NDI_frame_type;
	NDIlib_metadata_frame_t metadata_frame;
	// NDIlib_audio_frame_v2_t audio_frame;
	// Vers 4.5
	NDIlib_audio_frame_v3_t audio_frame;
	m_FrameType = NDIlib_frame_type_none;
	bool bRet = false;

	if (!bNDIinitialized) {
		printf("ofxNDIreceive : ReceiveImage not initialized\n");
		return false;
	}

	// Create receiver if not initialized
	// or a new sender has been selected
	if (!OpenReceiver()) {
		return false;
	}

	if (pNDI_recv) {

		// NDI_frame_type = p_NDILib->recv_capture_v2(pNDI_recv, &video_frame, &audio_frame, &metadata_frame, 0);
		// Vers 4.5
		NDI_frame_type = p_NDILib->recv_capture_v3(pNDI_recv, &video_frame, &audio_frame, &metadata_frame, 0);

		// Set frame type for external access
		m_FrameType = NDI_frame_type;

		// Clear existing metadata if any
		if (!m_metadataString.empty())
			m_metadataString.clear();
		m_bMetadata = false;

		// Default is a video frame
		// Retain any audio data that has been received
		m_bAudioFrame = false;

		switch (NDI_frame_type) {

			// No data received or the connection lost
			case NDIlib_frame_type_none:
				break;

			case NDIlib_frame_type_error:
				break;

			// The settings on this input have changed
			case NDIlib_frame_type_status_change:
				break;

			// Metadata
			case NDIlib_frame_type_metadata :
				if (metadata_frame.p_data) {
					m_bMetadata = true;
					// Save the metadata string
					m_metadataString = metadata_frame.p_data;
					// ReceiveImage will return false
					// Use IsMetadata() to determine whether metadata has been received
					// Free the captured buffer
					p_NDILib->recv_free_metadata(pNDI_recv, &metadata_frame);
				}
				break;

			case NDIlib_frame_type_audio :

				if (audio_frame.p_data) {
					if (m_bAudio) {
						// Copy the audio data to a local audio buffer
						// Allocate only for sample size change
						if (m_nAudioSamples != audio_frame.no_samples
							|| m_nAudioSampleRate != audio_frame.sample_rate
							|| m_nAudioChannels != audio_frame.no_channels) {
							// printf("Creating audio buffer - %d samples, %d channels\n", audio_frame.no_samples, audio_frame.no_channels);
							if (m_AudioData) free((void *)m_AudioData);
							m_AudioData = (float *)malloc((size_t)audio_frame.no_samples * (size_t)audio_frame.no_channels * sizeof(float));
						}
						m_nAudioChannels = audio_frame.no_channels;
						m_nAudioSamples = audio_frame.no_samples;
						m_nAudioSampleRate = audio_frame.sample_rate;
						if (m_AudioData)
							memcpy((void *)m_AudioData, (void *)audio_frame.p_data, ((size_t)m_nAudioSamples * (size_t)audio_frame.no_channels * sizeof(float)));
						m_bAudioFrame = true;
						//
						// ReceiveImage will return false (no image received)
						//
						// Use IsAudioFrame() to determine whether audio has been received
						// and GetAudioData to retrieve the sample buffer
					}
					// Vers 4.5
					p_NDILib->recv_free_audio_v3(pNDI_recv, &audio_frame);
				}
				break;

			case NDIlib_frame_type_video :

				if (video_frame.p_data) {

					// The caller can check whether a frame has been received
					bReceiverConnected = true;

					if (m_Width != (unsigned int)video_frame.xres || m_Height != (unsigned int)video_frame.yres) {
						m_Width = (unsigned int)video_frame.xres;
						m_Height = (unsigned int)video_frame.yres;
					}

					// Retain the video frame pointer for external access.
					// Buffers captured must then be freed using FreeVideoData.
					// Update the caller dimensions and return received OK
					// for the app to handle changed dimensions
					width  = m_Width;
					height = m_Height;

					// Get the current video frame timecode
					// UTC time since the Unix Epoch (1/1/1970 00:00) with 100 ns precision.
					m_VideoTimecode = video_frame.timecode;

					// Get the current video frame timestamp
					m_VideoTimestamp = video_frame.timestamp;

					// Update received frame counter
					UpdateFps();

					// Only return true for video data
					bRet = true;

				}
				else {
					// No video data
					bReceiverConnected = false;
				}
				break; // endif NDIlib_frame_type_video

			case NDIlib_frame_type_max:
				// Not used
				break;

			default:
				break;

		} // end switch frame type
	} // endif pNDI_recv
	else {
		// No video data - no sender
		bReceiverConnected = false;
	}
	
	return bRet;
}

// Get the video type received
NDIlib_FourCC_video_type_e ofxNDIreceive::GetVideoType()
{
	return video_frame.FourCC;
}

// Video frame line stride in bytes
unsigned int ofxNDIreceive::GetVideoStride()
{
	return (unsigned int)video_frame.data_size_in_bytes;
}

// Get a pointer to the current video frame data
unsigned char *ofxNDIreceive::GetVideoData()
{
	return (unsigned char *)video_frame.p_data;
}

// Free NDI video frame buffers
void ofxNDIreceive::FreeVideoData()
{
	if (p_NDILib && video_frame.p_data) {
		p_NDILib->recv_free_video_v2(pNDI_recv, &video_frame);
		// Check that the video frame data pointer is null
		// because the function may not reset the pointer
		// and we return video_frame.p_data in GetVideoData().
		// A leak would occur here if the function fails
		// but the data is not ours to free.
		if (video_frame.p_data) {
			// Reset video frame data pointer
			video_frame.p_data = nullptr;
		}
	}
}

// Free local audio frame buffer
void ofxNDIreceive::FreeAudioData()
{
	// Free audio data
	if (m_AudioData) free((void *)m_AudioData);
	m_AudioData =nullptr;
	m_bAudioFrame = false;
	m_nAudioSampleRate = 0;
	m_nAudioSamples = 0;
	m_nAudioChannels = 0;
}

// Get NDI dll version number
std::string ofxNDIreceive::GetNDIversion()
{
	if (p_NDILib)
		return p_NDILib->version();
	else
		return "";
}

// Get the received frame rate
int ofxNDIreceive::GetFps()
{
	return static_cast<int>(floor(m_fps + 0.5));
}

// Reset starting received frame rate
void ofxNDIreceive::ResetFps(double fps)
{
	m_fps = fps;
}

//
// Private functions
//

// Version 2
// Replacement for deprecated NDIlib_find_get_sources.
// If no timeout specified, return the sources that exist right now.
// For a timeout, wait for that timeout and return the sources that exist then.
// If that fails, return nullptr.
const NDIlib_source_t* ofxNDIreceive::FindGetSources(NDIlib_find_instance_t p_instance,
	uint32_t* p_no_sources,
	uint32_t timeout_in_ms)
{
	if (!bNDIinitialized) 
		return nullptr;

	if (!p_instance)
		return nullptr;

	if ((!timeout_in_ms) || (p_NDILib->find_wait_for_sources(p_instance, timeout_in_ms))) {
		// Recover the current set of sources (i.e. the ones that exist right this second)
		return p_NDILib->find_get_current_sources(p_instance, p_no_sources);
	}

	return nullptr;

}


// Received fps is independent of the application draw rate
void ofxNDIreceive::UpdateFps() {

	// Calculate the actual received fps
	lastTime = startTime;
	startTime = GetCounter(); // msec
	double frametime = (startTime - lastTime); // msec
	if (frametime > 1.0) {
		frametime = frametime/1000.0; // frame time in seconds
		// damping based on received frame time
		if (frametime <= 1.0) {
			m_fps *= (1.0-frametime);
			m_fps += frametime*(1.0/frametime);
		}
	}
}

void ofxNDIreceive::StartCounter()
{
	LARGE_INTEGER li;
	if (!QueryPerformanceFrequency(&li)) {
		printf("QueryPerformanceFrequency failed!\n");
		return;
	}
	PCFreq = double(li.QuadPart) / 1000.0;
	QueryPerformanceCounter(&li);
	CounterStart = li.QuadPart;
	// Reset starting frame rate value
	m_fps = 30.0;
}

double ofxNDIreceive::GetCounter()
{
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	return double(li.QuadPart - CounterStart) / PCFreq;
}
