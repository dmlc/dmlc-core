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

# this is the common build script for dmlc lib
export LDFLAGS= -pthread -lm
export CFLAGS = -O3 -Wall -msse2  -Wno-unknown-pragmas -Iinclude -std=c++11
LDFLAGS+= $(DMLC_LDFLAGS)
CFLAGS+= $(DMLC_CFLAGS) -I./include

ifdef DEPS_PATH
CFLAGS+= -I$(DEPS_PATH)/include 
endif

.PHONY: clean all test

OBJ=line_split.o recordio_split.o input_split_base.o io.o local_filesys.o data.o recordio.o config.o


ifeq ($(USE_HDFS), 1)
	OBJ += hdfs_filesys.o
	LDFLAGS += -lhdfs
endif

ifeq ($(USE_S3), 1)
	OBJ += s3_filesys.o
endif


ALIB=libdmlc.a
all: $(ALIB) test wrapper/libdmlc_wrapper.so

include test/dmlc_test.mk

ifeq ($(BUILD_TEST), 1)
test: $(ALL_TEST)
endif



libdmlc.a: $(OBJ)
wrapper/libdmlc_wrapper.so: wrapper/libdmlc_wrapper.o libdmlc.a
	$(CXX) $(CFLAGS) -shared -o $@ $^ $(LDFLAGS)

wrapper/libdmlc_wrapper.o: wrapper/dmlc_wrapper.cc
	$(CXX) -c $(DMLC_CFLAGS) -I./include -o $@ $^

$(BIN) :
	$(CXX) $(CFLAGS) -o $@ $(filter %.cpp %.o %.c %.cc %.a,  $^) $(LDFLAGS)

$(OBJ) :
	$(CXX) -c $(CFLAGS) -o $@ $(firstword $(filter %.cpp %.c %.cc, $^) )

$(ALIB):
	ar cr $@ $+

clean:
	$(RM) $(OBJ) $(BIN) $(ALIB) $(ALL_TEST) $(ALL_TEST_OBJ) *~ src/*~ src/*/*~ include/dmlc/*~ test/*~
