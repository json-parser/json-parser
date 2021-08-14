cdef extern from "wrap_json.c":
    object decode_json(char * value)
    object get_exception_class()

JSONException = get_exception_class()

def decode(value):
    value = value.encode('utf-8')
    return decode_json(value)
