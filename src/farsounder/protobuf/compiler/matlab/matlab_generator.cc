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

#include <farsounder/protobuf/compiler/matlab/matlab_generator.h>

#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <sstream>
#include <utility>
#include <vector>

#include <google/protobuf/io/printer.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/stubs/strutil.h>
#include <google/protobuf/wire_format.h>

namespace farsounder {
namespace protobuf {
namespace compiler {
namespace matlab {

using ::google::protobuf::Descriptor;
using ::google::protobuf::FieldDescriptor;
using ::google::protobuf::FieldDescriptor;
using ::google::protobuf::FileDescriptor;
using ::google::protobuf::LowerString;
using ::google::protobuf::SimpleDtoa;
using ::google::protobuf::SimpleFtoa;
using ::google::protobuf::SimpleItoa;
using ::google::protobuf::StringReplace;
using ::google::protobuf::compiler::GeneratorContext;
using ::google::protobuf::internal::MutexLock;
using ::google::protobuf::io::Printer;

using ::std::make_pair;
using ::std::map;
using ::std::max;
using ::std::pair;
using ::std::set;
using ::std::string;
using ::std::stringstream;
using ::std::vector;

namespace {
// CamelToLower taken from Dave Benson's modification for a C implementation
// of Protocol Buffers
string CamelToLower(const string &name) {
  string new_name;
  if (name.size() < 2) {
    new_name = name;
    LowerString(&new_name);
    return new_name;
  }
  new_name += tolower(name[0]);
  for (int i = 1; i < name.size(); ++i) {
    if (isupper(name[i]))
      new_name += "_";
    new_name += tolower(name[i]);
  }
  return new_name;
}
}  // namespace

// See Type enum in descriptor.h
const MatlabGenerator::MatlabType
MatlabGenerator::kTypeToMatlabTypeMap[FieldDescriptor::MAX_TYPE + 1] = {
  static_cast<MatlabGenerator::MatlabType>(-1),   //invalid
  MATLABTYPE_DOUBLE,   // TYPE_DOUBLE
  MATLABTYPE_SINGLE,   // TYPE_FLOAT
  MATLABTYPE_INT64,    // TYPE_INT64
  MATLABTYPE_UINT64,   // TYPE_UINT64
  MATLABTYPE_INT32,    // TYPE_INT32
  MATLABTYPE_UINT64,   // TYPE_FIXED64
  MATLABTYPE_UINT32,   // TYPE_FIXED32
  MATLABTYPE_UINT32,   // TYPE_BOOL
  MATLABTYPE_STRING,   // TYPE_STRING
  MATLABTYPE_MESSAGE,  // TYPE_GROUP
  MATLABTYPE_MESSAGE,  // TYPE_MESSAGE
  MATLABTYPE_BYTES,    // TYPE_BYTES
  MATLABTYPE_UINT32,   // TYPE_UINT32
  MATLABTYPE_ENUM,     // TYPE_ENUM
  MATLABTYPE_INT32,    // TYPE_SFIXED32
  MATLABTYPE_INT64,    // TYPE_SFIXED64
  MATLABTYPE_INT32,    // TYPE_SINT32
  MATLABTYPE_INT64     // TYPE_SINT64
};

const string
MatlabGenerator::kMatlabTypeToString[MatlabGenerator::MAX_MATLABTYPE + 1] = {
  "invalid", // invalid index
  "int32", // MATLABTYPE_INT32
  "int64", // MATLABTYPE_INT64
  "uint32", // MATLABTYPE_UINT32
  "uint64", // MATLABTYPE_UINT64
  "double", // MATLABTYPE_DOUBLE
  "single", // MATLABTYPE_SINGLE
  "string", // MATLABTYPE_STRING
  "uint8 vector", // MATLABTYPE_BYTES
  "message", // MATLABTYPE_MESSAGE
  "enum", // MATLABTYPE_ENUM
};

MatlabGenerator::MatlabGenerator() {}
MatlabGenerator::~MatlabGenerator() {}


bool MatlabGenerator::Generate(const FileDescriptor* file,
                               const string& parameter,
                               GeneratorContext* output_directory,
                               string* error) const {
  MutexLock lock(&mutex_);
  file_ = file;
  output_directory_ = output_directory;
  PrintMessageFunctions();
  return true;
}


void MatlabGenerator::PrintMessageFunctions() const {
  for (int i = 0; i < file_->message_type_count(); ++i) {
    PrintDescriptorFunction(*file_->message_type(i));
    PrintReadFunction(*file_->message_type(i));
  }
}


void MatlabGenerator::PrintDescriptorFunction(
    const Descriptor & descriptor) const {
  // Print nested messages
  for (int i = 0; i < descriptor.nested_type_count(); ++i) {
    PrintDescriptorFunction(*descriptor.nested_type(i));
  }

  string filename = DescriptorFunctionName(descriptor);
  filename += ".m";
  google::protobuf::internal::scoped_ptr<
    google::protobuf::io::ZeroCopyOutputStream>
      output(output_directory_->Open(filename));
  Printer printer(output.get(), '$');

  PrintDescriptorHeader(printer, descriptor);
  PrintDescriptorComment(printer, descriptor);
  printer.Indent();
  PrintDescriptorBody(printer, descriptor);
  printer.Outdent();
}


void MatlabGenerator::PrintDescriptorHeader(
    Printer & printer, const Descriptor & descriptor) const {
  string function_name = DescriptorFunctionName(descriptor);
  printer.Print("function [descriptor] = $function_name$()\n",
                "function_name", function_name);
}


void MatlabGenerator::PrintDescriptorComment(
    Printer & printer, const Descriptor & descriptor) const {
  string name = descriptor.name();
  string function_name = DescriptorFunctionName(descriptor);
  printer.Print("%$function_name$ Returns the descriptor for message $name$.\n",
                "name", name, "function_name", function_name);
  printer.Print("%   ");
  PrintDescriptorHeader(printer, descriptor);
  printer.Print("%\n");
  printer.Print("%   See also $read_function$",
                "read_function", ReadFunctionName(descriptor));
  printer.Print("\n");
}


void MatlabGenerator::PrintDescriptorBody(
    Printer & printer, const Descriptor & descriptor) const {
  printer.Print("\n");
  printer.Print("descriptor = struct( ...\n");
  printer.Indent();
  printer.Print("'name', '$name$', ...\n", "name", descriptor.name());
  printer.Print("'full_name', '$full_name$', ...\n", "full_name",
                descriptor.full_name());
  printer.Print("'filename', '$filename$', ...\n", "filename",
                descriptor.file()->name());
  printer.Print("'containing_type', '$containing_type$', ...\n",
                "containing_type",
                descriptor.containing_type() == NULL ? "" :
                descriptor.containing_type()->full_name());

  printer.Print("'fields', [ ...\n");
  printer.Indent();
  PrintFieldDescriptors(printer, descriptor);
  printer.Outdent();
  printer.Print("], ...\n");

  printer.Print("'extensions', [ ... % Not Implemented\n");
  printer.Indent();
  // PrintExtensions(printer, descriptor);
  printer.Outdent();
  printer.Print("], ...\n");

  printer.Print("'nested_types', [ ... % Not implemented\n");
  printer.Indent();
  // PrintNestedTypes(printer, descriptor);
  printer.Outdent();
  printer.Print("], ...\n");

  printer.Print("'enum_types', [ ... % Not Implemented\n");
  printer.Indent();
  // PrintEnumTypes(printer, descriptor);
  printer.Outdent();
  printer.Print("], ...\n");

  printer.Print("'options', [ ... % Not Implemented\n");
  printer.Indent();
  // PrintOptions(printer, descriptor);
  printer.Outdent();
  printer.Print("] ...\n");

  printer.Outdent();
  printer.Print(");\n\n");

  PrintFieldIndecesByNumber(printer, descriptor);
}


void MatlabGenerator::PrintFieldDescriptors(
    Printer & printer, const Descriptor & descriptor) const {
  vector<pair<int, int> > sorted_fields;
  for (int i = 0; i < descriptor.field_count(); ++i) {
    sorted_fields.push_back(make_pair(descriptor.field(i)->number(), i));
  }
  sort(sorted_fields.begin(), sorted_fields.end());

  const FieldDescriptor * field;
  map<string, string> m;
  for (int i = 0; i < descriptor.field_count(); ++i) {
    field = descriptor.field(sorted_fields[i].second);
    m.clear();
    printer.Print("struct( ...\n");
    printer.Indent();
    m["name"] = field->name();
    m["full_name"] = field->full_name();
    m["index"] = SimpleItoa(i + 1);
    m["number"] = SimpleItoa(field->number());
    m["type"] = SimpleItoa(field->type());
    m["matlab_type"] = SimpleItoa(kTypeToMatlabTypeMap[field->type()]);
    m["wire_type"] = SimpleItoa(
        google::protobuf::internal::WireFormat::WireTypeForFieldType(
            field->type()));
    m["label"] = SimpleItoa(field->label());
    m["default_value"] = DefaultValueToString(*field);

    m["read_function"] = MakeReadFunctionHandle(*field);
    m["write_function"] = MakeWriteFunctionHandle(*field);
    if (field->options().packed()) {
      m["packed"] = "true";
    } else {
      m["packed"] = "false";
    }
    printer.Print(m,
                  "'name', '$name$', ...\n"
                  "'full_name', '$full_name$', ...\n"
                  "'index', $index$, ...\n"
                  "'number', uint32($number$), ...\n"
                  "'type', uint32($type$), ...\n"
                  "'matlab_type', uint32($matlab_type$), ...\n"
                  "'wire_type', uint32($wire_type$), ...\n"
                  "'label', uint32($label$), ...\n"
                  "'default_value', $default_value$, ...\n"
                  "'read_function', $read_function$, ...\n"
                  "'write_function', $write_function$, ...\n"
                  "'options', struct('packed', $packed$) ...\n"
                  );
    printer.Outdent();

    if (i != descriptor.field_count() - 1)
      printer.Print("), ...\n");
    else
      printer.Print(") ...\n");
  }
}


void MatlabGenerator::PrintFieldIndecesByNumber(
    Printer & printer, const Descriptor & descriptor) const {
  // Assumes the fields are entered into an array by increasing tag values
  printer.Print("descriptor.field_indeces_by_number = java.util.HashMap;\n");
  const FieldDescriptor * field;
  map<string, string> map_values;
  vector<pair<int, int> > sorted_fields;
  for (int i = 0; i < descriptor.field_count(); ++i) {
    sorted_fields.push_back(make_pair(descriptor.field(i)->number(), i));
  }
  sort(sorted_fields.begin(), sorted_fields.end());
  for (int i = 0; i < descriptor.field_count(); ++i) {
    field = descriptor.field(sorted_fields[i].second);
    printer.Print("put(descriptor.field_indeces_by_number, uint32($number$), $index$);\n",
                  "number", SimpleItoa(field->number()),
                  "index", SimpleItoa(i + 1));
  }
  printer.Print("\n");
}


void MatlabGenerator::PrintReadFunction(const Descriptor & descriptor) const {
  // Print nested messages
  for (int i = 0; i < descriptor.nested_type_count(); ++i) {
    PrintReadFunction(*descriptor.nested_type(i));
  }

  string filename = ReadFunctionName(descriptor);
  filename += ".m";
  google::protobuf::internal::scoped_ptr<
    google::protobuf::io::ZeroCopyOutputStream>
      output(output_directory_->Open(filename));
  Printer printer(output.get(), '$');

  PrintReadHeader(printer, descriptor);
  PrintReadComment(printer, descriptor);
  printer.Indent();
  PrintReadBody(printer, descriptor);
  printer.Outdent();
}


void MatlabGenerator::PrintReadHeader(Printer & printer,
                                      const Descriptor & descriptor) const {
  string name = CamelToLower(descriptor.name());
  string function_name = ReadFunctionName(descriptor);
  printer.Print("function [$name$] = $function_name$(buffer, buffer_start, buffer_end)\n",
                "name", name,
                "function_name", function_name);
}


void MatlabGenerator::PrintReadComment(Printer & printer,
                                       const Descriptor & descriptor) const {
  string name = descriptor.name();
  string function_name = ReadFunctionName(descriptor);
  printer.Print("%$function_name$ Reads the protobuf message $name$.\n",
                "name", name,
                "function_name", function_name);
  printer.Print("%   ");
  PrintReadHeader(printer, descriptor);
  printer.Print("%\n");
  printer.Print("%   INPUTS:\n"
                "%     buffer       : a buffer of uint8's to parse\n"
                "%     buffer_start : optional starting index to consider of the buffer\n"
                "%                    defaults to 1\n"
                "%     buffer_end   : optional ending index to consider of the buffer\n"
                "%                    defaults to length(buffer)\n"
                "%\n"
                "%   MEMBERS:\n");
  for (int i = 0; i < descriptor.field_count(); ++i) {
    const FieldDescriptor & field = *descriptor.field(i);
    string buffer_space(max(0, static_cast<int>(15 - field.name().size())), ' ');
    printer.Print("%     $name$$buffer$: ",
                  "name", field.name(),
                  "buffer", buffer_space);
    switch(field.label()) {
      case FieldDescriptor::LABEL_OPTIONAL:
        printer.Print("optional ");
        break;
      case FieldDescriptor::LABEL_REQUIRED:
        printer.Print("required ");
        break;
      case FieldDescriptor::LABEL_REPEATED:
        printer.Print("repeated ");
        break;
      default:
        GOOGLE_LOG(FATAL)<<"Unhandled case in print comment.";
    }
    if (field.type() == FieldDescriptor::TYPE_MESSAGE) {
      printer.Print("<a href=\"matlab:help $read_function$\">$type$</a>",
                    "read_function", ReadFunctionName(*field.message_type()),
                    "type", field.message_type()->full_name());
    } else {
      printer.Print("$type$",
                    "type", kMatlabTypeToString[kTypeToMatlabTypeMap[field.type()]]);
    }
    printer.Print(", defaults to $default$.\n", "default", DefaultValueToString(field));
  }
  bool first_to_print = true;
  string beginning_string = "%\n%   See also";

  // Will store all the types we'll print in the See also help string
  // Used so we don't print duplicates
  int num = 0;
  set<const Descriptor *> used_types;

#define PRINT_SEE_ALSO(DESCRIPTOR)                                \
  if (used_types.count(DESCRIPTOR) == 0) {                        \
    if (first_to_print) {                                         \
      printer.Print("$beginning$ $function$",                     \
                    "beginning", beginning_string,                \
                    "function", ReadFunctionName(*(DESCRIPTOR))); \
      first_to_print = false;                                     \
    } else {                                                      \
      printer.Print(", $function$",                               \
                    "function", ReadFunctionName(*(DESCRIPTOR))); \
    }                                                             \
    used_types.insert(DESCRIPTOR);                                \
  }

  // Add the containing type
  if (descriptor.containing_type() != NULL) {
    PRINT_SEE_ALSO(descriptor.containing_type());
  }

  // Add types used as fields in this message
  for (int i = 0; i < descriptor.field_count(); ++i) {
    if (descriptor.field(i)->type() !=
        ::google::protobuf::FieldDescriptor::TYPE_MESSAGE) {
      continue;
    }
    PRINT_SEE_ALSO(descriptor.field(i)->message_type());
  }

  // Add nested types defined in this message
  for (int i = 0; i < descriptor.nested_type_count(); ++i) {
    PRINT_SEE_ALSO(descriptor.nested_type(i));
  }

  // Print other message types defined in the same file
  for (int i = 0; i < file_->message_type_count(); ++i) {
    if (file_->message_type(i) == &descriptor)
      continue;
    PRINT_SEE_ALSO(file_->message_type(i));
  }
#undef PRINT_SEE_ALSO

  if (!first_to_print) {
    printer.Print(".\n");
  }
}


void MatlabGenerator::PrintReadBody(Printer & printer,
                                    const Descriptor & descriptor) const {
  printer.Print("\n");
  printer.Print("if (nargin < 1)\n"
                "  buffer = uint8([]);\n"
                "end\n"
                "if (nargin < 2)\n"
                "  buffer_start = 1;\n"
                "end\n"
                "if (nargin < 3)\n"
                "  buffer_end = length(buffer);\n"
                "end\n"
                "\n");
  string name = CamelToLower(descriptor.name());
  string descriptor_function = DescriptorFunctionName(descriptor);
  printer.Print("descriptor = $descriptor_function$();\n",
                "descriptor_function", descriptor_function);
  printer.Print("$name$ = pblib_generic_parse_from_string(buffer, descriptor, buffer_start, buffer_end);\n",
                "name", name);
  printer.Print("$name$.descriptor_function = @$descriptor_function$;\n",
                "name", name, "descriptor_function", descriptor_function);
}


string MatlabGenerator::DefaultValueToString(
    const FieldDescriptor & field) const {
  MatlabType type = kTypeToMatlabTypeMap[field.type()];
  field.has_default_value();
  stringstream s;
  if (field.is_repeated()) {
    switch(type) {
      case MATLABTYPE_INT32:
        return "int32([])";
      case MATLABTYPE_INT64:
        return "int64([])";
      case MATLABTYPE_UINT32:
        return "uint32([])";
      case MATLABTYPE_UINT64:
        return "uint64([])";
      case MATLABTYPE_DOUBLE:
        return "double([])";
      case MATLABTYPE_SINGLE:
        return "single([])";
      case MATLABTYPE_MESSAGE:
        return "struct([])";
      case MATLABTYPE_ENUM:
        return "int32([])";
      case MATLABTYPE_STRING:
        return "char([])";
      case MATLABTYPE_BYTES:
        return "uint8([])";
    }
  } else {
    switch(type) {
      case MATLABTYPE_INT32:
        s << "int32(" << SimpleItoa(field.default_value_int32()) << ")";
        return s.str();
      case MATLABTYPE_INT64:
        s << "int64(" << SimpleItoa(field.default_value_int64()) << ")";
        return s.str();
      case MATLABTYPE_UINT32:
        s << "uint32(";
        // This is needed as the default values are in a union and if the bool
        // value is set it doesn't set the full 32 bits
        if (field.type() == FieldDescriptor::TYPE_BOOL) {
          s << SimpleItoa(field.default_value_bool());
        } else {
          s << SimpleItoa(field.default_value_uint32());
        }
        s << ")";
        return s.str();
      case MATLABTYPE_UINT64:
        s << "uint64(" << SimpleItoa(field.default_value_uint64()) << ")";
        return s.str();
      case MATLABTYPE_DOUBLE:
        s << "double(" << SimpleDtoa(field.default_value_double()) << ")";
        return s.str();
      case MATLABTYPE_SINGLE:
        s << "single(" << SimpleFtoa(field.default_value_float()) << ")";
        return s.str();
      case MATLABTYPE_MESSAGE:
        return "struct([])";
      case MATLABTYPE_ENUM:
        s << "int32(" << SimpleItoa(field.default_value_enum()->number())<< ")";
        return s.str();
      case MATLABTYPE_STRING:
        s << "'" << StringReplace(field.default_value_string(), "'", "''", true) << "'";
        return s.str();
      case MATLABTYPE_BYTES:
        s << "uint8('" << StringReplace(field.default_value_string(), "'", "''", true) << "')";
        return s.str();
    }
  }
  GOOGLE_LOG(FATAL) << "Not reached.";
  return "''";
}


string MatlabGenerator::MakeReadFunctionHandle(
    const FieldDescriptor & field) const {
  MatlabType type = kTypeToMatlabTypeMap[field.type()];
  FieldDescriptor::Type proto_type = field.type();
  string function_handle;
  switch(type) {
    case MATLABTYPE_INT32:
      // We must call pblib_helpers_first because the standard varint
      // will put the result into a uint64
      //
      // We must also figure out whether this is encoded using the ZigZag
      // encoding, which is the case if its type was specificied as sint32 or
      // sint64
      if (proto_type == FieldDescriptor::TYPE_SINT32) {
        return "@(x) pblib_helpers_first(typecast(pblib_helpers_iff(bitget(x, 1), bitset(bitshift(bitxor(intmax('uint64'), x), -1), 64), bitshift(x, -1)), 'int32'))";
      } else {
        return "@(x) pblib_helpers_first(typecast(x, 'int32'))";
      }
    case MATLABTYPE_INT64:
      // We must figure out whether this is encoded using the ZigZag encoding,
      // which is the case if its type was specificied as sint32 or sint64
      if (proto_type == FieldDescriptor::TYPE_SINT64) {
        return "@(x) typecast(pblib_helpers_iff(bitget(x, 1), bitset(bitshift(bitxor(intmax('uint64'), x), -1), 64), bitshift(x, -1)), 'int64')";
      } else {
        return "@(x) typecast(x, 'int64')";
      }
    case MATLABTYPE_UINT32:
      // We must call pblib_helpers_first because the standard varint
      // will put the result into a uint64
      return "@(x) pblib_helpers_first(typecast(x, 'uint32'))";
    case MATLABTYPE_UINT64:
      return "@(x) typecast(x, 'uint64')";
    case MATLABTYPE_DOUBLE:
      return "@(x) typecast(x, 'double')";
    case MATLABTYPE_SINGLE:
      return "@(x) typecast(x, 'single')";
    case MATLABTYPE_STRING:
      return "@(x) char(x{1}(x{2} : x{3}))";
    case MATLABTYPE_BYTES:
      return "@(x) uint8(x{1}(x{2} : x{3}))";
    case MATLABTYPE_MESSAGE:
      return "@(x) " + ReadFunctionName(*field.message_type()) + "(x{1}, x{2}, x{3})";
    case MATLABTYPE_ENUM:
      // We must call pblib_helpers_first because the standard varint
      // will put the result into a uint64
      return "@(x) pblib_helpers_first(typecast(x, 'int32'))";
  }
  GOOGLE_LOG(FATAL) << "Shouldn't get here since switch should catch all cases.";
  return "";
}


string MatlabGenerator::MakeWriteFunctionHandle(const FieldDescriptor & field) const {
  // These functions should undo the work done by the read functions so as to
  // pass as input to write_varint the same values that read_varint would pass
  // back as output.  That's why, in particular, we typecast fixed32,
  // sfixed32, etc. into uint8 as that's how read_varint would return them to
  // us.
  MatlabType matlab_type = kTypeToMatlabTypeMap[field.type()];
  FieldDescriptor::Type type = field.type();
  string function_handle;
  switch(matlab_type) {
    case MATLABTYPE_INT32:
      // We must figure out whether this is encoded using the ZigZag encoding,
      // which is the case if its type was specificied as sint32 or sint64 I
      // am not able to find an arithmetic shift in Matlab, but if one is
      // found we can do the zigzag with (n<<1) ^ (n>>31)
      switch (type) {
        case FieldDescriptor::TYPE_INT32:
          return "@(x) typecast(int32(x), 'uint32')";
        case FieldDescriptor::TYPE_SFIXED32:
          return "@(x) typecast(int32(x), 'uint8')";
        case FieldDescriptor::TYPE_SINT32:
          return "@(x) typecast(pblib_helpers_iff(int32(x) < 0, -2 * int32(x) - 1, 2 * int32(x)), 'uint32')";
      }
      GOOGLE_LOG(DFATAL)<<"Unhandled matlabtype_int32 type "<<type<<" in WriteFunctionHandle.";
      break;
    case MATLABTYPE_INT64:
      // We must figure out whether this is encoded using the ZigZag encoding,
      // which is the case if its type was specificied as sint32 or sint64 I
      // am not able to find an arithmetic shift in Matlab, but if one is
      // found we can do the zigzag with (n<<1) ^ (n>>63)
      switch(type) {
        case FieldDescriptor::TYPE_INT64:
          return "@(x) typecast(int64(x), 'uint64')";
        case FieldDescriptor::TYPE_SFIXED64:
          return "@(x) typecast(int64(x), 'uint8')";
        case FieldDescriptor::TYPE_SINT64:
          return "@(x) pblib_helpers_iff(int64(x) < 0, bitxor(bitshift(typecast(int64(x), 'uint64'), 1), intmax('uint64')), bitshift(typecast(int64(x), 'uint64'), 1))";
      }
      GOOGLE_LOG(DFATAL)<<"Unhandled matlabtype_int32 type "<<type<<" in WriteFunctionHandle.";
      break;
    case MATLABTYPE_UINT32:
      switch(type) {
        case FieldDescriptor::TYPE_FIXED32:
          return "@(x) typecast(uint32(x), 'uint8')";
        case FieldDescriptor::TYPE_BOOL:
        case FieldDescriptor::TYPE_UINT32:
          return "@(x) typecast(uint32(x), 'uint32')";
      }
      GOOGLE_LOG(DFATAL)<<"Unhandled matlabtype_uint32 type "<<type<<" in WriteFunctionHandle.";
      break;
    case MATLABTYPE_UINT64:
      switch(type) {
        case FieldDescriptor::TYPE_UINT64:
          return "@(x) typecast(uint64(x), 'uint64')";
        case FieldDescriptor::TYPE_FIXED64:
          return "@(x) typecast(uint64(x), 'uint8')";
      }
      GOOGLE_LOG(DFATAL)<<"Unhandled matlabtype_uint64 type "<<type<<" in WriteFunctionHandle.";
      break;
    case MATLABTYPE_DOUBLE:
      return "@(x) typecast(double(x), 'uint8')";
    case MATLABTYPE_SINGLE:
      return "@(x) typecast(single(x), 'uint8')";
    case MATLABTYPE_STRING:
      return "@uint8";
    case MATLABTYPE_BYTES:
      return "@uint8";
    case MATLABTYPE_MESSAGE:
      return "@pblib_generic_serialize_to_string";
    case MATLABTYPE_ENUM:
      // We must call pblib_helpers_first because the standard varint
      // will put the result into a uint64
      return "@(x) typecast(int32(x), 'uint32')";
  }
  GOOGLE_LOG(DFATAL) << "Shouldn't get here since switch should catch all cases.";
  return "";
}


string MatlabGenerator::DescriptorFunctionName(const Descriptor & descriptor) const {
  return "pb_descriptor_" + StringReplace(descriptor.full_name(), ".", "__", true);
}

string MatlabGenerator::ReadFunctionName(const Descriptor & descriptor) const {
  return "pb_read_" + StringReplace(descriptor.full_name(), ".", "__", true);
}


}  // namespace matlab
}  // namespace compiler
}  // namespace protobuf
}  // namespace google
