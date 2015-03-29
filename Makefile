ifneq ("$(wildcard ./config.mk)","")
	config = config.mk
else
	config = make/config.mk
endif
# use customized config file
include $(config)

# this is the common build script for wormhole lib
export LDFLAGS= -pthread -lm
export CFLAGS = -Wall  -msse2  -Wno-unknown-pragmas -fPIC

# setup opencv
ifeq ($(USE_HDFS),1)
	CFLAGS+= -DDMLC_USE_HDFS=1 -I$(HADOOP_HDFS_HOME)/include -I$(JAVA_HOME)/include
	LDFLAGS+= -L$(HADOOP_HDFS_HOME)/lib/native -L$(LIBJVM) -lhdfs -ljvm
else
	CFLAGS+= -DDMLC_USE_HDFS=0
endif

.PHONY: clean all

OBJ=line_split.o io.o
ALIB=libwormhole.a
TEST=logging_test

all: $(ALIB)

line_split.o: src/io/line_split.cc
io.o: src/io.cc
logging_test: src/test/logging_test.cc

libwormhole.a: $(OBJ)

$(TEST) :
	$(CXX) $(CFLAGS) -o $@ $(filter %.cpp %.o %.c %.cc,  $^) $(LDFLAGS)

$(BIN) :
	$(CXX) $(CFLAGS) -o $@ $(filter %.cpp %.o %.c %.cc,  $^) -lrabit $(LDFLAGS)

$(OBJ) :
	$(CXX) -c $(CFLAGS) -o $@ $(firstword $(filter %.cpp %.c %.cc, $^) )

$(ALIB):
	ar cr $@ $+

clean:
	$(RM) $(OBJ) $(BIN)  *~ ../src/*~
