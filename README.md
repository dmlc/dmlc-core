DMLC Core
====

[![Build Status](https://travis-ci.org/dmlc/dmlc-core.svg?branch=master)](https://travis-ci.org/dmlc/dmlc-core)

A common code-base for Distributed Machine Learning in C++.

Developer Channel [![Join the chat at https://gitter.im/dmlc/dmlc-core](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/dmlc/dmlc-core?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)



Contributing
====
Contributing to dmlc-core is welcomed! dmlc-core follows google's C style guide. If you are interested in contributing, take a look at [feature wishlist](https://github.com/dmlc/dmlc-core/labels/feature%20wishlist) and open a new issue if you like to add something.

* Use of c++11 is allowed, given that the code is macro guarded with ```DMLC_USE_CXX11```
* Try to introduce minimum dependency when possible

CheckList before submit code
=====
* Type ```make lint``` and fix all the style problems.
* Type ```make doc``` and fix all the warnings.

NOTE
====
deps:

libcurl4-openssl-dev
