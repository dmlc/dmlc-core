TEST=test/logging_test test/filesys_test test/dataiter_test test/ps_test

test/logging_test: test/logging_test.cc
test/filesys_test: test/filesys_test.cc src/io/*.h libdmlc.a
test/dataiter_test: test/dataiter_test.cc  libdmlc.a
test/ps_test: test/ps_test.cc include/dmlc/ps.h

$(TEST) :
	$(CXX) $(CFLAGS) -o $@ $(filter %.cpp %.o %.c %.cc %.a,  $^) $(LDFLAGS)
