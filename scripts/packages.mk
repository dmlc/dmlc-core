# Makfile for easily install dependencies

# List of packages here
.PHONY: gtest

# rules for gtest
${CACHE_PREFIX}/include/gtest:
	rm -rf gtest-1.7.0.zip gtest-1.7.0
	wget http://googletest.googlecode.com/files/gtest-1.7.0.zip
	unzip gtest-1.7.0.zip
	cd gtest-1.7.0 ; ./configure; make; cd -
	cp -r gtest-1.7.0/include/gtest ${CACHE_PREFIX}/include
	cp -r gtest-1.7.0/lib/.libs/* ${CACHE_PREFIX}/lib
	rm ${CACHE_PREFIX}/lib/libgtest_main.la ${CACHE_PREFIX}/lib/libgtest.la
	cp -f gtest-1.7.0/lib/*.la ${CACHE_PREFIX}/lib
	rm -rf gtest-1.7.0.zip

gtest: | ${CACHE_PREFIX}/include/gtest
