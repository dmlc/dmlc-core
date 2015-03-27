# this is the common build script for rabit programs
# you do not have to use it
export LDFLAGS= -L../../lib -pthread -lm -lrt
export CFLAGS = -Wall  -msse2  -Wno-unknown-pragmas -fPIC -I../../include  

# setup opencv
ifeq ($(USE_HDFS),1)
	CFLAGS+= -DDMLC_USE_HDFS=1 -I$(HADOOP_HDFS_HOME)/include -I$(JAVA_HOME)/include
	LDFLAGS+= -L$(HADOOP_HDFS_HOME)/lib/native -L$(LIBJVM) -lhdfs -ljvm
else
	CFLAGS+= -DDMLC_USE_HDFS=0
endif

.PHONY: clean all

all: $(BIN)

$(BIN) : 
	$(CXX) $(CFLAGS) -o $@ $(filter %.cpp %.o %.c %.cc,  $^) -lrabit $(LDFLAGS) 

$(OBJ) : 
	$(CXX) -c $(CFLAGS) -o $@ $(firstword $(filter %.cpp %.c %.cc, $^) )

clean:
	$(RM) $(OBJ) $(BIN)  *~ ../src/*~
