protobuf-matlab - FarSounder's Protobuf compiler for Matlab
Copyright 2011 FarSounder, Inc.
http://code.google.com/p/protobuf-matlab/


Overview
========

This package provides a Matlab code generator for version 2.4.0a of Google's
Protocol Buffers compiler (protoc) as well as support libraries for the
generated Matlab code.


Building protoc with Matlab support
===================================

1. Get the protobuf source code from
   http://protobuf.googlecode.com/svn/tags/2.4.0a

2. Add the src/farsounder/protobuf/compiler/matlab/* files to the protobuf
   project.

3. Replace the src/google/protobuf/compiler/main.cc file with the one
   provided.

4. Compile the modified protobuf project.

This should yield a protoc executable with a --matlab_out option. You can now
use protoc to generate Matlab reading and writing code for your .proto file(s).


Matlab support library setup
============================

In order to use the generated Matlab code, you'll need to add the protobuflib
directory to your Matlab path. protobuflib is a collection of .m utility files
used by the generated code.
