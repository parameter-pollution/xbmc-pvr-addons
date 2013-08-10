#
# Makefile for the PVR demo add-on for XBMC PVR
#
# See the README for copyright information and
# how to reach the author.
#

ADDONBINNAME    = XBMC_A1TV
ADDONNAME       = pvr.a1tv
LIBNAME         = liba1tv-addon
lib_LTLIBRARIES = liba1tv-addon.la

LIBS            = 

include ../Makefile.include.am

liba1tv_addon_la_SOURCES = 	src/client.cpp \
                            src/A1TVData.cpp
liba1tv_addon_la_LDFLAGS = @TARGET_LDFLAGS@

