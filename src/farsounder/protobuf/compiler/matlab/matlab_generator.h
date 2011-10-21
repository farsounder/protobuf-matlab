// protobuf-matlab - FarSounder's Protocol Buffer support for Matlab
// Copyright (c) 2008, FarSounder Inc.  All rights reserved.
// http://code.google.com/p/protobuf-matlab/
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
//     * Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
//     * Neither the name of the FarSounder Inc. nor the names of its
// contributors may be used to endorse or promote products derived from this
// software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

// Author: fedor.labounko@gmail.com (Fedor Labounko)
//  Based on Google's C++ Protobuf compiler.
//
// Generates Matlab code for a given .proto file.

#ifndef FARSOUNDER_PROTOBUF_COMPILER_MATLAB_GENERATOR_H__
#define FARSOUNDER_PROTOBUF_COMPILER_MATLAB_GENERATOR_H__

#include <string>
#include <google/protobuf/compiler/code_generator.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/stubs/common.h>

namespace google {
namespace protobuf {
class Descriptor;
class FileDescriptor;
namespace io {
class Printer;
}  // namespace io
}  // namespace protobuf
}  // namespace google

namespace farsounder {
namespace protobuf {
namespace compiler {
namespace matlab {

// CodeGenerator implementation which generates a C++ source file and
// header.  If you create your own protocol compiler binary and you want
// it to support C++ output, you can do so by registering an instance of this
// CodeGenerator with the CommandLineInterface in your main() function.
class LIBPROTOC_EXPORT MatlabGenerator :
  public ::google::protobuf::compiler::CodeGenerator {
 public:
  MatlabGenerator();
  ~MatlabGenerator();

  // implements CodeGenerator ----------------------------------------
  bool Generate(const ::google::protobuf::FileDescriptor* file,
                const ::std::string& parameter,
                ::google::protobuf::compiler::GeneratorContext* output_directory,
                ::std::string* error) const;

  enum MatlabType {
    MATLABTYPE_INT32      = 1,    // TYPE_SINT32, TYPE_INT32, TYPE_SFIXED32
    MATLABTYPE_INT64      = 2,    // TYPE_INT64, TYPE_SINT64, TYPE_SFIXED64
    MATLABTYPE_UINT32     = 3,    // TYPE_UINT32, TYPE_FIXED32, TYPE_BOOL
    MATLABTYPE_UINT64     = 4,    // TYPE_UINT64, TYPE_FIXED64
    MATLABTYPE_DOUBLE     = 5,    // TYPE_DOUBLE
    MATLABTYPE_SINGLE     = 6,    // TYPE_FLOAT
    MATLABTYPE_STRING     = 7,    // TYPE_STRING
    MATLABTYPE_BYTES      = 8,    // TYPE_BYTES
    MATLABTYPE_MESSAGE    = 9,    // TYPE_MESSAGE, TYPE_GROUP
    MATLABTYPE_ENUM       = 10,   // TYPE_ENUM

    MAX_MATLABTYPE        = 10

  };
  static const MatlabType kTypeToMatlabTypeMap[
    ::google::protobuf::FieldDescriptor::MAX_TYPE + 1];
  static const ::std::string kMatlabTypeToString[MAX_MATLABTYPE + 1];

 private:
  void PrintMessageFunctions() const;

  void PrintDescriptorFunction(
      const ::google::protobuf::Descriptor & descriptor) const;
  void PrintDescriptorHeader(
      ::google::protobuf::io::Printer & printer,
      const ::google::protobuf::Descriptor & descriptor) const;
  void PrintDescriptorComment(
      ::google::protobuf::io::Printer & printer,
      const ::google::protobuf::Descriptor & descriptor) const;
  void PrintDescriptorBody(
      ::google::protobuf::io::Printer & printer,
      const ::google::protobuf::Descriptor & descriptor) const;
  void PrintFieldDescriptors(
      ::google::protobuf::io::Printer & printer,
      const ::google::protobuf::Descriptor & descriptor) const;
  void PrintFieldIndecesByNumber(
      ::google::protobuf::io::Printer & printer,
      const ::google::protobuf::Descriptor & descriptor) const;

  void PrintReadFunction(
      const ::google::protobuf::Descriptor & descriptor) const;
  void PrintReadHeader(
      ::google::protobuf::io::Printer & printer,
      const ::google::protobuf::Descriptor & descriptor) const;
  void PrintReadComment(
      ::google::protobuf::io::Printer & printer,
      const ::google::protobuf::Descriptor & descriptor) const;
  void PrintReadBody(
      ::google::protobuf::io::Printer & printer,
      const ::google::protobuf::Descriptor & descriptor) const;

  ::std::string DefaultValueToString(
      const ::google::protobuf::FieldDescriptor & field) const;

  ::std::string MakeReadFunctionHandle(
      const ::google::protobuf::FieldDescriptor & field) const;
  ::std::string MakeWriteFunctionHandle(
      const ::google::protobuf::FieldDescriptor & field) const;

  ::std::string DescriptorFunctionName(
      const ::google::protobuf::Descriptor & descriptor) const;
  ::std::string ReadFunctionName(
      const ::google::protobuf::Descriptor & descriptor) const;

  // Very coarse-grained lock to ensure that Generate() is reentrant.
  // Guards file_ and printer_.
  mutable ::google::protobuf::internal::Mutex mutex_;
  mutable const ::google::protobuf::FileDescriptor*
      file_;  // Set in Generate().  Under mutex_.
  mutable ::google::protobuf::compiler::GeneratorContext*
      output_directory_;  // Set in Generate().

  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(MatlabGenerator);
};

}  // namespace cpp
}  // namespace compiler
}  // namespace protobuf

}  // namespace farsounder
#endif  // FARSOUNDER_PROTOBUF_COMPILER_MATLAB_GENERATOR_H__
