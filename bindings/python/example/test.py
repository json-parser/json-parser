import sys
sys.path.append('..')

import jsonparser

data = open('test.json', 'r').read()
try:
    output = jsonparser.decode(data)
    print(output)
except jsonparser.JSONException as e:
    print('Error -> %s' % e)
