project('libperspective', 'cpp',
  version : '0.1',
  default_options : ['warning_level=2', 'cpp_std=c++14'])

pymod = import('python')

py2 = pymod.find_installation('python2', required: get_option('python2'))
py3 = pymod.find_installation('python3', required: get_option('python3'))

if py3.found()
    py_dep = py3.dependency(embed: true)
elif py2.found()
    py_dep = py2.dependency(embed: true)
else
    py_dep = dependency('', required:false)
endif

lib_src = [
    'RawData.cpp',
    'Graph.cpp',
    'Projection.cpp',
]
if py_dep.found()
    lib_src += ['PythonGraph.cpp']
endif

library(
    'perspective',
    sources: lib_src,
    install : true,
    dependencies: [py_dep]
)

test_src = [
    'tests/main.cpp',
    'tests/graph.cpp',
    'tests/quaternion.cpp',
]
if py_dep.found()
    test_src += ['tests/graph_python.cpp']
endif

test_exe = executable(
    'tests',
    sources: [ lib_src, test_src ],
    dependencies: [py_dep]
)

test('catch2 tests', test_exe, args: ['-r', 'tap'], protocol: 'tap')

swig = find_program('swig')

swig_h_files = [
    'Quaternion.h',
    'Point.h',
    'Projection.h',
    'Helpers.h',
    'Space.h',
    'Graph.h',
    'PythonGraph.h',
]

swig_tmp = custom_target(
    'swig_tmp',
    build_by_default: py_dep.found(),
    command: [swig, '-python', '-c++', '-doxygen', '-outcurrentdir', '@INPUT@'],
    depend_files: [swig_h_files],
    input: 'libperspective.i',
    output: ['libperspective_wrap.cxx', 'libperspective.py'],
    install: true,
    install_dir: ''
)

library(
    '_libperspective',
    [swig_tmp],
    name_prefix: '',
    sources: [lib_src],
    install : true,
    dependencies: [py_dep],
)
