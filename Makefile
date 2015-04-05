ifneq ("$(wildcard ./config.mk)","")
	config = config.mk
else
	config = make/config.mk
endif
# use customized config file
include $(config)
include make/dmlc.mk

# this is the common build script for dmlc lib
export LDFLAGS= -pthread -lm
export CFLAGS = -Wall  -msse2  -Wno-unknown-pragmas -fPIC -Iinclude
LDFLAGS+= $(DMLC_LDFLAGS)
CFLAGS+= $(DMLC_CFLAGS)

.PHONY: clean all test

OBJ=line_split.o io.o local_filesys.o s3_filesys.o data.o

# TODO move to make/dmlc.mk?
ifeq ($(USE_HDFS), 1)
OBJ+=hdfs_filesys.o
endif

ALIB=libdmlc.a
TEST=test/logging_test test/filesys_test test/dataiter_test

all: $(ALIB) $(TEST)
test: $(TEST)

line_split.o: src/io/line_split.cc
hdfs_filesys.o: src/io/hdfs_filesys.cc
s3_filesys.o: src/io/s3_filesys.cc
local_filesys.o: src/io/local_filesys.cc
io.o: src/io.cc
data.o: src/data.cc

test/logging_test: test/logging_test.cc
test/filesys_test: test/filesys_test.cc src/io/*.h libdmlc.a
test/dataiter_test: test/dataiter_test.cc  libdmlc.a

libdmlc.a: $(OBJ)

$(TEST) :
	$(CXX) $(CFLAGS) -o $@ $(filter %.cpp %.o %.c %.cc %.a,  $^) $(LDFLAGS)

$(BIN) :
	$(CXX) $(CFLAGS) -o $@ $(filter %.cpp %.o %.c %.cc %.a,  $^) $(LDFLAGS)

$(OBJ) :
	$(CXX) -c $(CFLAGS) -o $@ $(firstword $(filter %.cpp %.c %.cc, $^) )

$(ALIB):
	ar cr $@ $+

clean:
	$(RM) $(OBJ) $(BIN) $(ALIB)  *~ src/*~ src/*/*~
