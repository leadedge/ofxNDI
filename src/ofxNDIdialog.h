/*
	ofxNDIdialog.cpp

	Sender dialog support

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

	16-06-16	- create file

*/
#pragma once
#ifndef __ofxNDIdialog__
#define __ofxNDIdialog__

#include <windows.h>
#include <string>
#include <vector>

// Static variables for the NDI sender selection dialog
static int listindex = 0;
static bool resetlist = false;
static std::vector<std::string> senderlist;
LRESULT CALLBACK NDIsenderDialog(HWND, UINT, WPARAM, LPARAM);


class ofxNDIdialog {

public:

	ofxNDIdialog();
    ~ofxNDIdialog();

	bool SelectNDIPanel(std::vector<std::string> Sendernames, int &senderIndex);
	bool SenderSelected();

private:

	bool bSenderSelected;

};

#endif
