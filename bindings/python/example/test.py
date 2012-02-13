import sys
sys.path.append('..')

import jsonparser

data = open('test.json', 'rb').read()
try:
    output = jsonparser.decode(data)
    print output
except jsonparser.JSONException, e:
    print 'Error -> %s' % e