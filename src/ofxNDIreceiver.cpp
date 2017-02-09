/*
	NDI Receiver

	using the NDI SDK to receive frames from the network

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

*/
#include "ofxNDIreceiver.h"

// Version 2
static std::atomic<bool> exit_loop(false);
static void sigint_handler(int)
{	
	exit_loop = true;
}


ofxNDIreceiver::ofxNDIreceiver()
{
	pNDI_find = NULL;
	pNDI_recv = NULL;
	p_sources = NULL;
	no_sources = 0;
	bNDIinitialized = false;
	bSenderSelected = false;
	nsenders = 0;
	m_Width = 0;
	m_Height = 0;
	senderIndex = 0;
	m_bandWidth = NDIlib_recv_bandwidth_highest;

	if(!NDIlib_is_supported_CPU() ) {
		MessageBoxA(NULL, "CPU does not support NDI\nNDILib requires SSE4.1", "NDIreceiver", MB_OK);
	}
	else {
		bNDIinitialized = NDIlib_initialize();
		if(!bNDIinitialized) {
			MessageBoxA(NULL, "Cannot run NDI\nNDILib initialization failed", "NDIreceiver", MB_OK);
		}
		else {
			// Version 2
			// Catch interrupt so that we can shut down gracefully
			signal(SIGINT, sigint_handler);
		}
	}

}


ofxNDIreceiver::~ofxNDIreceiver()
{
	if(pNDI_recv) NDIlib_recv_destroy(pNDI_recv);
	if(pNDI_find) NDIlib_find_destroy(pNDI_find);
	if(bNDIinitialized)	NDIlib_destroy();
	
}


// Create a finder to look for a sources on the network
void ofxNDIreceiver::CreateFinder()
{
	if(!bNDIinitialized) return;

	if(pNDI_find) NDIlib_find_destroy(pNDI_find);
	// const NDIlib_find_create_t NDI_find_create_desc = { TRUE, NULL };
	const NDIlib_find_create_t NDI_find_create_desc = { TRUE, NULL, NULL }; // Version 2
	// pNDI_find = NDIlib_find_create(&NDI_find_create_desc);
	pNDI_find = NDIlib_find_create2(&NDI_find_create_desc);
	p_sources = NULL;
	no_sources = 0;
	nsenders = 0;

}


// Release the current finder
void ofxNDIreceiver::ReleaseFinder()
{
	if(!bNDIinitialized) return;

	if(pNDI_find) NDIlib_find_destroy(pNDI_find);
	pNDI_find = NULL;
	p_sources = NULL;
	no_sources = 0;

}

//
// Version 2
//
// Replacement for deprecated NDIlib_find_get_sources
//
const NDIlib_source_t* ofxNDIreceiver::FindGetSources(NDIlib_find_instance_t p_instance, 
													  uint32_t* p_no_sources,
													  uint32_t timeout_in_ms)
{
	if(!p_instance)
		return NULL;

	// If no timeout specified, return the sources that exist right now
	// For a tiemout, wait for that timeout and return the sources that exist then
	// If that fails, return NULL
	if ((!timeout_in_ms) || (NDIlib_find_wait_for_sources(p_instance, timeout_in_ms))) {
		// Recover the current set of sources (i.e. the ones that exist right this second)
		return NDIlib_find_get_current_sources(p_instance, p_no_sources);
	}

	return NULL;
}


int ofxNDIreceiver::FindSenders()
{
	std::string name;

	if(!bNDIinitialized) {
		printf("NDI not initialized\n");
		return 0;
	}

	// If a finder was created, use it to find senders on the network
	if(pNDI_find) {
		//
		// This may be called for every frame so has to be fast.
		//
		// Specify a delay so that p_sources is returned only for a network change.
		// If there was no network change, p_sources is NULL and no_sources = 0 
		// and can't be used for other functions so the sender names as well as 
		// the sender count need to be saved locally.
		// Version 2
		// p_sources = NDIlib_find_get_sources(pNDI_find, &no_sources, 1);
		p_sources = FindGetSources(pNDI_find, &no_sources, 1);
		if(p_sources && no_sources > 0) {
			printf("no_sources = %d\n", no_sources);
			NDIsenders.clear();
			nsenders = 0;
			if(p_sources && no_sources > 0) {
				for(int i = 0; i < (int)no_sources; i++) {
					// The sender name should be valid in the list but check anyway to avoid a crash
					if(p_sources[i].p_ndi_name && p_sources[i].p_ndi_name[0]) {
						printf("Sender %d [%s]\n", i, p_sources[i].p_ndi_name);
						name = p_sources[i].p_ndi_name;
						NDIsenders.push_back(name);
						nsenders++;
					}
				}
			}
		}
	}
	else {
		CreateFinder();
	}

	return nsenders;
}


// Refresh sender list with the current network snapshot
// int ofxNDIreceiver::RefreshSenders(DWORD dwTimeout)
int ofxNDIreceiver::RefreshSenders(uint32_t timeout)
{
	std::string name;

	if(!bNDIinitialized) return 0;

	// Release the current finder
	if(pNDI_find) ReleaseFinder();
	if(!pNDI_find) CreateFinder();

	// If a finder was created, use it to find senders on the network
	// Give it a timeout in case of connection trouble.
	if(pNDI_find) {
		// Clear the current local list
		NDIsenders.clear();
		nsenders = 0;

		// Version 2 ?? test
		// p_sources = FindGetSources(pNDI_find, &no_sources, timeout);

		dwStartTime = timeGetTime();
		dwElapsedTime = 0;
		do {
			p_sources = NDIlib_find_get_current_sources(pNDI_find, &no_sources);
			dwElapsedTime = timeGetTime() - dwStartTime;
		} while(no_sources == 0 && (uint32_t)dwElapsedTime < timeout);


		if(p_sources && no_sources > 0) {
			NDIsenders.clear();
			nsenders = 0;
			for(int i = 0; i < (int)no_sources; i++) {
				// The sender name should be valid in the list but check anyway to avoid a crash
				if(p_sources[i].p_ndi_name && p_sources[i].p_ndi_name[0]) {
					name = p_sources[i].p_ndi_name;
					NDIsenders.push_back(name);
					nsenders++;
				}
			}
			return nsenders;
		}
	}

	return 0;
}

// Set the sender list index variable
void ofxNDIreceiver::SetSenderIndex(int index)
{
	if(!bNDIinitialized) return;

	senderIndex = index;
	if(senderIndex > nsenders) {
		senderIndex = 0;
	}
	bSenderSelected = true; // Set to show the user has changed

}

// Return the index of the current sender
int ofxNDIreceiver::GetSenderIndex()
{
	return senderIndex;
}


// Return the index of a sender name
bool ofxNDIreceiver::GetSenderIndex(char *sendername, int &index)
{
	if(NDIsenders.size() > 0) {
		for(int i=0; i<(int)NDIsenders.size(); i++)  {
			if(strcmp(sendername, NDIsenders.at(i).c_str()) == 0) {
				index = i;
				return true;
			}
		}
	}

	return false;
}

// Has the user changed the sender index ?
bool ofxNDIreceiver::SenderSelected()
{
	bool bSelected = bSenderSelected;
	bSenderSelected = false; // one off - the user has to select again

	return bSelected;
}

int ofxNDIreceiver::GetSenderCount()
{
	return (int)NDIsenders.size();
}


// Return a sender name for the requested index
bool ofxNDIreceiver::GetSenderName(char *sendername, int userindex)
{
	int index = userindex; 

	// If no index has been specified, use the currently selected index
	if(userindex < 0) 
		index = senderIndex;

	if(NDIsenders.size() > 0 && (unsigned int)index < NDIsenders.size() && !NDIsenders.empty()) {
		strcpy_s(sendername, 256, NDIsenders.at(index).c_str());
		return true;
	}

	return false;
}

//
// TODO : not working
//
// Bandwidth
//
// NDIlib_recv_bandwidth_lowest will provide a medium quality stream that takes almost no bandwidth,
// this is normally of about 640 pixels in size on it is longest side and is a progressive video stream.
// NDIlib_recv_bandwidth_highest will result in the same stream that is being sent from the up-stream source
//
void ofxNDIreceiver::SetLowBandwidth(bool bLow)
{
	if(bLow) {
		// printf("Set low bandwidth\n");
		m_bandWidth = NDIlib_recv_bandwidth_lowest; // Low bandwidth receive option
	}
	else {
		// printf("Set High bandwidth\n");
		m_bandWidth = NDIlib_recv_bandwidth_highest;
	}


}


//
// Metadata
//
bool ofxNDIreceiver::IsMetadata()
{
	return m_bMetadata;
}

std::string ofxNDIreceiver::GetMetadataString()
{
	return m_metadataString;
}


// HB for RGBA
bool ofxNDIreceiver::CreateReceiver(int userindex)
{
	// Default to BGRA if no format color format is specified
	return CreateReceiver(NDIlib_recv_color_format_e_BGRX_BGRA,userindex);
}

bool ofxNDIreceiver::CreateReceiver(NDIlib_recv_color_format_e colorFormat , int userindex)
{
	if (!bNDIinitialized) return false;

	int index = userindex;

	if (!pNDI_recv) {

		// The continued check in FindSenders is for a network change and
		// p_sources is returned NULL, so we need to find all the sources
		// again to get a pointer to the selected sender.
		// Give it a timeout in case of connection trouble.

		// Does not work
		// p_sources = FindGetSources(pNDI_find, &no_sources, 4000);
		// Does not work
		// if(NDIlib_find_wait_for_sources(pNDI_find, 4000))
			// p_sources = NDIlib_find_get_current_sources(pNDI_find, &no_sources);

		if (pNDI_find) {
			dwStartTime = timeGetTime();
			do {
				// p_sources = NDIlib_find_get_sources(pNDI_find, &no_sources, 0);
				p_sources = NDIlib_find_get_current_sources(pNDI_find, &no_sources);
				dwElapsedTime = timeGetTime() - dwStartTime;
			} while (no_sources == 0 && dwElapsedTime < 4000);
		}

		// TODO - reset sender name vector?

		if (p_sources && no_sources > 0) {

			// If no index has been specified, use the currently set index
			if (userindex < 0)
				index = senderIndex;

			// We tell it that we prefer BGRA
			// TODO : does "prefer" mean we might get YUV as well ?
			// NDIlib_recv_create_t NDI_recv_create_desc = { p_sources[senderIndex], FALSE };
			// 16-06-16 - SDK change
			NDIlib_recv_create_t NDI_recv_create_desc = {
				p_sources[index],
				colorFormat,
				m_bandWidth, // Changed by SetLowBandwidth, default NDIlib_recv_bandwidth_highest
				TRUE };

			// Create the receiver
			// Deprecated version sets bandwidth to highest and allow fields to true.
			// pNDI_recv = NDIlib_recv_create(&NDI_recv_create_desc);
			pNDI_recv = NDIlib_recv_create2(&NDI_recv_create_desc);
			if (!pNDI_recv) {
				return false;
			}

			// on_program = TRUE, on_preview = FALSE
			const NDIlib_tally_t tally_state = { TRUE, FALSE };
			NDIlib_recv_set_tally(pNDI_recv, &tally_state);

			return true;

		}
	} // end create receiver

	return false;
}



void ofxNDIreceiver::ReleaseReceiver()
{
	if(!bNDIinitialized) return;

	if(pNDI_recv) NDIlib_recv_destroy(pNDI_recv);
	m_Width = 0;
	m_Height = 0;
	pNDI_recv = NULL;
	bSenderSelected = false;

}



bool ofxNDIreceiver::ReceiveImage(unsigned char *pixels, 
								  unsigned int &width, unsigned int &height, 
								  bool bInvert)
{
	NDIlib_frame_type_e NDI_frame_type;
	NDIlib_metadata_frame_t metadata_frame;

	if(pNDI_recv) {

		// NDI_frame_type = NDIlib_recv_capture(pNDI_recv, &video_frame, NULL, NULL, 0); // 16);
		NDI_frame_type = NDIlib_recv_capture(pNDI_recv, &video_frame, NULL, &metadata_frame, 0); 

		// Is the connection lost or no data received ?
		if(NDI_frame_type == NDIlib_frame_type_error || NDI_frame_type == NDIlib_frame_type_none) {
			return false;
		}

		// Metadata
		if(NDI_frame_type == NDIlib_frame_type_metadata) {
			// printf("ofxNDIreceiver::ReceiveImage - Received Metadata\n%s\n", metadata_frame.p_data);
			m_bMetadata = true;
			m_metadataString = metadata_frame.p_data;
		}
		else {
			// printf("ofxNDIreceiver::ReceiveImage - No metadata\n");
			m_bMetadata = false;
			m_metadataString.empty();
		}

		if(video_frame.p_data && NDI_frame_type == NDIlib_frame_type_video)	{
			if(m_Width != video_frame.xres || m_Height != video_frame.yres) {
				m_Width  = video_frame.xres;
				m_Height = video_frame.yres;
				// Update the caller dimensions and return received OK
				// for the app to handle changed dimensions
				width = m_Width;
				height = m_Height;
				return true;
			}

			// Otherwise sizes are current - copy the received frame data to the local buffer
			if(video_frame.p_data && pixels) {

				/*
				// Preferred type is bgra, so YUV may never be received anyway
				if(video_frame.FourCC == NDIlib_FourCC_type_UYVY)
					ofxNDIutils::YUV422_to_RGBA((const unsigned char *)video_frame.p_data, pixels, m_Width, m_Height, (unsigned int)video_frame.line_stride_in_bytes);
				else
					ofxNDIutils::CopyImage((const unsigned char *)video_frame.p_data, pixels, m_Width, m_Height, (unsigned int)video_frame.line_stride_in_bytes, bSwapRB, bInvert);
				*/
				// Preferred type is bgra
				switch(video_frame.FourCC) {

					// NDIlib_FourCC_type_UYVA not supported
					// Alpha copied as received

					case NDIlib_FourCC_type_UYVY: // YCbCr color space
						ofxNDIutils::YUV422_to_RGBA((const unsigned char *)video_frame.p_data, pixels, m_Width, m_Height, (unsigned int)video_frame.line_stride_in_bytes);
						break;

					case NDIlib_FourCC_type_RGBA: // RGBA
					case NDIlib_FourCC_type_RGBX: // RGBX
						// printf("RGBA\n");
						ofxNDIutils::CopyImage((const unsigned char *)video_frame.p_data, pixels, m_Width, m_Height, (unsigned int)video_frame.line_stride_in_bytes, false, bInvert);
						break;

					case NDIlib_FourCC_type_BGRA: // BGRA
					case NDIlib_FourCC_type_BGRX: // BGRX
					default: // BGRA
						// printf("BGRA\n");
						ofxNDIutils::CopyImage((const unsigned char *)video_frame.p_data, pixels, m_Width, m_Height, (unsigned int)video_frame.line_stride_in_bytes, true, bInvert);
						break;

				} // end switch received format

				
				// Buffers captured must be freed
				NDIlib_recv_free_video(pNDI_recv, &video_frame);

				// The caller always checks the received dimensions
				width = m_Width;
				height = m_Height;

				return true;

			} // endif video frame data
			return true;

		} // endif NDIlib_frame_type_video
	} // endif pNDI_recv

	return false;
}
