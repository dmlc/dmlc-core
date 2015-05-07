UNITTEST=unittest/dmlc_unittest
UNITTEST_SRC=unittest/unittest_config.cc
UNITTEST_OBJ=$(patsubst %.cc,%.o,$(UNITTEST_SRC)) unittest/unittest_main.o

GTEST_LIB=$(GTEST_PATH)/lib/
GTEST_INC=$(GTEST_PATH)/include

unittest/%.o : unittest/%.cc libdmlc.a
	$(CXX) -std=c++11 $(CFLAGS) -I$(GTEST_INC) -o $@ -c $<

$(UNITTEST) : $(UNITTEST_OBJ)
	$(CXX) -std=c++11 $(CFLAGS) -L$(GTEST_LIB) -o $@ $^ libdmlc.a $(LDFLAGS) -lgtest 
