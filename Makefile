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
LDFLAGS+= $(WORMHOLE_LDFLAGS)
CFLAGS+= $(WORMHOLE_CFLAGS)

.PHONY: clean all test

OBJ=line_split.o io.o
ALIB=libwormhole.a
TEST=test/logging_test

all: $(ALIB)
test: $(TEST)

line_split.o: src/io/line_split.cc
io.o: src/io.cc
test/logging_test: test/logging_test.cc

libwormhole.a: $(OBJ)


$(TEST) : libwormhole.a
	$(CXX) $(CFLAGS) -o $@ $(filter %.cpp %.o %.c %.cc %.a,  $^) $(LDFLAGS)

$(BIN) :
	$(CXX) $(CFLAGS) -o $@ $(filter %.cpp %.o %.c %.cc %.a,  $^) $(LDFLAGS)

$(OBJ) :
	$(CXX) -c $(CFLAGS) -o $@ $(firstword $(filter %.cpp %.c %.cc, $^) )

$(ALIB):
	ar cr $@ $+

clean:
	$(RM) $(OBJ) $(BIN)  *~ ../src/*~
