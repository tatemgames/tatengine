#-------------------------------------------------
#
# TatEngine template for qmake and mingw
#
#-------------------------------------------------

TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle qt

TARGET = %%%TEMPLATE_NAME%%%

DESTDIR = ../../../Bin/

# todo : disable only stupid warnings
QMAKE_CXXFLAGS_WARN_ON = -w
QMAKE_CFLAGS_WARN_ON = -w

win32::CONFIG -= x86_64
win32::CONFIG += x86
win32::DEFINES += TE_PLATFORM_WIN __CYGWIN32__
win32::LIBS += -L../../../Bin/ -lfmodex
win32::LIBS += libopengl32 libgdi32

macx::CONFIG += x86
macx::CONFIG -= ppc x86_64 ppc64
macx::DEFINES += TE_PLATFORM_MAC
macx::LIBS += -framework Cocoa
