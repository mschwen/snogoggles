import os

# Appends Xerces-C++
def xercesc(env):
    if "XERCESCROOT" in os.environ:
        env.Append( CPPPATH = [ os.environ["XERCESCROOT"] + "/include" ] )
        env.Append( LIBPATH = [ os.environ["XERCESCROOT"] + "/lib" ] )
    env.Append( LIBS = ['xerces-c'] )

# Appends Geant4 and CLHEP
def geant4(env):
    env.ParseConfig( "geant4-config --cflags --libs" )
    env.ParseConfig( "clhep-config --include --libs" )
    env.Append( LIBS=['Xm'] ) # Required for some reason??

# Appends ROOT
def root(env):
    ROOTSYS = os.path.join(os.environ["ROOTSYS"], 'bin')
    env.ParseConfig( os.path.join(ROOTSYS, 'root-config') + " --cflags --ldflags --libs ")
    env.Append( CPPPATH = [ os.environ["ROOTSYS"] + "/include"])
    env.Append( LIBS = 'PyROOT' )
  
# Appends SFML and GLEW
def sfml(env):
    env.Append( CPPPATH = [ os.environ["SFMLROOT"] + "/include", os.environ["GLEWROOT"] + "/include" ] )
    env.Append( LIBPATH = [ os.environ["SFMLROOT"] + "/lib", os.environ["GLEWROOT"] + "/lib" ] )
    env.Append( LIBS = [ 'sfml-graphics', 'sfml-window', 'sfml-system', 'GLEW' ] )
    # Need to put an "If APPLE" wrapper around this
    if os.uname()[0] == 'Darwin':
        env.Append(LINKFLAGS=['-framework', 'OpenGL'])

# Appends RAT
def rat(env):
    env.Append( CPPPATH = [ os.environ["RATROOT"] + "/include" ] )
    env.Append( LIBPATH = [ os.environ["RATROOT"] + "/lib" ] )
    env.Append( LIBS = [ 'RATEvent_' + os.environ["RATSYSTEM"] ] )
    Curl(env)

# Appends Curl and Bzip (for RAT)
def Curl(env):
    env.Append( LIBS = [ "bz2" ] )
    if "BZIPROOT" in os.environ:
        env['CPPPATH'].append( os.environ['BZIPROOT'] + "/include" )
        env['LIBPATH'].append( os.environ['BZIPROOT'] + "/lib" )
    env.ParseConfig( "curl-config --cflags --libs" )

def glut(env):
    env.Append( LIBPATH="/usr/X11/lib" )
    # Need to put an "If APPLE" wrapper around this
    if os.uname()[0] == 'Darwin':
        env.Append( LINKFLAGS=['-framework', 'glut'])
    env.Append( LIBS = ["glut", "GLU", "GL"])

# Appends Pthread
def PThread(env):
    env.Append( LIBS = [ "pthread" ] )

#Append the OrcaRoot Raw data converter
def OrcaRoot(env):
    orcaroot_path = os.path.join(os.environ['ORDIR'], "")
    env.Append(CPPPATH = [os.path.join(orcaroot_path, "")])
    env.Append(LIBPATH = [os.path.join(orcaroot_path, "lib")])
    env.Append(LIBPATH = ["/home/mschwen/snoing/install/snogoggles-dev/src/Thread/"])
    env.Append(LIBS = ["ORDecoders", "ORIO", "ORManagement", "ORProcessors", "ORUtil", "ORViewerProcessor"])

# Append the rattools zdab convertor
def ratzdab(env):
    ratzdab_path = os.path.join(os.environ['RATTOOLS'], "ratzdab")
    env.Append(CPPPATH = [os.path.join(ratzdab_path, "src")])
    env.Append(LIBPATH = [os.path.join(ratzdab_path, "lib")])
    env.Append(LIBS = ["zdispatch", "zconvert", "zfile"])
    env.Append(CPPFLAGS='-D__ZDAB')

# Append Avalanche and zmq
def Avalanche(env):
    avalancheLibPath = os.environ['AVALANCHEROOT']
    env.Append( CPPPATH = [ os.environ['AVALANCHEROOT'] + "/src" ] )
    env.Append( LIBPATH = [ os.environ['AVALANCHEROOT'] + "/lib" ] )
    env.Append( LIBS = ["avalanche"] )

# Append Python libraries
def Python(env):
    ldflags = env.backtick( "python-config --includes").split() # Split on space, diff flags
    ldflags.extend( env.backtick( "python-config --libs").split() )
    ldflags.extend( env.backtick( "python-config --ldflags").split() )
    # Put all options after -u in LINKFLAGS, may not have a -u part though
    try: 
        idx = ldflags.index('-u') 
        env.Append( LINKFLAGS=ldflags[idx:] ) 
        del ldflags[idx:] # Remove the -u part
    except ValueError: 
        idx = -1 
    env.MergeFlags( ' '.join( ldflags ) ) 
    
# Adds all packages
def addpackages(env, zdab, orca):
    rat(env)
    glut(env)    
    geant4(env)
    root(env)
    sfml(env)
    Python(env)
    xercesc(env)
    PThread(env)
    if orca: 
        OrcaRoot(env)
    if zdab:
        ratzdab(env)
    #Avalanche(env)


