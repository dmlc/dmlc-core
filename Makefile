ifneq ("$(wildcard ./config.mk)","")
	config = config.mk
else
	config = make/config.mk
endif
# use customized config file
include $(config)
include make/wormhole.mk

# this is the common build script for wormhole lib
export LDFLAGS= -pthread -lm
export CFLAGS = -Wall  -msse2  -Wno-unknown-pragmas -fPIC -Iinclude 
LDFLAGS+= $(WORMHOLE_LDFLAGS) -lcurl 
CFLAGS+= $(WORMHOLE_CFLAGS)

.PHONY: clean all test

OBJ=line_split.o io.o hdfs_filesys.o local_filesys.o s3_filesys.o
ALIB=libwormhole.a
TEST=test/logging_test test/filesys_test

all: $(ALIB) $(TEST)
test: $(TEST)

line_split.o: src/io/line_split.cc
hdfs_filesys.o: src/io/hdfs_filesys.cc
s3_filesys.o: src/io/s3_filesys.cc
local_filesys.o: src/io/local_filesys.cc
io.o: src/io.cc

test/logging_test: test/logging_test.cc
test/filesys_test: test/filesys_test.cc src/io/*.h  libwormhole.a

libwormhole.a: $(OBJ)

$(TEST) : 
	$(CXX) $(CFLAGS) -o $@ $(filter %.cpp %.o %.c %.cc %.a,  $^) $(LDFLAGS)

$(BIN) :
	$(CXX) $(CFLAGS) -o $@ $(filter %.cpp %.o %.c %.cc %.a,  $^) $(LDFLAGS)

$(OBJ) :
	$(CXX) -c $(CFLAGS) -o $@ $(firstword $(filter %.cpp %.c %.cc, $^) )

$(ALIB):
	ar cr $@ $+

clean:
	$(RM) $(OBJ) $(BIN)  *~ src/*~ src/*/*~

