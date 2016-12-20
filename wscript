import sys, subprocess

top = '.'
out = 'build'

def options(cnf):
	cnf.load('compiler_c compiler_cxx')
	cnf.add_option('-w', '--windows', dest='windows_build', default=False, action='store_true', help='Enables windows builds')
	
def configure(cnf):
	cnf.env.BINPREFIX = ''
	cnf.env.ACTUAL_LIBDIR = []

	if cnf.options.windows_build:
		cnf.env.WINDOWS_BUILD = True

		cnf.env.PREFIX = '/opt/mxe/usr/i686-w64-mingw32.static'
		cnf.env.BINDIR = ['/opt/mxe/usr/bin/', cnf.env.PREFIX+'/bin/']
		cnf.env.LIBDIR = [cnf.env.PREFIX+'/lib/']
		cnf.env.ACTUAL_LIBDIR = cnf.env.LIBDIR
		cnf.env.BINPREFIX = '/opt/mxe/usr/bin/i686-w64-mingw32.static-'

		cnf.find_program('i686-w64-mingw32.static-g++', var = 'CXX', mandatory = True, path_list = cnf.env.BINDIR)
		cnf.find_program('i686-w64-mingw32.static-gcc', var = 'CC', mandatory = True, path_list = cnf.env.BINDIR)
		cnf.find_program('i686-w64-mingw32.static-ar', var = 'AR', mandatory = True, path_list = cnf.env.BINDIR)

	cnf.load('compiler_c compiler_cxx')
	cnf.check_cfg(path=cnf.env.BINPREFIX + 'sdl2-config', args='--cflags --libs',
				package='', uselib_store='SDL2')

	cnf.check_cfg(args='--cflags --libs', package='lua5.2', uselib_store='lua')
	cnf.check_cfg(args='--cflags --libs', package='bullet', uselib_store='bullet')

def build(bld):
	sflags = ['-O2', '-g', '-std=c++11', '-Wall', '-Wextra']
	includes = ['.']
	libs = []

	if bld.env.DEST_OS.find("linux") >= 0:
		libs += ['dl', 'GL']
	elif bld.env.DEST_OS.find("win32") >= 0 or bld.env.WINDOWS_BUILD:
		libs += ['opengl32']

	# Build dependencies
	bld.objects(
		target		=	'ext',
		# source	=	bld.path.ant_glob("ext/*.c*"),
		source		=	bld.path.ant_glob("ext/stb_stub.c"),
		features	=	'c'
	)

	bld.recurse("lua-synth")

	bld.program(
		target		=	'voi',
		source		=	bld.path.ant_glob("*.cpp"),
		features	=	'cxx cxxprogram',

		includes	=	includes,
		lib			=	libs,
		libpath		=	bld.env.ACTUAL_LIBDIR,

		use			=	'SDL2 ext lua bullet synth',

		cxxflags	=	sflags
	)

def run(ctx):
	args = [
		"window.fullscreen=false"
	]

	subprocess.Popen(["build/voi"] + args)
	# subprocess.Popen(["optirun", "build/voi"] + args)

def package(ctx):
	# suffix = ''
	# if ctx.env.WINDOWS_BUILD:
	# 	suffix = '.exe'

	files = []
	files += ['scripts/test5.lua', 'data/cursor.png', 'data/cursor2.png', 'Testing/test5.voi', 'voi.cfg']

	# ctx.exec_command('mkdir __voi_package')
	# for f in files 
	# ctx.exec_command('cp %s __voi_package' % f)

	ctx.exec_command('tar -caf package.tar.gz %s' % ' '.join(str(x) for x in files))
	ctx.exec_command('tar -raf ../package.tar.gz voi.exe', cwd = 'build')
