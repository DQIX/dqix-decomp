import argparse
import os
from pathlib import Path
import platform
import re
import subprocess

parser = argparse.ArgumentParser(description='Mangles function declarations')
parser.add_argument('file', help='C/C++ source file with defined function to mangle')

tools_dir = Path(os.path.dirname(os.path.realpath(__file__)))
cc_path = tools_dir / 'mwccarm' / '2.0' / 'sp1p5' / 'mwccarm.exe'
root_dir = tools_dir.parent
include_dir = root_dir / 'include'
libs_dir = root_dir / 'libs'
libc_include_dir = libs_dir / 'c' / 'include'
libcpp_include_dir = libs_dir / 'cpp' / 'include'
libnds_include_dir = libs_dir / 'nds' / 'include'

if platform.system() == 'Windows': cc = [str(cc_path)]
else: cc = ['wine', str(cc_path)]

args = parser.parse_args()

cc.extend([
    '-enum', 'int',
    '-char', 'signed',
    '-proc', 'arm946e',
    '-gccext,on',
    '-fp', 'soft',
    '-inline', 'noauto',
    '-Cpp_exceptions', 'off',
    '-RTTI', 'off',
    '-interworking',
    '-nolink',
    '-msgstyle', 'gcc',
    '-dis',
    '-gccinc',
    '-i', include_dir,
    '-i', libc_include_dir,
    '-i', libcpp_include_dir,
    '-i', libnds_include_dir,
    args.file
])

try:
    output = subprocess.check_output(cc)
except subprocess.CalledProcessError as e:
    print(e.stdout.decode())
    exit(1)

output = output.decode()

# print(output)

mangled_funcs: list[str] = re.findall(r'.text +([^\$ ]\S+)', output)
init_funcs: list[str] = re.findall(r'.init +([^\$ ]\S+)', output)
mangled_data: list[str] = re.findall(r'.data +([^\. ]\S+)', output)
mangled_bss: list[str] = re.findall(r'.bss +([^\. ]\S+)', output)

if len(mangled_funcs) > 0:
    print('.text:\n')
    for func in mangled_funcs:
        print(func)
    print('\n')
if len(init_funcs) > 0:
    print('.init:\n')
    for func in init_funcs:
        print(func)
    print('\n')
if len(mangled_data) > 0:
    print('.data:\n')
    for data in mangled_data:
        print(data)
    print('\n')
if len(mangled_bss) > 0:
    print('.bss:\n')
    for bss in mangled_bss:
        print(bss)
    print('\n')
