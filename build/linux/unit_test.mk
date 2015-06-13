# Project parameters
###########################################################

PROJECT		:= unit_test
TARGET		:= unit_test
ROOTDIR		:= ../..
OUTDIR		:= $(ROOTDIR)/bin/$(PROJECT)
INCDIR		:= $(ROOTDIR)/src/
SRCDIR		:= $(ROOTDIR)/src
LIBDIR		:= $(ROOTDIR)/bin/lib
DOCDIR		:= $(ROOTDIR)/docs
INSTLIBDIR	:=
INSTINCDIR	:=
INSTALLDIR	:=
DEBUG		:= YES
LIBRARY		:= NO
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
	-llog4cplus \
	-levent_core \
	-lprotobuf \
	-lrt

#ifeq (YES, $(DEBUG))
#	LIBS	+= \
#	-lelfox_d \
#	-lpb_d
#else
#	LIBS	+= \
#	-lcjson \
#	-lelfox \
#	-lpb
#endif

LDFLAGS		:= \
	-L$(LIBDIR) \
	$(LIBS)

SRCDIRS		:= $(ROOTDIR)/src/$(PROJECT) $(ROOTDIR)/src/common
SRCS_C_EXCLUDE_FILTER 	:=
SRCS_CPP_EXCLUDE_FILTER	:=

include common.mk

