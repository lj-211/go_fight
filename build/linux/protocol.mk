# Project parameters
###########################################################

PROJECT		:= protocol
TARGET		:= protocol
ROOTDIR		:= ../..
OUTDIR		:= $(ROOTDIR)/bin/lib
INCDIR		:= $(ROOTDIR)/src/
SRCDIR		:= $(ROOTDIR)/src
LIBDIR		:= $(ROOTDIR)/bin/lib
DOCDIR		:= $(ROOTDIR)/docs
INSTLIBDIR	:=
INSTINCDIR	:=
INSTALLDIR	:=
DEBUG		:= YES
LIBRARY		:= YES
PROFILE		:= NO
CFLAGS		:= \
	-I$(ROOTDIR)/include \
	-I$(ROOTDIR)/include/libevent/include \
	-I$(ROOTDIR)/include/libevent \
	-I$(INCDIR)/$(PROJECT) \
	-I$(ROOTDIR)/src/common
CPPFLAGS	:= \
	-DELF_HAVE_PRAGMA_ONCE \
	-DELF_USE_ALL
LIBS		:= \
	-lprotobuf \
	-lrt
STATIC_LIBS := 

LDFLAGS		:= \
	-L/usr/lib64/mysql \
	-L$(LIBDIR) \
	$(LIBS)

SRCDIRS		:= $(ROOTDIR)/src/$(PROJECT)
SRCS_C_EXCLUDE_FILTER 	:=
SRCS_CPP_EXCLUDE_FILTER	:=

include common.mk

