import os, sys

libs = []
sources = ['main.cpp', 'midiinput.cpp', 'RtMidi.cpp', 'anyoption.cpp', 'options.cpp']

if sys.platform == 'darwin':
    env = Environment(
        FRAMEWORKS = ['CoreMidi', 'CoreAudio', 'CoreFoundation'],
        CCFLAGS = '-D__MACOSX_CORE__'
    )
    libs.append(['lo'])
elif sys.platform == 'win32':
    env = Environment(
        CCFLAGS = '-D__WINDOWS_MM__',
    )
    libs.append(['ws2_32', 'winmm'])
    sources.append(['tinyosc.c'])
else:
    env = Environment(        
        CCFLAGS = '-D__LINUX_ALSASEQ__'
    )
    libs.append(['lo', 'asound', 'pthread'])

env.Program('midiosc', sources, LIBS=libs)
