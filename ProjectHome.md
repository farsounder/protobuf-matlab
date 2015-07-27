# What's this? #

Protocol Buffers support for Matlab.

Please see the [README.txt](http://code.google.com/p/protobuf-matlab/source/browse/README.txt) file for information on installation and setup.

# A Quick Example #

## Generating the Matlab Code ##

Given a proto file named person.proto:

```
message Person {
  required int32 id = 2;
  required string name = 1;
  optional string email = 3;

  enum PhoneType {
    MOBILE = 0;
    HOME = 1;
    WORK = 2;
  }

  message PhoneNumber {
    required string number = 1;
    optional PhoneType type = 2 [default = HOME];
  }

  repeated PhoneNumber phone = 4;
}
```

Compile it via `protoc --matlab_out=. person.proto` and you'll get four files:

  * pb\_read\_Person.m
  * pb\_read\_Person\_\_PhoneNumber.m
  * pb\_descriptor\_Person.m
  * pb\_descriptor\_Person\_\_PhoneNumber.m

## Using the Generated Code ##

You can now populate a Person message and write it to a file:

```
bob = pb_read_Person();
bob = pblib_set(bob, 'id', 123);
bob = pblib_set(bob, 'name', 'Bob');
bob = pblib_set(bob, 'email', 'bob@example.com');

bob = pblib_set(bob, 'phone', pb_read_Person__PhoneNumber());
bob.phone(1) = pblib_set(bob.phone(1), 'number', '555-555-1111');
bob.phone(1) = pblib_set(bob.phone(1), 'type', 1);

bob.phone(end+1) = pb_read_Person__PhoneNumber();
bob.phone(2) = pblib_set(bob.phone(2), 'number', '555-555-2222');
bob.phone(2) = pblib_set(bob.phone(2), 'type', 2);

buffer = pblib_generic_serialize_to_string(bob);
fid = fopen('person.pb', 'w');
fwrite(fid, buffer, 'uint8');
fclose(fid);
```

Lastly, if you'd like to read that file back in:

```
fid = fopen('person.pb');
buffer = fread(fid, [1 inf], '*uint8');
fclose(fid);
person = pb_read_Person(buffer);
```