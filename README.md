# protobuf-matlab - FarSounder's Protobuf compiler for Matlab
## Copyright 2011 FarSounder, Inc.

http://code.google.com/p/protobuf-matlab/

**NOTE(Heath - 04/2024):** this is not maintained as we're not really using matlab anymore (we
haven't in a long time). Please feel free to use for whatever you want if it helps you get
protobuf working with matlab. I'm not sure we'll ever update to latest protobuf version as
I don't think I have access to a current matlab license to test any changes. If you're inclined
to PR something, feel free to open and issue first and 'at' me (@heathhenley) or email
sw@farsounder.com to check so you don't waste any effort. 

Overview
========

This package provides a Matlab code generator for version 2.4.1 of Google's
Protocol Buffers compiler (protoc) as well as support libraries for the
generated Matlab code.


Building protoc with Matlab support
===================================

1. Get the protobuf source:
   svn co http://protobuf.googlecode.com/svn/tags/2.4.1 protobuf

2. Get the protobuf-matlab source:
   git clone https://code.google.com/p/protobuf-matlab/

3. Add the protobuf-matlab src files to the Google Protobuf src:
   cp -r protobuf-matlab/src protobuf

4. Compile the modified protobuf project.

This should yield a protoc executable with a --matlab_out option. You can now
use protoc to generate Matlab reading and writing code for your .proto file(s).


Matlab support library setup
============================

In order to use the generated Matlab code, you'll need to add the protobuflib
directory to your Matlab path. protobuflib is a collection of .m utility files
used by the generated code.
