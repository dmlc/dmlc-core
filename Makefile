ifndef config
	ifneq ("$(wildcard ./config.mk)","")
		config = config.mk
	else
		config = make/config.mk
	endif
endif
# use customized config file
include $(config)
include make/dmlc.mk

NOLINT_FILES = --exclude_path include/dmlc/concurrentqueue.h include/dmlc/blockingconcurrentqueue.h

# this is the common build script for dmlc lib
export LDFLAGS= -pthread -lm
export CFLAGS = -O3 -Wall -Wno-unknown-pragmas -Iinclude
CFLAGS+=-std=c++11
LDFLAGS+= $(DMLC_LDFLAGS) $(ADD_LDFLAGS)
CFLAGS+= $(DMLC_CFLAGS) $(ADD_CFLAGS)

ifndef USE_SSE
	USE_SSE = 1
endif

ifeq ($(USE_SSE), 1)
	CFLAGS += -msse2
endif

ifdef DEPS_PATH
CFLAGS+= -I$(DEPS_PATH)/include
LDFLAGS+= -L$(DEPS_PATH)/lib
endif

.PHONY: clean all test doc example

OBJ=line_split.o indexed_recordio_split.o recordio_split.o input_split_base.o io.o filesys.o local_filesys.o data.o recordio.o config.o

ifeq ($(USE_HDFS), 1)
	OBJ += hdfs_filesys.o
endif

ifeq ($(USE_S3), 1)
	OBJ += s3_filesys.o
endif

ifeq ($(USE_AZURE), 1)
	OBJ += azure_filesys.o
endif

ifndef LINT_LANG
	LINT_LANG="all"
endif


ALIB=libdmlc.a
all: $(ALIB)

include example/dmlc_example.mk

ifeq ($(BUILD_TEST), 1)
endif

example: $(ALL_EXAMPLE)

line_split.o: src/io/line_split.cc
recordio_split.o: src/io/recordio_split.cc
indexed_recordio_split.o: src/io/indexed_recordio_split.cc
input_split_base.o: src/io/input_split_base.cc
filesys.o: src/io/filesys.cc
hdfs_filesys.o: src/io/hdfs_filesys.cc
s3_filesys.o: src/io/s3_filesys.cc
azure_filesys.o: src/io/azure_filesys.cc
local_filesys.o: src/io/local_filesys.cc
io.o: src/io.cc
data.o: src/data.cc
recordio.o: src/recordio.cc
config.o: src/config.cc

libdmlc.a: $(OBJ)


$(BIN) :
	$(CXX) $(CFLAGS) -o $@ $(filter %.cpp %.o %.c %.cc %.a,  $^) $(LDFLAGS)

$(OBJ) :
	$(CXX) -c $(CFLAGS) -o $@ $(firstword $(filter %.cpp %.c %.cc, $^) )

$(ALIB):
	$(AR) cr $@ $+

doxygen:
	doxygen doc/Doxyfile

clean:
	$(RM) $(OBJ) $(BIN) $(ALIB) $(ALL_TEST) $(ALL_TEST_OBJ) *~ src/*~ src/*/*~ include/dmlc/*~
