import os

# Common compiler options.
cflags = '-std=c99'
cxxflags = '-std=c++14'
#ccflags = (
#  '-Werror -Wfatal-errors -Wpedantic -pedantic-errors -Wall -Wextra '
#  '-Wno-missing-braces -Wno-strict-overflow -Wno-sign-compare ')

# Additional compiler options per build variant.
debug = '-O0 -g '
devel = '-O3 '
release = '-O3 -DNDEBUG '

# Get options from the command line.
variant = ARGUMENTS.get('variant', 'Devel')

# Assemble ccflags according to the build variant.
ccflags = '-Wfatal-errors '
if variant == 'Debug':
  ccflags += debug
elif variant == 'Devel':
  ccflags += devel
elif variant == 'Release':
  ccflags += release
else:
  print('Invalid value for option variant: ' + variant + '.')
  print("Valid values are: ('Debug', 'Devel', 'Release')")
  Exit(1)


# Set construction variables in the default environment.
DefaultEnvironment(
  CFLAGS = cflags,
  CXXFLAGS = cxxflags,
  CCFLAGS = ccflags,
#  CPPDEFINES = cppdefines,
  CPPPATH = ['./External/RoutingKit/include/', '#', './External/routing-framework/', './External/', './External/routing-framework/'],
  LIBPATH=['./External/RoutingKit/lib/'],
  ENV = {
    'CPLUS_INCLUDE_PATH': os.environ.get('CPLUS_INCLUDE_PATH'),
    'LIBRARY_PATH': os.environ.get('LIBRARY_PATH'),
    'PATH': os.environ.get('PATH')})

p1 = Program(source=['OsmImport/Import.cpp'], LIBS=['cairo', 'routingkit'])
p2 = Program(source=['WebServer/SkiServer.cpp', 'External/mongoose/mongoose.c'])

Alias('Import', p1)
Alias('SkiServer', p2)
