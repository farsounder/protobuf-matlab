# Protobuf Matlab

protobuf-matlab - FarSounder's Protobuf compiler for Matlab
Copyright 2011 FarSounder, Inc.
http://code.google.com/p/protobuf-matlab/


## Overview

This package provides a Matlab code generator for version 2.4.1 of Google's
Protocol Buffers compiler (protoc) as well as support libraries for the
generated Matlab code.


## Building protoc with Matlab support

1. Get the protobuf source:
```
git clone https://github.com/protocolbuffers/protobuf.git
```

2. Get the protobuf-matlab source:
```
git clone https://github.com/farsounder/protobuf-matlab.git
```

3. Add the protobuf-matlab src files to the Google Protobuf src:
```
cp -r protobuf-matlab/{src,cmake} protobuf
```

4. Compile the modified protobuf project:
```
cd protobuf/cmake
mkdir build && cd build
cmake ..
make
```

This should yield a protoc executable with a --matlab_out option. You can now
use protoc to generate Matlab reading and writing code for your .proto file(s).

```
protoc --matlab_out test.proto
```

## Matlab support library setup

In order to use the generated Matlab code, you'll need to add the protobuflib
directory to your Matlab path. protobuflib is a collection of .m utility files
used by the generated code.
