// Copyright (C) 2010 Jonny Stutters
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#pragma once
#include <iostream>
#include <algorithm>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <string>
#include <vector>

#include "RtMidi.h"

#if defined(__WINDOWS_MM__)

#include "Ws2tcpip.h"
#include "winsock2.h"
#include "tinyosc.h"

#else

#include "lo/lo.h"

#endif

#include "midimessages.h"
#include "midithreaddata.h"

class MidiInput {
	private:
  
		RtMidiIn* midiIn;
		void stringReplace(std::string*, char);
		
	public:
		MidiInput(int, int);
		~MidiInput();
		MidiThreadData threadData;
		static void onMidi(double, std::vector<unsigned char>*, void*);
    static void sendData(std::string message_type, std::vector<unsigned char> *message, MidiThreadData *data, std::string path, int bytes, unsigned char channel);
#if defined(__WINDOWS_MM__)
    static SOCKET fd;
#endif

};
