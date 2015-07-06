# Project parameters
###########################################################

PROJECT		:= gate
TARGET		:= gate.process
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
	-lmysqlclient_r \
	-lhiredis \
	-llua \
	-lrt
STATIC_LIBS := $(ROOTDIR)/lib/liblua.a

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
	-L/usr/lib64/mysql \
	-L$(LIBDIR) \
	$(LIBS)

SRCDIRS		:= $(ROOTDIR)/src/$(PROJECT) $(ROOTDIR)/src/common
SRCS_C_EXCLUDE_FILTER 	:=
SRCS_CPP_EXCLUDE_FILTER	:=

include common.mk

