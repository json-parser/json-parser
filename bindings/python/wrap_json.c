/*
 * Copyright (C) 2012-2021 the json-parser authors  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "../../json.c"

PyObject * json_exception = PyErr_NewException("jsonparser.JSONException",
    NULL, NULL);

PyObject * get_exception_class()
{
    return json_exception;
}

PyObject * convert_value(json_value * data)
{
    PyObject * value;
    switch (data->type) {
        case json_object:
            value = PyDict_New();
            for (int i = 0; i < data->u.object.length; i++) {
                PyObject * name = PyUnicode_FromString(
                    data->u.object.values[i].name);
                PyObject * object_value = convert_value(
                    data->u.object.values[i].value);
                PyDict_SetItem(value, name, object_value);
            }
            break;
        case json_array:
            value = PyList_New(0);
            for (int i = 0; i < data->u.array.length; i++) {
                PyObject * array_value = convert_value(
                    data->u.array.values[i]);
                PyList_Append(value, array_value);
            }
            break;
        case json_integer:
            value = PyInt_FromLong(data->u.integer);
            break;
        case json_double:
            value = PyFloat_FromDouble(data->u.dbl);
            break;
        case json_string:
            value = PyUnicode_FromStringAndSize(data->u.string.ptr,
                data->u.string.length);
            break;
        case json_boolean:
            value = PyBool_FromLong((long)data->u.boolean);
            break;
        default:
            // covers json_null, json_none
            Py_INCREF(Py_None);
            value = Py_None;
            break;
    }
    return value;
}

PyObject * decode_json(char * data)
{
    json_settings settings;
    memset(&settings, 0, sizeof (json_settings));
    settings.settings = json_enable_comments;
    char error[256];
    json_value * value = json_parse_ex(&settings, data, strlen(data), error);
    if (value == 0) {
        return PyErr_Format(json_exception, error);
    }
    PyObject * converted = convert_value(value);
    json_value_free(value);
    return converted;
}
