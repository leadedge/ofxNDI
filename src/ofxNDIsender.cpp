/*
	NDI sender

	using the NDI SDK to send the frames via network

	http://NDI.NewTek.com

	Copyright (C) 2016-2018 Lynn Jarvis.

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
	07.11.16	- Added CPU support check
	12.11.16	- Fix MessageBox \N to \nN
	13.11.16	- Do not clock the video for async sending
	15.11.16	- add audio support
	09.02.17	- include changes by Harvey Buchan for NDI SDK version 2
				  (RGBA sender option)
				- Added Metadata
	17.02.17	- Added MetaData functions
				- Added GetNDIversion - NDIlib_version
	22.02.17	- corrected DWORD cast to int in NDI_connection_type
	// Changes for NDI Version 3
	04.11.17	- const char for SendImage
				- change functions to _v2
				- change variable types
	31.03.18	- Update to NDI SDK Version 3 - search on "Vers 3"
				- change functions to _v2
				- change variable types

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

	// Audio
	bNDIaudio = false; // No audio default
	m_AudioSampleRate = 48000; // 48kHz
	m_AudioChannels = 1; // Default mono
	m_AudioSamples = 1602; // There can be up to 1602 samples, can be changed on the fly
	m_AudioTimecode = NDIlib_send_timecode_synthesize; // Timecode (synthesized for us !)
	m_AudioData = NULL; // Audio buffer

	if(!NDIlib_is_supported_CPU() ) {
		MessageBoxA(NULL, "CPU does not support NDI\nNDILib requires SSE4.1", "NDIsender", MB_OK);
		bNDIinitialized = false;
	}
	else {
		bNDIinitialized = NDIlib_initialize();
		if(!bNDIinitialized) {
			MessageBoxA(NULL, "Cannot run NDI\nNDILib initialization failed", "NDIsender", MB_OK);
		}
	}
}

// Create default BGRA sender
bool ofxNDIsender::CreateSender(const char *sendername, unsigned int width, unsigned int height)
{
	return CreateSender(sendername, width, height, NDIlib_FourCC_type_BGRA);
}

// Allow for creating RGBA sender
bool ofxNDIsender::CreateSender(const char *sendername, unsigned int width, unsigned int height, NDIlib_FourCC_type_e colorFormat)
{
	// Create an NDI source that is clocked to the video.
	// unless async sending has been selected.
	// Vers 3
	// NDI_send_create_desc.p_ndi_name = (const char *)sendername;
	NDI_send_create_desc.p_ndi_name = sendername;
	NDI_send_create_desc.p_groups = NULL;
	NDI_send_create_desc.clock_video = m_bClockVideo;
	NDI_send_create_desc.clock_audio = false; // FALSE;

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
			(int)::strlen(p_connection_string),
			// Timecode (synthesized for us !)
			NDIlib_send_timecode_synthesize,
			// The string
			p_connection_string
		};

		NDIlib_send_add_connection_metadata(pNDI_send, &NDI_connection_type);
		
		// We are going to create an non-interlaced frame at 60fps
		if(p_frame) free((void *)p_frame);
		p_frame = NULL; // invert  buffer

		video_frame.xres = (int)width;
		video_frame.yres = (int)height;
		video_frame.FourCC = colorFormat;
		video_frame.frame_rate_N = m_frame_rate_N; // clock the frame (default 60fps)
		video_frame.frame_rate_D = m_frame_rate_D;
		video_frame.picture_aspect_ratio = m_picture_aspect_ratio; // default source (width/height)
		// 24-1-17 SDK Change to NDI v2
		//video_frame.is_progressive = m_bProgressive; // progressive of interlaced (default progressive)
		if (m_bProgressive) video_frame.frame_format_type = NDIlib_frame_format_type_progressive;
		else video_frame.frame_format_type = NDIlib_frame_format_type_interleaved;
		// The timecode of this frame in 100ns intervals
		video_frame.timecode = NDIlib_send_timecode_synthesize; // 0LL; // Let the API fill in the timecodes for us.
		video_frame.p_data = NULL;
		video_frame.line_stride_in_bytes = (int)width*4; // The stride of a line BGRA

		if(bNDIaudio) {
			// Create an audio buffer
			audio_frame.sample_rate = m_AudioSampleRate;
			audio_frame.no_channels = m_AudioChannels;
			audio_frame.no_samples  = m_AudioSamples;
			audio_frame.timecode    = m_AudioTimecode;
			audio_frame.p_data      = m_AudioData;
			// mono/stereo inter channel stride
			audio_frame.channel_stride_in_bytes = (m_AudioChannels-1)*m_AudioSamples*sizeof(FLOAT);
		}

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
		// NDIlib_send_send_video_async(pNDI_send, NULL);
		NDIlib_send_send_video_async_v2(pNDI_send, NULL);
	}

	// Free the local buffer, it is re-created in SendImage if invert is needed
	if(p_frame) free((void *)p_frame);
	p_frame = NULL;
	video_frame.p_data = NULL;

	// Reset video frame size
	video_frame.xres = (int)width;
	video_frame.yres = (int)height;
	video_frame.line_stride_in_bytes = (int)width * 4;

	return true;
}


// Vers 3
// void ofxNDIsender::SetFrameRate(DWORD framerate_N, DWORD framerate_D)
void ofxNDIsender::SetFrameRate(int framerate_N, int framerate_D)
{
	m_frame_rate_N = framerate_N;
	m_frame_rate_D = framerate_D;
	// Aspect ratio is calculated in CreateSender
}

// Vers 3
// void ofxNDIsender::GetFrameRate(DWORD &framerate_N, DWORD &framerate_D)
void ofxNDIsender::GetFrameRate(int &framerate_N, int &framerate_D)
{
	framerate_N = m_frame_rate_N;
	framerate_D = m_frame_rate_D;
}

// Vers 3
// void ofxNDIsender::SetAspectRatio(DWORD horizontal, DWORD vertical)
void ofxNDIsender::SetAspectRatio(int horizontal, int vertical)
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
	if(bAsync)
		m_bClockVideo = false; // Do not clock the video for async sending
	else
		m_bClockVideo = true;
}


// Get asynchronous sending mode
bool ofxNDIsender::GetAsync()
{
	return bAsync;
}

//
// Audio
//
void ofxNDIsender::SetAudio(bool bAudio)
{
	bNDIaudio = bAudio;
}

// Vers 3
// void ofxNDIsender::SetAudioSampleRate(DWORD sampleRate)
void ofxNDIsender::SetAudioSampleRate(int sampleRate)
{
	m_AudioSampleRate = sampleRate;
	audio_frame.sample_rate = sampleRate;
}

// Vers 3
// void ofxNDIsender::SetAudioChannels(DWORD nChannels)
void ofxNDIsender::SetAudioChannels(int nChannels)
{
	m_AudioChannels = nChannels;
	audio_frame.no_channels = nChannels;
	audio_frame.channel_stride_in_bytes = (m_AudioChannels-1)*m_AudioSamples*sizeof(FLOAT);
}

// Vers 3
// void ofxNDIsender::SetAudioSamples(DWORD nSamples)
void ofxNDIsender::SetAudioSamples(int nSamples)
{
	m_AudioSamples = nSamples;
	audio_frame.no_samples  = nSamples;
	audio_frame.channel_stride_in_bytes = (m_AudioChannels-1)*m_AudioSamples*sizeof(FLOAT);
}

// Vers 3
// void ofxNDIsender::SetAudioTimecode(LONGLONG timecode)
void ofxNDIsender::SetAudioTimecode(int64_t timecode)
{
	m_AudioTimecode = timecode;
	audio_frame.timecode = timecode;
}

// Vers 3
// void ofxNDIsender::SetAudioData(FLOAT *data)
void ofxNDIsender::SetAudioData(float *data)
{
	m_AudioData = data;
	audio_frame.p_data = data;
}

//
// Metadata
//
void ofxNDIsender::SetMetadata(bool bMetadata)
{
	m_bMetadata = bMetadata;
}

void ofxNDIsender::SetMetadataString(std::string datastring)
{
	m_metadataString = datastring;
}


// Get NDI dll version number
std::string ofxNDIsender::GetNDIversion()
{
	return NDIlib_version();
}

bool ofxNDIsender::SendImage(const unsigned char * pixels, unsigned int width, unsigned int height,
							 bool bSwapRB, bool bInvert)
{
	if(pixels && width > 0 && height > 0) {

		// Allow for forgotten UpdateSender
		if(video_frame.xres != (int)width || video_frame.yres != (int)height) {
			video_frame.xres = (int)width;
			video_frame.yres = (int)height;
			video_frame.line_stride_in_bytes = width * 4;
		}

		if(bSwapRB || bInvert) {
			// Local memory buffer is only needed for rgba to bgra or invert
			if(!p_frame) {
				// Vers 3
				// p_frame = (BYTE*)malloc(width*height*4*sizeof(unsigned char));
				p_frame = (uint8_t*)malloc(width*height * 4 * sizeof(unsigned char));
				if(!p_frame) {
					MessageBoxA(NULL, "Out of memory in SendImage\n", "NDIsender", MB_OK); 
					return false;
				}
				// Vers 3
				// video_frame.p_data = (BYTE *)p_frame;
				video_frame.p_data = p_frame;
				
			}
			ofxNDIutils::CopyImage((const unsigned char *)pixels, (unsigned char *)video_frame.p_data,
									width, height, (unsigned int)video_frame.line_stride_in_bytes, bSwapRB, bInvert);
		}
		else {
			// No bgra conversion or invert, so use the pointer directly
			// video_frame.p_data = (BYTE *)pixels;
			video_frame.p_data = (uint8_t*)pixels;

		}

		// Submit the audio buffer first.
		// Refer to the NDI SDK example where for 48000 sample rate
		// and 29.97 fps, an alternating sample number is used.
		// Do this in the application using SetAudioSamples(nSamples);
		// General reference : http://jacklinstudios.com/docs/post-primer.html
		if(bNDIaudio && audio_frame.p_data != NULL)
			NDIlib_send_send_audio_v2(pNDI_send, &audio_frame);
			// NDIlib_send_send_audio(pNDI_send, &audio_frame);

		// Metadata
		if(m_bMetadata && !m_metadataString.empty()) {
			// metadata_frame.length = (DWORD)m_metadataString.size();
			metadata_frame.length = (int)m_metadataString.size();
			metadata_frame.timecode = NDIlib_send_timecode_synthesize;
			// metadata_frame.p_data = (CHAR *)m_metadataString.c_str(); // XML message format
			metadata_frame.p_data = (char *)m_metadataString.c_str(); // XML message format
			NDIlib_send_send_metadata(pNDI_send, &metadata_frame);
			// printf("Metadata\n%s\n", m_metadataString.c_str());
		}

		if(bAsync) {
			// Submit the frame asynchronously. This means that this call will return immediately and the 
			// API will "own" the memory location until there is a synchronizing event. A synchronouzing event is 
			// one of : NDIlib_send_send_video_async, NDIlib_send_send_video, NDIlib_send_destroy
			// NDIlib_send_send_video_async(pNDI_send, &video_frame);
			NDIlib_send_send_video_async_v2(pNDI_send, &video_frame);
		}
		else {
			// Submit the frame. Note that this call will be clocked
			// so that we end up submitting at exactly the predetermined fps.
			// NDIlib_send_send_video(pNDI_send, &video_frame);
			NDIlib_send_send_video_v2(pNDI_send, &video_frame);
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

