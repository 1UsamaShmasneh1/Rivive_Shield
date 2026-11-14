from pathlib import Path
import json, subprocess
import xml.etree.ElementTree as ET
root=Path(__file__).resolve().parents[1]
items=json.loads((root/'catalog.json').read_text())
header=(root/'contracts/p3_draft.hpp').read_text()
catalog=(root/'docs/EXACT_CATALOG.md').read_text()
for item in items:
    for declaration in item['fields']+item['methods']+item['private']:
        assert declaration in header, (item['name'], declaration)
        assert declaration in catalog, (item['name'], declaration)
    svg=root/'diagrams'/f"{item['name']}.svg"
    assert svg.is_file()
    svg_text=' '.join(ET.parse(svg).getroot().itertext())
    for declaration in item['fields']+item['methods']+item['private']:
        assert declaration in svg_text, (item['name'], 'SVG drift', declaration)
subprocess.run(['g++','-std=c++17','-Wall','-Wextra','-Werror','-pedantic','-fsyntax-only',str(root/'contracts/compile_contracts.cpp')],check=True)
print(f'PASS: {len(items)} type catalogs and SVGs; strict C++17 declaration/static-trait checks.')
print('NOT tested: linking, implementation, MSVC, macOS, application behavior.')
