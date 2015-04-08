TEST=test/logging_test test/filesys_test test/dataiter_test test/streambuf_test

test/logging_test: test/logging_test.cc
test/filesys_test: test/filesys_test.cc src/io/*.h libdmlc.a
test/dataiter_test: test/dataiter_test.cc  libdmlc.a
test/streambuf_test: test/streambuf_test.cc libdmlc.a

$(TEST) :
	$(CXX) $(CFLAGS) -o $@ $(filter %.cpp %.o %.c %.cc %.a,  $^) $(LDFLAGS)
