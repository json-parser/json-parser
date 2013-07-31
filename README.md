Very low footprint JSON parser written in portable ANSI C.

* BSD licensed with no dependencies (i.e. just drop the C file into your project)
* Never recurses or allocates more memory than it needs
* Very simple API with operator sugar for C++

[![Build Status](https://secure.travis-ci.org/udp/json-parser.png)](http://travis-ci.org/udp/json-parser)

Installing
----------

There is now a makefile which will produce a libjsonparser static and dynamic library.  However, this
is _not_ required to build json-parser, and the source files (`json.c` and `json.h`) should be happy
in any build system you already have in place.


API
---

    json_value * json_parse (const json_char * json,
                             size_t length);

    json_value * json_parse_ex (json_settings * settings,
                                const json_char * json,
                                size_t length,
                                char * error);

    void json_value_free (json_value *);

The `type` field of `json_value` is one of:

* `json_object` (see `u.object.length`, `u.object.values[x].name`, `u.object.values[x].value`)
* `json_array` (see `u.array.length`, `u.array.values`)
* `json_integer` (see `u.integer`)
* `json_double` (see `u.dbl`)
* `json_string` (see `u.string.ptr`, `u.string.length`)
* `json_boolean` (see `u.boolean`)
* `json_null`
