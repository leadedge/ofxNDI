/*
	NDI sender

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

	13-06-16	- removed internal buffer
				- used in-place buffer flip function so that internal buffer is not needed
	27.07.16	- restored FlipBuffer with additional temporary buffer
				- used optimised assembler memcpy in FlipBuffer - 4-5 fps increase at 2560x1440
				  FlipBuffer should be avoided if a GPU texture copy/invert is possible
	10.10.16	- updated SSE2 memcpy with intrinsics for 64bit compatibility
	12.10.16	- Included a bgra conversion option for SendImage
	05.11.16	- Added SetClockVideo
*/
#include "ofxNDIsender.h"

ofxNDIsender::ofxNDIsender()
{
	pNDI_send = NULL;
	p_frame = NULL;
	m_frame_rate_N = 60000; // 60 fps default
	m_frame_rate_D = 1000;
	// m_frame_rate_N = 30000; // 29.97 fps
	// m_frame_rate_D = 1001;
	m_horizontal_aspect = 1; // source aspect ratio by default
	m_vertical_aspect = 1;
	m_picture_aspect_ratio = 16.0f/9.0f;
	m_bProgressive = TRUE; // progressive default
	m_bClockVideo = TRUE; // clock video default
	bAsync = false;
	bNDIinitialized = false;

	bNDIinitialized = NDIlib_initialize();
	if(!bNDIinitialized) {
		// Cannot run NDI. Most likely because the CPU is not sufficient (see SDK documentation).
		// you can check this directly with a call to NDIlib_is_supported_CPU()
		MessageBoxA(NULL, "Cannot run NDI", "NDIsender", MB_OK);
	}
}


bool ofxNDIsender::CreateSender(const char *sendername, unsigned int width, unsigned int height)
{

	// Create an NDI source that is clocked to the video.
	NDI_send_create_desc.p_ndi_name = (const char *)sendername;
	NDI_send_create_desc.p_groups = NULL;
	NDI_send_create_desc.clock_video = m_bClockVideo;
	NDI_send_create_desc.clock_audio = FALSE;

	// Calulate aspect ratio
	// Source (1:1)
	// Normal 4:3
	// Widescreen 16:9

	// 1:1 means use the source aspect ratio
	if(m_horizontal_aspect == 1 && m_vertical_aspect == 1) 
		m_picture_aspect_ratio = (float)width/(float)height;
	else
		m_picture_aspect_ratio = (float)m_horizontal_aspect/(float)m_vertical_aspect;

	// We create the NDI sender
	pNDI_send = NDIlib_send_create(&NDI_send_create_desc);

	if (pNDI_send) {
		// Provide a meta-data registration that allows people to know what we are. Note that this is optional.
		// Note that it is possible for senders to also register their preferred video formats.
		char* p_connection_string = "<ndi_product long_name=\"Spout NDI sender\" "
												 "             short_name=\"Spout NDI Sender\" "
												 "             manufacturer=\"spout@zeal.co\" "
												 "             version=\"1.000.000\" "
												 "             session=\"default\" "
												 "             model_name=\"none\" "
												 "             serial=\"none\"/>";
		
		const NDIlib_metadata_frame_t NDI_connection_type = {
			// The length
			(DWORD)::strlen(p_connection_string),
			// Timecode (synthesized for us !)
			NDIlib_send_timecode_synthesize,
			// The string
			p_connection_string
		};

		NDIlib_send_add_connection_metadata(pNDI_send, &NDI_connection_type);
		
		// We are going to create an non-interlaced frame at 60fps
		if(p_frame) free((void *)p_frame);
		p_frame = NULL; // invert  buffer

		video_frame.xres = width;
		video_frame.yres = height;
		video_frame.FourCC = NDIlib_FourCC_type_BGRA;
		video_frame.frame_rate_N = m_frame_rate_N; // clock the frame (default 60fps)
		video_frame.frame_rate_D = m_frame_rate_D;
		video_frame.picture_aspect_ratio = m_picture_aspect_ratio; // default source (width/height)
		video_frame.is_progressive = m_bProgressive; // progressive of interlaced (default progressive)
		// The timecode of this frame in 100ns intervals
		video_frame.timecode = NDIlib_send_timecode_synthesize; // 0LL; // Let the API fill in the timecodes for us.
		video_frame.p_data = NULL;
		video_frame.line_stride_in_bytes = width*4; // The stride of a line BGRA

		return true;
	}

	return false;
}


bool ofxNDIsender::UpdateSender(unsigned int width, unsigned int height)
{
	if(pNDI_send && bAsync) {
		// Because one buffer is in flight we need to make sure that 
		// there is no chance that we might free it before NDI is done with it. 
		// You can ensure this either by sending another frame, or just by
		// sending a frame with a NULL pointer.
		NDIlib_send_send_video_async(pNDI_send, NULL);
	}

	// Free the local buffer, it is re-created in SendImage if invert is needed
	if(p_frame) free((void *)p_frame);
	p_frame = NULL;
	video_frame.p_data = NULL;

	// Reset video frame size
	video_frame.xres = width;
	video_frame.yres = height;
	video_frame.line_stride_in_bytes = width * 4;

	return true;
}


void ofxNDIsender::SetFrameRate(DWORD framerate_N, DWORD framerate_D)
{
	m_frame_rate_N = framerate_N;
	m_frame_rate_D = framerate_D;
	// Aspect ratio is calculated in CreateSender
}

void ofxNDIsender::GetFrameRate(DWORD &framerate_N, DWORD &framerate_D)
{
	framerate_N = m_frame_rate_N;
	framerate_D = m_frame_rate_D;
}

void ofxNDIsender::SetAspectRatio(DWORD horizontal, DWORD vertical)
{
	m_horizontal_aspect = horizontal;
	m_vertical_aspect = vertical;
	// Calculate for return
	m_picture_aspect_ratio = (float)horizontal/(float)vertical;
}

void ofxNDIsender::GetAspectRatio(float &aspect)
{
	aspect = m_picture_aspect_ratio;
}

void ofxNDIsender::SetProgressive(bool bProgressive)
{
	m_bProgressive = bProgressive;
}

bool ofxNDIsender::GetProgressive()
{
	if(m_bProgressive)
		return true;
	else
		return false;
}

void ofxNDIsender::SetClockVideo(bool bClocked)
{
	m_bClockVideo = bClocked;
}

bool ofxNDIsender::GetClockVideo()
{
	if(m_bClockVideo)
		return true;
	else
		return false;
}

// Set asynchronous sending mode
void ofxNDIsender::SetAsync(bool bActive)
{
	bAsync = bActive;
}


// Get asynchronous sending mode
bool ofxNDIsender::GetAsync()
{
	return bAsync;
}


bool ofxNDIsender::SendImage(unsigned char * pixels, unsigned int width, unsigned int height,
							 bool bSwapRB, bool bInvert)
{
	if(pixels && width > 0 && height > 0) {

		// Allow for forgotten UpdateSender
		if(video_frame.xres != width || video_frame.yres != height) {
			video_frame.xres = width;
			video_frame.yres = height;
			video_frame.line_stride_in_bytes = width * 4;
		}

		if(bSwapRB || bInvert) {
			// Local memory buffer is only needed for rgba to bgra or invert
			if(!p_frame) {
				p_frame = (BYTE*)malloc(width*height*4*sizeof(unsigned char));
				if(!p_frame) {
					MessageBoxA(NULL, "Out of memory in SendImage\n", "NDIsenderL", MB_OK); 
					return false;
				}
				video_frame.p_data = (BYTE *)p_frame;
			}
			ofxNDIutils::CopyImage((const unsigned char *)pixels, (unsigned char *)video_frame.p_data,
									width, height, (unsigned int)video_frame.line_stride_in_bytes, bSwapRB, bInvert);
		}
		else {
			// No bgra conversion or invert, so use the pointer directly
			video_frame.p_data = (BYTE *)pixels;
		}

		if(bAsync) {
			// Submit the frame asynchronously. This means that this call will return immediately and the 
			// API will "own" the memory location until there is a synchronizing event. A synchronouzing event is 
			// one of : NDIlib_send_send_video_async, NDIlib_send_send_video, NDIlib_send_destroy
			NDIlib_send_send_video_async(pNDI_send, &video_frame);
		}
		else {
			// Submit the frame. Note that this call will be clocked
			// so that we end up submitting at exactly the predetermined fps.
			NDIlib_send_send_video(pNDI_send, &video_frame);
		}
		return true;
	}

	return false;
}


void ofxNDIsender::ReleaseSender()
{
	// Destroy the NDI sender
	if(pNDI_send) NDIlib_send_destroy(pNDI_send);

	// Release the invert buffer
	if(p_frame) free((void*)p_frame);

	p_frame = NULL;
	pNDI_send = NULL;

}


ofxNDIsender::~ofxNDIsender()
{
	// Release the library
	NDIlib_destroy();
	bNDIinitialized = false;
}

