/*
	NDI Receiver

	using the NDI SDK to receive frames from the network

	http://NDI.NewTek.com

	Copyright (C) 2016-2021 Lynn Jarvis.

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

	08.07.16 - Uses ofxNDIreceive class
	11.07.18 - Add ReceiveImage for a texture
			 - Change to always create an RGBA receiver.
			   Allow the SDK to do the conversion from other formats. 
			   Openframeworks is RGBA so any other format would need
			   to be converted in any case. 
	16.07.18 - Add GetFrameType
	06.08.18 - Add receive to ofFbo
			 - Check for receiver creation in ReceiveImage to unsigned char array

	New functions and changes for 3.5 update:

			bool ReceiverCreated()
			bool ReceiveImage(ofFbo &fbo)
			bool ReceiveImage(ofTexture &texture)
			bool ReceiveImage(ofImage &image)
			bool ReceiveImage(ofPixels &pixels)
			NDIlib_frame_type_e GetFrameType()
			bool GetSenderName(char *sendername, int maxsize, int index = -1)
			std::string GetSenderName(int index = -1)
			unsigned int GetSenderWidth()
			unsigned int GetSenderHeight()
			double GetFps()

	24.03.19 - Add float GetSenderFps(), bool ReceiverConnected()
	05.11.18 - Update to NDI Version 4.0
			   See further : ofxNDIreceive.cpp
	04.12.19 - Revise for ARM port (https://github.com/IDArnhem/ofxNDI)
			   Cleanup
	29.02.20 - Change GetFps from double to int
	30.10.21 - Add SetSenderName
	02.12.21 - Use setFromPixels in Receive to ofPixels
			   Ensure the NDI video buffer is freed on fail in GetPixelData()

*/
#include "ofxNDIreceiver.h"

ofxNDIreceiver::ofxNDIreceiver()
{

}

ofxNDIreceiver::~ofxNDIreceiver()
{
	
}

// Create a receiver
bool ofxNDIreceiver::CreateReceiver(int userindex)
{
	return NDIreceiver.CreateReceiver(userindex);
}

// Create a receiver with preferred colour format
bool ofxNDIreceiver::CreateReceiver(NDIlib_recv_color_format_e color_format, int userindex)
{
	return NDIreceiver.CreateReceiver(color_format, userindex);

}

// Open the receiver to receive
bool ofxNDIreceiver::OpenReceiver()
{
	// Update the NDI sender list to find new senders
	// There is no delay if no new senders are found
	NDIreceiver.FindSenders();
	// Check the sender count
	int nSenders = GetSenderCount();
	if (nSenders > 0) {

		// Has the user changed the sender index ?
		if (NDIreceiver.SenderSelected()) {
			// Retain the last sender in case of network delay
			// Wait for the network to come back up or for the
			// user to select another sender when it does
			if (nSenders == 1)
				return false;
			// Release the current receiver.
			// A new one is then created from the selected sender index.
			NDIreceiver.ReleaseReceiver();
			return false;
		}

		// Receiver already created
		if (NDIreceiver.ReceiverCreated())
			return true;

		// Create a new receiver if one does not exist.
		// A receiver is created from an index into a list of sender names.
		// The current user selected index is saved in the NDIreceiver class
		// and is used to create the receiver unless you specify a particular index.
		// Receiver is created with default preferred format BGRA
		return NDIreceiver.CreateReceiver();

	}

	// No senders
	return false;

}

// Return whether the receiver has been created
bool ofxNDIreceiver::ReceiverCreated()
{
	return NDIreceiver.ReceiverCreated();
}

// Return whether a receiver has connected to a sender
bool ofxNDIreceiver::ReceiverConnected()
{
	return NDIreceiver.ReceiverConnected();
}

// Close receiver and release resources
void ofxNDIreceiver::ReleaseReceiver()
{
	NDIreceiver.ReleaseReceiver();
}

// Receive ofTexture
// Receive ofFbo
// Receive ofImage
// Receive ofPixels
//   Re-allocated to changed sender dimensions
// For false return :
//   Check for metadata using IsMetadata()
//   Use IsAudioFrame() to determine whether audio has been received
//   and GetAudioData to retrieve the sample buffer


// Receive ofTexture
bool ofxNDIreceiver::ReceiveImage(ofTexture &texture)
{
	if (!texture.isAllocated())
		return false;

	// Check for receiver creation
	if (!OpenReceiver())
		return false;

	// Receive a pixel image first
	unsigned int width = (unsigned int)texture.getWidth();
	unsigned int height = (unsigned int)texture.getHeight();

	// LJ DEBUG
	// if (NDIreceiver.ReceiveTest()) {
		// return true;
	// }
	

	if (NDIreceiver.ReceiveImage(width, height)) {

		// Check for changed sender dimensions
		if (width != (unsigned int)texture.getWidth() || height != (unsigned int)texture.getHeight())
			texture.allocate(width, height, GL_RGBA);

		// Get NDI pixel data from the video frame
		return GetPixelData(texture);

	}

	return false;


}


// Receive ofFbo
bool ofxNDIreceiver::ReceiveImage(ofFbo &fbo)
{
	if (!fbo.isAllocated())
		return false;

	// Check for receiver creation
	if (!OpenReceiver())
		return false;

	// Receive a pixel image first
	unsigned int width = (unsigned int)fbo.getWidth();
	unsigned int height = (unsigned int)fbo.getHeight();
	if (NDIreceiver.ReceiveImage(width, height)) {

		// Check for changed sender dimensions
		if (width != (unsigned int)fbo.getWidth() || height != (unsigned int)fbo.getHeight())
			fbo.allocate(width, height, GL_RGBA);

		// Get NDI pixel data from the video frame
		return GetPixelData(fbo.getTexture());
	}

	return false;

}

// Receive ofImage
bool ofxNDIreceiver::ReceiveImage(ofImage &image)
{
	if (!image.isAllocated())
		return false;

	// Check for receiver creation
	if (!OpenReceiver())
		return false;

	// Receive a pixel image first
	unsigned int width = (unsigned int)image.getWidth();
	unsigned int height = (unsigned int)image.getHeight();
	if (NDIreceiver.ReceiveImage(width, height)) {

		// Check for changed sender dimensions
		if (width != (unsigned int)image.getWidth() || height != (unsigned int)image.getHeight())
			image.allocate(width, height, OF_IMAGE_COLOR_ALPHA);

		// Get NDI pixel data from the video frame
		return GetPixelData(image.getTexture());
	}

	return false;

}

// Receive ofPixels
bool ofxNDIreceiver::ReceiveImage(ofPixels &buffer)
{
	if (!buffer.isAllocated())
		return false;

	// Check for receiver creation
	if (!OpenReceiver())
		return false;

	unsigned int width = (unsigned int)buffer.getWidth();
	unsigned int height = (unsigned int)buffer.getHeight();

	// Receive a pixel image first
	if (NDIreceiver.ReceiveImage(width, height)) {

		// Check for changed sender dimensions
		if (width != (unsigned int)buffer.getWidth() || height != (unsigned int)buffer.getHeight())
			buffer.allocate(width, height, OF_IMAGE_COLOR_ALPHA);

		// Get the video frame buffer pointer
		unsigned char *videoData = NDIreceiver.GetVideoData();
		if (!videoData) {
			// Ensure the video buffer is freed
			NDIreceiver.FreeVideoData();
			return false;
		}

		// Get the NDI frame pixel data into the pixel buffer
		switch (NDIreceiver.GetVideoType()) {
			// Note : the receiver is set up to prefer BGRA format by default
			// YCbCr - RGBA conversion not supported
			case NDIlib_FourCC_type_UYVY: // YCbCr using 4:2:2
			case NDIlib_FourCC_type_UYVA: // YCbCr using 4:2:2:4
			case NDIlib_FourCC_type_P216: // YCbCr using 4:2:2 in 16bpp
			case NDIlib_FourCC_type_PA16: // YCbCr using 4:2:2:4 in 16bpp
				printf("UYVY/UYVA/P216/PA16 - not supported\n");
				break;
			case NDIlib_FourCC_type_RGBA: // RGBA
			case NDIlib_FourCC_type_RGBX: // RGBX
				// setFromPixels copies between buffers so that the videoData pointer can be freed
				buffer.setFromPixels((unsigned char *)videoData, width, height, OF_PIXELS_RGBA);
				break;
			case NDIlib_FourCC_type_BGRA: // BGRA
			case NDIlib_FourCC_type_BGRX: // BGRX
			default: // BGRA
				buffer.setFromPixels((unsigned char *)videoData, width, height, OF_PIXELS_RGBA);
				buffer.swapRgb();
				break;
		} // end switch received format

		// Free the NDI video buffer
		NDIreceiver.FreeVideoData();

		return true;
	}

	return false;

}

// Receive image pixels to a char buffer
// Retained for compatibility with previous version of ofxNDI
// Return sender width and height
// Test width and height for change with true return
// For false return :
//   Check for metadata using IsMetadata()
//   Use IsAudioFrame() to determine whether audio has been received
//   and GetAudioData to retrieve the sample buffer
bool ofxNDIreceiver::ReceiveImage(unsigned char *pixels,
	unsigned int &width, unsigned int &height, bool bInvert)
{
	if (!pixels)
		return false;

	// Check for receiver creation
	if (!OpenReceiver())
		return false;

	return NDIreceiver.ReceiveImage(pixels, width, height, bInvert);
}

// Create a finder to look for a sources on the network
void ofxNDIreceiver::CreateFinder()
{
	NDIreceiver.CreateFinder();
}

// Release the current finder
void ofxNDIreceiver::ReleaseFinder()
{
	NDIreceiver.ReleaseFinder();
}

// Find all current NDI senders
int ofxNDIreceiver::FindSenders()
{
	return NDIreceiver.FindSenders();
}

// Refresh sender list with the current network snapshot
int ofxNDIreceiver::RefreshSenders(uint32_t timeout)
{
	return NDIreceiver.RefreshSenders(timeout);
}

// Set the sender list index variable
bool ofxNDIreceiver::SetSenderIndex(int index)
{
	return NDIreceiver.SetSenderIndex(index);
}

// Return the index of the current sender
int ofxNDIreceiver::GetSenderIndex()
{
	return NDIreceiver.GetSenderIndex();
}

// Return the index of a sender name
bool ofxNDIreceiver::GetSenderIndex(char *sendername, int &index)
{
	return NDIreceiver.GetSenderIndex(sendername, index);
}

// Has the user changed the sender index ?
bool ofxNDIreceiver::SenderSelected()
{
	return NDIreceiver.SenderSelected();
}

// Return the number of senders
int ofxNDIreceiver::GetSenderCount()
{
	return NDIreceiver.GetSenderCount();
}

// Set a sender name to receive from
void ofxNDIreceiver::SetSenderName(std::string sendername)
{
	NDIreceiver.SetSenderName(sendername);
}

// Return the name string of a sender index
std::string ofxNDIreceiver::GetSenderName(int userindex)
{
	return NDIreceiver.GetSenderName(userindex);
}

// Return the name characters of a sender index
// For back-compatibility only
// Char functions replaced with string versions
bool ofxNDIreceiver::GetSenderName(char *sendername)
{
	// Note : length of user name string is not known
	int index = -1;
	return GetSenderName(sendername, 128, index);
}

bool ofxNDIreceiver::GetSenderName(char *sendername, int index)
{
	// NOTE : length of user name string is not known
	return GetSenderName(sendername, 128, index);
}

bool ofxNDIreceiver::GetSenderName(char *sendername, int maxsize, int userindex)
{
	return NDIreceiver.GetSenderName(sendername, maxsize, userindex);
}

// Return the current sender width
unsigned int ofxNDIreceiver::GetSenderWidth() {
	return NDIreceiver.GetSenderWidth();
}

// Return the current sender height
unsigned int ofxNDIreceiver::GetSenderHeight() {
	return NDIreceiver.GetSenderHeight();
}

// Return current sender frame rate
float ofxNDIreceiver::GetSenderFps()
{
	return NDIreceiver.GetSenderFps();
}


//
// Bandwidth
//
// NDIlib_recv_bandwidth_lowest will provide a medium quality stream that takes almost no bandwidth,
// this is normally of about 640 pixels in size on it is longest side and is a progressive video stream.
// NDIlib_recv_bandwidth_highest will result in the same stream that is being sent from the up-stream source
//
void ofxNDIreceiver::SetLowBandwidth(bool bLow)
{
	NDIreceiver.SetLowBandwidth(bLow);
}

// Return the received frame type.
// Note that "allow_video_fields" is currently set false when
// the receiver is created, so all video received will be progressive
NDIlib_frame_type_e ofxNDIreceiver::GetFrameType()
{
	return NDIreceiver.GetFrameType();
}

// Is the current frame MetaData ?
bool ofxNDIreceiver::IsMetadata()
{
	return NDIreceiver.IsMetadata();
}

// Return the current MetaData string
std::string ofxNDIreceiver::GetMetadataString()
{
	return NDIreceiver.GetMetadataString();
}

// Is the current frame Audio ?
bool ofxNDIreceiver::IsAudioFrame()
{
	return NDIreceiver.IsAudioFrame();
}

// Return the current audio frame data
// output - the audio data pointer
void ofxNDIreceiver::GetAudioData(float *&output, int &samplerate, int &samples, int &nChannels)
{
	NDIreceiver.GetAudioData(output, samplerate, samples, nChannels);
}

// Return the NDI dll version number
std::string ofxNDIreceiver::GetNDIversion()
{
	return NDIreceiver.GetNDIversion();
}

// Return the received frame rate
int ofxNDIreceiver::GetFps()
{
	return NDIreceiver.GetFps();
}


//
// Private functions
//

// Get NDI pixel data from the video frame to ofTexture 
bool ofxNDIreceiver::GetPixelData(ofTexture &texture)
{
	// Get the video frame buffer pointer
	unsigned char *videoData = NDIreceiver.GetVideoData();
	if (!videoData) {
		// Ensure the video buffer is freed
		NDIreceiver.FreeVideoData();
		return false;
	}

	// Get the NDI video frame pixel data into the texture
	switch (NDIreceiver.GetVideoType()) {
		// Note : the receiver is set up to prefer BGRA format by default
		// YCbCr - RGBA conversion not supported
	case NDIlib_FourCC_type_UYVY: // YCbCr using 4:2:2
	case NDIlib_FourCC_type_UYVA: // YCbCr using 4:2:2:4
	case NDIlib_FourCC_type_P216: // YCbCr using 4:2:2 in 16bpp
	case NDIlib_FourCC_type_PA16: // YCbCr using 4:2:2:4 in 16bpp
		printf("UYVY/UYVA/P216/PA16 - not supported\n");
		break;
	case NDIlib_FourCC_type_RGBX: // RGBX
	case NDIlib_FourCC_type_RGBA: // RGBA
		texture.loadData((const unsigned char *)videoData, (int)texture.getWidth(), (int)texture.getHeight(), GL_RGBA);
		break;
	case NDIlib_FourCC_type_BGRX: // BGRX
	case NDIlib_FourCC_type_BGRA: // BGRA
	default: // BGRA
		texture.loadData((const unsigned char *)videoData, (int)texture.getWidth(), (int)texture.getHeight(), GL_BGRA);
		break;
	} // end switch received format

	// Free the NDI video buffer
	NDIreceiver.FreeVideoData();

	return true;

}
