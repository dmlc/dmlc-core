#---------------------------------------------------------------------------------------
#  mshadow configuration script
#
#  include wormhole.mk after the variables are set
#
#  Add WORMHOLE_CFLAGS to the compile flags
#  Add WORMHOLE_LDFLAGS to the linker flags
#----------------------------------------------------------------------------------------
ifndef LIBJVM
	LIBJVM=$(JAVA_HOME)/jre/lib/amd64/server
endif

# setup HDFS
ifeq ($(USE_HDFS),1)
	WORMHOLE_CFLAGS+= -DDMLC_USE_HDFS=1 -I$(HADOOP_HDFS_HOME)/include -I$(JAVA_HOME)/include
	WORMHOLE_LDFLAGS+= -L$(HADOOP_HDFS_HOME)/lib/native -L$(LIBJVM) -lhdfs -ljvm
else
	WORMHOLE_CFLAGS+= -DDMLC_USE_HDFS=0
endif
