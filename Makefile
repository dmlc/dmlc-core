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
export CFLAGS = -O3 -Wall -msse2  -Wno-unknown-pragmas -fPIC -Iinclude
LDFLAGS+= $(DMLC_LDFLAGS)
CFLAGS+= $(DMLC_CFLAGS)

.PHONY: clean all test

OBJ=line_split.o recordio_split.o input_split_base.o io.o local_filesys.o data.o recordio.o

ifeq ($(USE_HDFS), 1)
	OBJ += hdfs_filesys.o
endif

ifeq ($(USE_S3), 1)
	OBJ += s3_filesys.o
endif


ALIB=libdmlc.a
all: $(ALIB) test

include test/dmlc_test.mk
test: $(TEST)

line_split.o: src/io/line_split.cc
recordio_split.o: src/io/recordio_split.cc
input_split_base.o: src/io/input_split_base.cc
hdfs_filesys.o: src/io/hdfs_filesys.cc
s3_filesys.o: src/io/s3_filesys.cc
local_filesys.o: src/io/local_filesys.cc
io.o: src/io.cc
data.o: src/data.cc
recordio.o: src/recordio.cc

libdmlc.a: $(OBJ)


$(BIN) :
	$(CXX) $(CFLAGS) -o $@ $(filter %.cpp %.o %.c %.cc %.a,  $^) $(LDFLAGS)

$(OBJ) :
	$(CXX) -c $(CFLAGS) -o $@ $(firstword $(filter %.cpp %.c %.cc, $^) )

$(ALIB):
	ar cr $@ $+

clean:
	$(RM) $(OBJ) $(BIN) $(ALIB) $(TEST) *~ src/*~ src/*/*~ include/dmlc/*~ test/*~
