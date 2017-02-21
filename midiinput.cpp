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

#include "midiinput.h"

using namespace std;

#if defined(__WINDOWS_MM__)
SOCKET MidiInput::fd;
#endif

MidiInput::MidiInput(int p, int op) {
    try {
        midiIn = new RtMidiIn();
    } catch(RtMidiError &error) {
        error.printMessage();
    }

#if defined(__WINDOWS_MM__)
    WSADATA wsa;
    printf("\nInitialising Winsock...");
    if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
      {
        printf("Failed. Error Code : %d",WSAGetLastError());
        exit(EXIT_FAILURE);
    }

    printf("\nInitialized. starting...");
    if((MidiInput::fd = socket(AF_INET, SOCK_DGRAM, 0)) == SOCKET_ERROR){
      printf("socket() failed with error code : %d" , WSAGetLastError());
    }
    printf("started");
#endif

    string portName;
    string* outputPort;

    stringstream ss;
    ss << op;
    outputPort = new string(ss.str());

    midiIn->openPort(p);
    portName = midiIn->getPortName(p);
    stringReplace(&portName, '_');

    threadData.portName = portName;
    threadData.outputPort = *outputPort;

    midiIn->setCallback(onMidi, &threadData);
    //respond to sysex and timing, ignore active sensing
    midiIn->ignoreTypes(false, false, true);
    cout << "Listening to port " << midiIn->getPortName(p) << endl;
}

MidiInput::~MidiInput() {
    delete this->midiIn;

#if defined(__WINDOWS_MM__)
    closesocket(MidiInput::fd);
    WSACleanup();
#endif
}

void MidiInput::stringReplace(string* str, char rep) {
    replace_if(str->begin(), str->end(), bind2nd(equal_to<char>(), ' '), rep);
}

#if defined(__WINDOWS_MM__)

void MidiInput::sendData(string message_type, vector<unsigned char> *message, MidiThreadData *data, string path, int bytes, unsigned char channel) {
  char buffer[1024];

  int len = 0;

  if(bytes == 0) {
    len = tosc_writeMessage(buffer, sizeof(buffer), path.c_str(), "s", message_type.c_str());
  }
  else if (bytes == 2) {
    len = tosc_writeMessage(buffer, sizeof(buffer), path.c_str(), "sii", message_type.c_str(), (int) message->at(1), (int) message->at(2));
  }

  struct sockaddr_in addr;
  memset((char *) &addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(9000);
  addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

  if (sendto(MidiInput::fd, buffer, len, 0, (struct sockaddr *) &addr, sizeof(addr)) == SOCKET_ERROR)
  {
    printf("sendto() failed with error code : %d" , WSAGetLastError());
  }
}

#else

void MidiInput::sendData(string message_type, vector<unsigned char> *message, MidiThreadData *data, string path, int bytes, unsigned char channel) {
  int j = 0;
  lo_message m = lo_message_new();
  lo_message_add_string(m, message_type.c_str());

  for(j = 0; j < bytes; j++) {
    lo_message_add_int32(m, (int)message->at(j + 1));
  }

  lo_address t = lo_address_new("localhost", data->outputPort.c_str());
  lo_send_message(t, path.c_str(), m);
  lo_address_free(t);
  lo_message_free(m);
}

#endif

void MidiInput::onMidi(double deltatime, vector<unsigned char> *message, void *userData) {
    unsigned int nBytes = 0;
    unsigned char channel = 0, status = 0;
    string message_type;
    int bytes = 0;

    try {
        nBytes = message->size();
    } catch(RtMidiError &error) {
        error.printMessage();
        exit(EXIT_FAILURE);
    }

    if((message->at(0) & 0xf0) != 0xf0) {
        channel = message->at(0) & 0x0f;
        status = message->at(0) & 0xf0;
    } else {
        channel = 0;
        status = message->at(0);
    }

    MidiThreadData *data = static_cast<MidiThreadData*>(userData);
    switch(status) {
        case NOTE_OFF:
            message_type = "note_off";
            bytes = 2;
            break;

        case NOTE_ON:
            message_type = "note_on";
            bytes = 2;
            break;

        case KEY_PRESSURE:
            message_type = "key_pressure";
            bytes = 2;
            break;

        case CONTROLLER_CHANGE:
            message_type = "controller_change";
            bytes = 2;
            break;

        case PROGRAM_CHANGE:
            message_type = "program_change";
            bytes = 2;
            break;

        case CHANNEL_PRESSURE:
            message_type = "channel_pressure";
            bytes = 2;
            break;

        case PITCH_BEND:
            message_type = "pitch_bend";
            bytes = 2;
            break;

        case SYSTEM_EXCLUSIVE:
            if(message->size() == 6) {
                unsigned int type = message->at(4);
                if(type == 1) {
                    message_type = "mmc_stop";
                } else if(type == 2) {
                    message_type = "mmc_play";
                } else if(type == 4) {
                    message_type = "mmc_fast_forward";
                } else if(type == 5) {
                    message_type = "mmc_rewind";
                } else if(type == 6) {
                    message_type = "mmc_record";
                } else if(type == 9) {
                    message_type = "mmc_pause";
                }
            }
            bytes = 0;
            break;

        case SONG_POSITION:
            message_type = "song_position";
            bytes = 2;
            break;

        case SONG_SELECT:
            message_type = "song_select";
            bytes = 2;
            break;

        case TUNE_REQUEST:
            message_type = "tune_request";
            bytes = 2;
            break;

        case TIMING_TICK:
            message_type = "timing_tick";
            bytes = 0;
            break;

        case START_SONG:
            message_type = "start_song";
            bytes = 0;
            break;

        case CONTINUE_SONG:
            message_type = "continue_song";
            bytes = 0;
            break;

        case STOP_SONG:
            message_type = "stop_song";
            bytes = 0;
            break;

        default:
            message_type = "";
            bytes = 0;
            break;
    }

    if(status == NOTE_ON && message->at(2) == 0) {
        message_type = "note_off";
    }

    stringstream path;
    path << "/midi/" << data->portName;
    if(bytes > 0) {
      path << "/" << (int)channel;
    }

    sendData(message_type, message, data, path.str(), bytes, channel);
}
