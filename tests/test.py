#!/usr/bin/env python3

# See bindings/python for the python jsonparser module

import jsonparser
import json
import glob

passed = "\033[92mPassed\033[0m"
failed = "\033[91mFailed\033[0m"

# Strict JSON tests
#
for i, test in enumerate(
        map(lambda file: open(file).read(), sorted(glob.glob('valid*.json')))):

    try:
        jsonparser.decode(test)
    except jsonparser.JSONException as error:
        print('valid/%d: Failed with error: %s' % (i, error))
        continue

    py_decoded = json.loads(test)
    py_reencoded = json.dumps(py_decoded)

    try:
        reencoded = jsonparser.decode(py_reencoded)
    except jsonparser.JSONException as error:
        print('valid/%d: Failed on re-encoded version with error: %s'
              % (i, error))
        continue

    if reencoded != py_decoded:
        print('valid/%d: %s:\n\n%s\n\nbecame\n\n%s\n'
              % (i, failed, test, reencoded))
    else:
        print('valid/%d: %s' % (i, passed))

for i, test in enumerate(map(lambda file: open(file).read(),
                             sorted(glob.glob('invalid*.json')))):

    try:
        jsonparser.decode(test)
    except jsonparser.JSONException as error:
        print('invalid/%d: %s: %s' % (i, passed, error))
        continue

    print('invalid/%d: %s (parsing succeeded and shouldn\'t have)'
          % (i, failed))


# Extension tests
#
for i, test in enumerate(map(lambda file: open(file).read(),
                         sorted(glob.glob('ext-valid*.json')))):

    try:
        decoded = jsonparser.decode(test)
        print('ext-valid/%d: %s: %s' % (i, passed, json.dumps(decoded)))
    except jsonparser.JSONException as error:
        print('ext-valid/%d: Failed with error: %s' % (i, error))
        continue

for i, test in enumerate(map(lambda file: open(file).read(),
                         sorted(glob.glob('ext-invalid*.json')))):

    try:
        jsonparser.decode(test)
    except jsonparser.JSONException as error:
        print('ext-invalid/%d: %s: %s' % (i, passed, error))
        continue

    print ('ext-invalid/%d: %s (parsing succeeded and shouldn\'t have)'
           % (i, failed))
