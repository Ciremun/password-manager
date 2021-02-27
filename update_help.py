import re
docs = re.search(r'## Usage\n([^##]+)', open('README.md').read()).group(1).replace('\n', r'\\n\\\n')
main = re.sub(r'(const char \*help_s = ")([^\"]*)"', r'\1{0}"'.format(docs), open('src/io/common.c').read())
open('src/io/common.c', 'w').write(main)