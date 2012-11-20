////////////////////////////////////////////////////////////////////////
/// \file SNOGoggles
///
/// \brief   Entry point to SNOGoggles viewer
///
/// \author  Phil Jones <p.g.jones@qmul.ac.uk>
///
/// REVISION HISTORY:\n
///     29/06/11 : P.Jones - First Revision, new file. \n
///     23/05/12 : P.Jones - Second Revision, New initialisation order.\n
///
/// \detail  The main function is defined here.
///
////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <getopt.h>
using namespace std;

#include <Viewer/ViewerWindow.hh>
#include <Viewer/Semaphore.hh>
#include <Viewer/LoadRootFileThread.hh>
#include <Viewer/LoadZdabFileThread.hh>
#include <Viewer/LoadOrcaFileThread.hh>
#include <Viewer/ReceiverThread.hh>
#include <Viewer/ConfigurationFile.hh>
#include <Viewer/ConfigurationTable.hh>

#include <Viewer/RenderState.hh>
#include <Viewer/GeodesicSphere.hh>
#include <Viewer/DataStore.hh>
#include <Viewer/GUIProperties.hh>
using namespace Viewer;

#include <xercesc/util/PlatformUtils.hpp>
using namespace xercesc;

const int kConfigVersion = 1; /// The configuration version expected in the xml config files.

class CmdOptions
{
public:
  CmdOptions() : fStream( false ) { };

  bool fStream; /// < Are the events dispatched via avalanche to snogoggles?
  std::string fArgument; /// < The argument, url or fileName  
  std::string fConfigFile; /// The config file location
};
/// Parse the command options
CmdOptions ParseArguments( int argc, char *argv[] );
/// Print the help information to the terminal
void PrintHelp();

/// Load the singletons that can be loaded without the DataStore class.
void PreInitialise();
/// Load the singletons that require event data/the DataStore class.
void PostInitialise();
/// Load the configuration, if it exists and is the correct version
ConfigurationFile* OpenConfiguration( bool output, const CmdOptions& options );

int main( int argc, char *argv[] )
{
  CmdOptions options = ParseArguments( argc, argv );
  PreInitialise();
  ConfigurationFile* loadConfig = OpenConfiguration( false, options );
  const ConfigurationTable* loadConfigTable = NULL;
  if( loadConfig != NULL )
    loadConfigTable = loadConfig->GetTable();
  GUIProperties::GetInstance().PreInitialise( loadConfigTable );
  RenderState::Initialise(); // Must be after GUIProperties
  ViewerWindow& viewer = ViewerWindow::GetInstance();
  viewer.PreInitialise( loadConfigTable );
  Thread* loadData;
  if( options.fStream )
    {
      Semaphore sema;
      loadData = new ReceiverThread( options.fArgument, sema );
      // Wait for first event to be loaded
      sema.Wait();
    }
  else
    {
      Semaphore sema;
      if( options.fArgument.substr( options.fArgument.size() - 4 ) == string( "root" ) )
        loadData = new LoadRootFileThread( options.fArgument, sema );
      else if ( options.fArgument.substr( options.fArgument.size() - 4 ) == string( "4882" ) )
        loadData = new LoadOrcaFileThread( options.fArgument, sema );
      else
        {
          loadData = new LoadZdabFileThread( options.fArgument, sema );
#ifndef __ZDAB
          cout << "Cannot load ZDAB files, scons zdab=1 will solve this - unless on a Mac." << endl;
          exit(1);
#endif
        }
      // Wait for first event to be loaded
      sema.Wait();
    }
  PostInitialise();
  viewer.PostInitialise( loadConfigTable );
  delete loadConfig; // No longer needed
  viewer.Run();
  ConfigurationFile* saveConfig = OpenConfiguration( true, options );
  viewer.SaveConfiguration( saveConfig->NewTable() );
  saveConfig->Save();
  delete saveConfig;
  viewer.Destruct();
  // Finish the thread
  loadData->KillAndWait();
  delete loadData;
  GUIProperties::GetInstance().Destruct();
  XMLPlatformUtils::Terminate();
  return 0;
}

void
PreInitialise()
{
  XMLPlatformUtils::Initialize();
  DataStore::GetInstance();
}

void 
PostInitialise()
{
  GeodesicSphere::GetInstance(); // Forces it to load, should be initialised, PHIL
  DataStore::GetInstance().Initialise();
}

CmdOptions 
ParseArguments( int argc, char** argv )
{
  static struct option opts[] = { {"help", 0, NULL, 'h'}, {"stream", 2, NULL, 's'}, {"config", 1, NULL, 'c'}, {0,0,0,0} };
  CmdOptions options;
  int option_index = 0;
  int c = getopt_long(argc, argv, "o:s::hc:", opts, &option_index);
  while (c != -1) 
    {
      switch (c) 
        {
        case 'h': PrintHelp(); exit(0); break;
        case 'o': 
          {
            options.fStream = false;
            options.fArgument = "Run4882";    
          }      
        break;
        case 's': 
          {
            options.fStream = true; 
            if( optarg != NULL )
              {
                options.fArgument = optarg;
                options.fArgument.erase( 0, 1 ); 
              }
            else
              {
                cout << "Enter the stream location:" << endl;
                cin >> options.fArgument;
              }
          } 
          break;
        case 'c': options.fConfigFile = optarg; break;
        }
      c = getopt_long(argc, argv, "o:s::hc", opts, &option_index);
    }
  if( option_index >= argc || argc == 1 )
    {
      PrintHelp();
      exit(1);
    }
  if( options.fArgument == "" )
    {
      options.fArgument = argv[optind];
    }
  return options;
}

void 
PrintHelp()
{
  cout << "usage:snogoggle FileName.root" << " or: snogoggles -s=address\n";
  cout << "options:" << endl;
  cout << " -h        show this help message and exit" << endl;
  cout << " -s=addr   connect to a zdab dispatcher at addr" << endl;
  cout << " -c path   use the configuration script at path" << endl;
  cout << " -o FileName.ORCA  load a raw orca file" << endl;
}

ConfigurationFile*
OpenConfiguration( bool output, 
                   const CmdOptions& options )
{
  stringstream configFileName;
  if( options.fConfigFile.empty() )
    configFileName << getenv( "VIEWERROOT" ) << "/snogoggles.xml";
  else
    configFileName << options.fConfigFile;
  if( output == true )
    {
      ConfigurationFile* saveConfig = new ConfigurationFile( configFileName.str(), true );
      saveConfig->SetVersion( kConfigVersion );
      return saveConfig;
    }
  else
    {
      try
        {
          ConfigurationFile* loadConfig = new ConfigurationFile( configFileName.str(), false );
          // Check file is up to date, if not throw a recoverable error (after deleting it)
          if( loadConfig->GetVersion() != kConfigVersion )
            {
              remove( configFileName.str().c_str() );
              throw ConfigurationFile::NoFileError( "Configuration version miss match" );
            }
          return loadConfig;
        }
      catch( ConfigurationFile::NoFileError& e )
        {
          cout << "No compatible configuration file found." << endl;
          return NULL;
        }
      return NULL;
    }
}
