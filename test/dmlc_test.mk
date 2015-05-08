TEST=test/filesys_test test/dataiter_test\
	test/iostream_test test/recordio_test test/split_read_test\
	test/stream_read_test test/split_test test/libsvm_parser_test\
	test/threadediter_test test/split_repeat_read_test test/strtonum_test

test/filesys_test: test/filesys_test.cc src/io/*.h libdmlc.a
test/dataiter_test: test/dataiter_test.cc  libdmlc.a
test/iostream_test: test/iostream_test.cc libdmlc.a
test/recordio_test: test/recordio_test.cc libdmlc.a
test/split_read_test: test/split_read_test.cc libdmlc.a
test/split_repeat_read_test: test/split_repeat_read_test.cc libdmlc.a
test/stream_read_test: test/stream_read_test.cc libdmlc.a
test/split_test: test/split_test.cc libdmlc.a
test/libsvm_parser_test: test/libsvm_parser_test.cc src/data/libsvm_parser.h libdmlc.a
test/threadediter_test: test/threadediter_test.cc libdmlc.a
test/strtonum_test: test/strtonum_test.cc src/data/strtonum.h

$(TEST) :
	$(CXX) -std=c++11 $(CFLAGS) -o $@ $(filter %.cpp %.o %.c %.cc %.a,  $^) $(LDFLAGS)

include test/unittest/dmlc_unittest.mk

ALL_TEST=$(TEST) $(UNITTEST)
ALL_TEST_OBJ=$(UNITTEST_OBJ)
