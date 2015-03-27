dmlc
====
A common code-base for Distributed Machine Learning in C++

NOTE
====
Think about minimum dependency with minimum amount of code
* It is OK to depend on thirdparty library with optional compilation
  - The libraries can be disabled when 
* C++11, used with care in base code, guard with macro ```__cplusplus >= 201103L```
* Put interface headers in include
  - Put internal headers in src
  