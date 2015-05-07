UNITTEST=unittest/dmlc_unittest
UNITTEST_SRC=unittest/unittest_config.cc
UNITTEST_OBJ=$(patsubst %.cc,%.o,$(UNITTEST_SRC))

DEP=libdmlc.a
GTEST_LIB=$(GTEST_PATH)/lib/
GTEST_INC=$(GTEST_PATH)/include

unittest/unittest_main.o: unittest/unittest_main.cc
	$(CXX) -L$(GTEST_LIB) -I$(GTEST_INC) -o unittest_main.o -c unittest/unittest_main.cc -lgtest

unittest/unittest_config.o : unittest/unittest_config.cc
	$(CXX) $(CFLAGS) -I$(GTEST_INC) -o $@ -c $<

$(UNITTEST) : unittest/unittest_main.o $(UNITTEST_OBJ)
	$(CXX) $(CFLAGS) -L$(GTEST_LIB) -o $@ $^ $(LDFLAGS) -lgtest 
