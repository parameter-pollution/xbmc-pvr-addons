#
# Makefile for the PVR IPTV Simple add-on for XBMC PVR
#
# See the README for copyright information and
# how to reach the author.
#

ADDONBINNAME    = XBMC_A1TV
ADDONNAME       = pvr.a1tv
LIBNAME         = liba1tv-addon
lib_LTLIBRARIES = liba1tv-addon.la

LIBS            = @abs_top_srcdir@/lib/jsoncpp/libjsoncpp.la

include ../Makefile.include.am

INCLUDES+=-Isrc -Isrc/lib/filesystem -I@abs_top_srcdir@/lib/jsoncpp/include

liba1tv_addon_la_SOURCES = src/client.cpp \
                           src/A1TVData.cpp
liba1tv_addon_la_LDFLAGS = $(ZLIB_LIBS) @TARGET_LDFLAGS@






