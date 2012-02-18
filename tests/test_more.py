
# See bindings/python for the python jsonparser module

import jsonparser
import json
import glob

tests = [

    { 'foo': 'bar'},
    [ 1, 2, 3 ],
    4,
    'test',
    [[3, { "test": 412.2 }, 1], "hello"]

] + map(lambda file: json.loads(open(file).read()), glob.glob('valid*.json'))

for i, test in enumerate(tests):

    reencoded = jsonparser.decode(json.dumps(test))

    if reencoded != test:
        print '%d : Failed:\n\n%s\n\nbecame\n\n%s\n' % (i, encoded, reencoded)
    else:
        print '%d : Passed' % i



