# TODO

*file* read & write file on local disk, hdfs, s3... refer to

https://github.com/mli/parameter_server/blob/master/src/util/file.h
https://github.com/tqchen/rabit/blob/master/include/rabit_serializable.h#L18

either is fine.

*input data format:* libsvm is the simplest format.

https://github.com/mli/parameter_server/wiki/Data

supports binary sparse format and multiple slots. VW format is another option.

https://github.com/JohnLangford/vowpal_wabbit/wiki/Input-format

*internal data format* we need an single internal data format (at least for text
data format) to simplify text parsers and data cache. one option is that

https://github.com/mli/parameter_server/blob/master/src/data/proto/example.proto

examples are then can serialized by recordio

https://github.com/mli/parameter_server/blob/master/src/util/recordio.h

*text parser*
ps already implemented some of
them. e.g. https://github.com/mli/parameter_server/blob/master/src/data/stream_reader.h

the key is that =strtoint= is usually the most time consuming function. we need
multi-thread implementation. such as a consumer-producer model.

*concurrent objects* i suggest to use c++11 thread support library, rather than
 build by our own. it brings dependencies on new version compilers, but it should
 be fine once we use docker

*common data structure* we can use mshadow to replace eigen3
 for most use cases. (eigen3 may has a more complete lapack
 implementation.) PS uses a shared array heavily to get ride of memory copy.

https://github.com/mli/parameter_server/blob/master/src/util/shared_array.h

 it also has a simple but efficient multi-thread implementation of sparse
 matrices. but we don't need them at this moment.

https://github.com/mli/parameter_server/blob/master/src/util/sparse_matrix.h
https://github.com/mli/parameter_server/blob/dev/src/util/localizer.h

*common object*
 we need some common representation of objects which may be communicated over
 network under /src/proto/. for example:

 data:
 https://github.com/mli/parameter_server/blob/dev/src/data/proto/data.proto
 node:
https://github.com/mli/parameter_server/blob/dev/src/system/proto/node.proto
 workload:
 https://github.com/mli/parameter_server/blob/dev/src/learner/proto/workload.proto

*third libraries*

i suggest use protobuf for data structures that passed between functions and
processes, gtest for unit testing, gflags for logging (CHECK, VLOG are more
power than the ones used by mshadow). they are lightweight and easy to build
from sources.
