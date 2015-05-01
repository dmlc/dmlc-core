#---------------------------------------------------------------------------------------
#  mshadow configuration script
#
#  include dmlc.mk after the variables are set
#
#  Add DMLC_CFLAGS to the compile flags
#  Add DMLC_LDFLAGS to the linker flags
#----------------------------------------------------------------------------------------
ifndef LIBJVM
	LIBJVM=$(JAVA_HOME)/jre/lib/amd64/server
endif

DMLC_LDFLAGS = -lrt

ifndef NO_OPENMP
	DMLC_CFLAGS += -fopenmp 	
	DMLC_LDFLAGS += -fopenmp
endif

#Using default hadoop_home
ifndef HADOOP_HDFS_HOME
	HADOOP_HDFS_HOME=$(HADOOP_HOME)
endif

ifeq ($(USE_HDFS),1)
	DMLC_CFLAGS+= -DDMLC_USE_HDFS=1 -I$(HADOOP_HDFS_HOME)/include -I$(JAVA_HOME)/include
	DMLC_LDFLAGS+= -L$(HADOOP_HDFS_HOME)/lib/native -L$(LIBJVM) -lhdfs -ljvm -Wl,-rpath=$(LIBJVM) 
else
	DMLC_CFLAGS+= -DDMLC_USE_HDFS=0
endif

# setup S3
ifeq ($(USE_S3),1)
	DMLC_CFLAGS+= -DDMLC_USE_S3=1
	DMLC_LDFLAGS+= -lcurl -lssl -lcrypto
else
	DMLC_CFLAGS+= -DDMLC_USE_S3=0
endif
