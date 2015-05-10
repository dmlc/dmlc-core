UTEST_ROOT=test/unittest
UNITTEST=$(UTEST_ROOT)/dmlc_unittest
UNITTEST_SRC=$(UTEST_ROOT)/unittest_config.cc $(UTEST_ROOT)/unittest_logging.cc
UNITTEST_OBJ=$(patsubst %.cc,%.o,$(UNITTEST_SRC)) $(UTEST_ROOT)/unittest_main.o

GTEST_LIB=$(GTEST_PATH)/lib/
GTEST_INC=$(GTEST_PATH)/include/

$(UTEST_ROOT)/%.o : $(UTEST_ROOT)/%.cc libdmlc.a
	$(CXX) $(CFLAGS) -I$(GTEST_INC) -o $@ -c $<

$(UNITTEST) : $(UNITTEST_OBJ)
	$(CXX) $(CFLAGS) -L$(GTEST_LIB) -o $@ $^ libdmlc.a $(LDFLAGS) -lgtest 
