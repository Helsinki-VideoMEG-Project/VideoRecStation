# Author: Andrey Zhdanov
# Copyright (C) 2014 BioMag Laboratory, Helsinki University Central Hospital
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, version 3.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

TEMPLATE = app
TARGET = VideoRecStation
QT += core \
    gui \
    widgets
HEADERS += settings.h \
    settingsdialog.h \
    videodialog.h \
    filewriter.h \
    audiocompressorthread.h \
    stoppablethread.h \
    speakerthread.h \
    nonblockingbuffer.h \
    audiofilewriter.h \
    cycdatabuffer.h \
    microphonethread.h \
    videofilewriter.h \
    config.h \
    cameracontroller.h \
    frameobserver.h \
    videowidget.h \
    maindialog.h \
    gpujpegencoder.h \
    videodecompressorthread.h \
    multipleofeightspinbox.h
SOURCES += settings.cpp \
    settingsdialog.cpp \
    videodialog.cpp \
    filewriter.cpp \
    audiocompressorthread.cpp \
    stoppablethread.cpp \
    speakerthread.cpp \
    nonblockingbuffer.cpp \
    audiofilewriter.cpp \
    cycdatabuffer.cpp \
    microphonethread.cpp \
    videofilewriter.cpp \
    cameracontroller.cpp \
    frameobserver.cpp \
    videowidget.cpp \
    main.cpp \
    maindialog.cpp \
    gpujpegencoder.cpp \
    videodecompressorthread.cpp
FORMS += videodialog.ui \
    maindialog.ui \
    settingsdialog.ui
INCLUDEPATH += /usr/include/c++/4.4 \
    /usr/include \
    /opt/VimbaX_2025-3/api/include \
    /opt/GPUJPEG/include
LIBS += -L/usr/local/lib \
    -L/usr/lib \
    -lasound \
    -L/opt/VimbaX_2025-3/api/lib \
    -lVmbCPP \
    -L/opt/GPUJPEG/lib \
    -lgpujpeg
RESOURCES += 
DEFINES += __STDC_LIMIT_MACROS
CONFIG += debug_and_release