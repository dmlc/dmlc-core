# Makfile for easily install dependencies

# List of packages here
.PHONY: gtest

ifndef CACHE_PREFIX
	CACHE_PREFIX = ${HOME}/.cache/usr
endif

# rules for gtest
${CACHE_PREFIX}/include/gtest:
    rm -rf gtest-1.7.0.zip gtest-1.7.0
    wget http://googletest.googlecode.com/files/gtest-1.7.0.zip
    unzip gtest-1.7.0.zip
    cd gtest-1.7.0
    ./configure
    make
    cp -r include/gtest ${CACHE_PREFIX}/include
    cp -r lib/.libs/* ${CACHE_PREFIX}/lib
    cp -f lib/*.la ${CACHE_PREFIX}/lib
    cd ..
    rm -rf gtest-1.7.0.zip

gtest: | ${CACHE_PREFIX}/include/gtest
