env = Environment()

# command line options
vars = Variables(None, ARGUMENTS)
vars.Add(BoolVariable('debug', "Set to build for debug", 'no'))
vars.Update(env)

# path setup
srcpath = "src"
buildpath = "build"
if env['debug']:
	buildpath = buildpath + "/debug"
else:
	buildpath = buildpath + "/release"

# environment setup
env.Append(LIBS = Split('boost_system boost_thread'))
env.Append(LINKFLAGS = ['-pthread'])
env.Append(CCFLAGS = Split('-Wall -std=c++11 -pthread'))
if env['debug']:
	env.Append(CCFLAGS = Split('-g3'))
else:
	env.Append(CCFLAGS = Split('-O3 -flto'))
	env.Append(LINKFLAGS = Split('-flto'))
env.Append(CPPPATH = ['.'])

# help output
env.Help(vars.GenerateHelpText(env))


env.Export('env')
env.SConscript(srcpath + "/SConscript", variant_dir = buildpath, duplicate = 0)
