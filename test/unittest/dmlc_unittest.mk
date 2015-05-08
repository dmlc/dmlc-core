UNITTEST_ROOT=test/unittest
UNITTEST=$(UNITTEST_ROOT)/dmlc_unittest
UNITTEST_SRC=$(UNITTEST_ROOT)/unittest_config.cc
UNITTEST_OBJ=$(patsubst %.cc,%.o,$(UNITTEST_SRC)) $(UNITTEST_ROOT)/unittest_main.o

GTEST_LIB=$(GTEST_PATH)/lib/
GTEST_INC=$(GTEST_PATH)/include/

$(UNITTEST_ROOT)/%.o : $(UNITTEST_ROOT)/%.cc libdmlc.a
	$(CXX) -std=c++11 $(CFLAGS) -I$(GTEST_INC) -o $@ -c $<

$(UNITTEST) : $(UNITTEST_OBJ)
	$(CXX) -std=c++11 $(CFLAGS) -L$(GTEST_LIB) -o $@ $^ libdmlc.a $(LDFLAGS) -lgtest 
