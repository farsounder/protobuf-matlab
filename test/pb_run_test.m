function [msg, new_msg, buffer] = pb_run_test()
%pb_run_test
%   function [msg, new_msg, buffer] = pb_run_test()
%
%   Runs a basic write/read comparison test of basic proto functionality. This
%   is in no way a comprehensive test but just to catch obvious and common
%   errors. It might report float roundoff errors due to the original being
%   stored as doubles.

%   protobuf-matlab - FarSounder's Protocol Buffer support for Matlab
%   Copyright (c) 2008, FarSounder Inc.  All rights reserved.
%   http://code.google.com/p/protobuf-matlab/
%
%   Redistribution and use in source and binary forms, with or without
%   modification, are permitted provided that the following conditions are met:
%
%       * Redistributions of source code must retain the above copyright
%   notice, this list of conditions and the following disclaimer.
%
%       * Redistributions in binary form must reproduce the above copyright
%   notice, this list of conditions and the following disclaimer in the
%   documentation and/or other materials provided with the distribution.
%
%       * Neither the name of the FarSounder Inc. nor the names of its
%   contributors may be used to endorse or promote products derived from this
%   software without specific prior written permission.
%
%   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
%   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
%   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
%   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
%   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
%   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
%   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
%   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
%   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
%   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
%   POSSIBILITY OF SUCH DAMAGE.

%   Author: fedor.labounko@gmail.com (Fedor Labounko)

  msg = pb_read_test__TestAllTypes([]);
  msg = pblib_set(msg, 'optional_int32', 33);
  msg = pblib_set(msg, 'optional_int64', 234980098);
  msg = pblib_set(msg, 'optional_uint32', 23098);
  msg = pblib_set(msg, 'optional_uint64', 35098723058);
  msg = pblib_set(msg, 'optional_sint32', 25);
  msg = pblib_set(msg, 'optional_sint64', 98482);
  msg = pblib_set(msg, 'optional_fixed32', 2309);
  msg = pblib_set(msg, 'optional_fixed64', 24350809);
  msg = pblib_set(msg, 'optional_sfixed32', 2309);
  msg = pblib_set(msg, 'optional_sfixed64', 24350809);
  msg = pblib_set(msg, 'optional_float', 23089.235);
  msg = pblib_set(msg, 'optional_double', 24305.2342);
  msg = pblib_set(msg, 'optional_bool', 130498);
  msg = pblib_set(msg, 'optional_string', 'asldfkj');
  msg = pblib_set(msg, 'optional_bytes', uint8('aslkdjlj'));
  msg = pblib_set(msg, 'optional_nested_message', pb_read_test__TestAllTypes__NestedMessage([]));
  msg.optional_nested_message = pblib_set(msg.optional_nested_message, 'bb', -2);
  msg = pblib_set(msg, 'optional_foreign_message', pb_read_test__ForeignMessage([]));
  msg.optional_foreign_message = pblib_set(msg.optional_foreign_message, 'c', 14098);
  msg = pblib_set(msg, 'optional_import_message', pb_read_test_import__ImportMessage([]));
  msg.optional_import_message = pblib_set(msg.optional_import_message, 'd', 98);

  msg = pblib_set(msg, 'repeated_int32', [23 33]);
  msg = pblib_set(msg, 'repeated_int64', [9823408 234980098]);
  msg = pblib_set(msg, 'repeated_uint32', [23098 23098]);
  msg = pblib_set(msg, 'repeated_uint64', [23408 35098723058]);
  msg = pblib_set(msg, 'repeated_sint32', [-48 25]);
  msg = pblib_set(msg, 'repeated_sint64', [-234080 98482]);
  msg = pblib_set(msg, 'repeated_fixed32', [2309 230489]);
  msg = pblib_set(msg, 'repeated_fixed64', [23408 24350809]);
  msg = pblib_set(msg, 'repeated_sfixed32', [-2309 230489]);
  msg = pblib_set(msg, 'repeated_sfixed64', [-23408 24350809]);
  msg = pblib_set(msg, 'repeated_float', [23089.235 245789 30948.23508]);
  msg = pblib_set(msg, 'repeated_double', [24305.2342 20398.234089]);
  msg = pblib_set(msg, 'repeated_bool', [130498 9038]);
  msg = pblib_set(msg, 'repeated_string', {'asldfkj', 'asdlkfj'});
  msg = pblib_set(msg, 'repeated_bytes', {uint8('aslkdjlj'), uint8('hgsh')});
  msg = pblib_set(msg, 'repeated_nested_message', ...
                  [pb_read_test__TestAllTypes__NestedMessage([]) ...
                   pb_read_test__TestAllTypes__NestedMessage([])]);
  msg.repeated_nested_message(1) = pblib_set(msg.repeated_nested_message(1), 'bb', -2);
  msg.repeated_nested_message(2) = pblib_set(msg.repeated_nested_message(2), 'bb', 12394087);
  msg = pblib_set(msg, 'repeated_foreign_message', ...
                  [pb_read_test__ForeignMessage([]) ...
                   pb_read_test__ForeignMessage([])]);
  msg.repeated_foreign_message(1) = pblib_set(msg.repeated_foreign_message(1), 'c', 14098);
  msg.repeated_foreign_message(2) = pblib_set(msg.repeated_foreign_message(2), 'c', -90);
  msg = pblib_set(msg, 'repeated_import_message', ...
                  [pb_read_test_import__ImportMessage([]) ...
                   pb_read_test_import__ImportMessage([])]);
  msg.repeated_import_message(1) = pblib_set(msg.repeated_import_message(1), 'd', 98);
  msg.repeated_import_message(2) = pblib_set(msg.repeated_import_message(2), 'd', 98);

  buffer = pblib_generic_serialize_to_string(msg);
  new_msg = pb_read_test__TestAllTypes(buffer);

  check_msg_equal(msg, new_msg);

function check_msg_equal(old_msg, new_msg)
  d = new_msg.descriptor_function();
  for i=1:length(d.fields)
    field = d.fields(i);
    if (field.label == 3) % repeated
      for j=1:length(old_msg.(field.name))
        if (field.matlab_type == 7 || field.matlab_type == 8)
          old_val = old_msg.(field.name){j};
          new_val = new_msg.(field.name){j};
        else
          old_val = old_msg.(field.name)(j);
          new_val = new_msg.(field.name)(j);
        end
        if (field.matlab_type == 9)
          check_msg_equal(old_val, new_val);
        elseif (old_val ~= new_val)
          disp([field.name ': ' num2str(old_val) ' != ' num2str(new_val)]);
        end
      end
    else
      if (field.matlab_type == 9)
        check_msg_equal(old_msg.(field.name), new_msg.(field.name));
      elseif (old_msg.(field.name) ~= new_msg.(field.name))
        disp([field.name ': ' num2str(old_msg.(field.name)) ' != ' ...
              num2str(new_msg.(field.name))]);
      end
    end
  end

