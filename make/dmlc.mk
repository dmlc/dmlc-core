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

ifndef NO_OPENMP
	DMLC_CFLAGS += -fopenmp 	
	DMLC_LDFLAGS += -fopenmp
endif

ifndef HADOOP_HDFS_HOME
	#Using default hadoop_home
	HADOOP_HDFS_HOME=$(HADOOP_HOME)
endif

ifeq ($(USE_HDFS),1)
	DMLC_CFLAGS+= -DDMLC_USE_HDFS=1 -I$(HADOOP_HDFS_HOME)/include -I$(JAVA_HOME)/include
	ifeq ($(HADOOP_CDH_BINARY), 1)
		DMLC_LDFLAGS+= -L$(LIBJVM) -ljvm -Wl,-rpath=$(LIBJVM)
	else
		DMLC_LDFLAGS+= -L$(HADOOP_HDFS_HOME)/lib/native -L$(LIBJVM) -lhdfs -ljvm -Wl,-rpath=$(LIBJVM) 
	endif
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
