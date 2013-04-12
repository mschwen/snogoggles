#include <RAT/DS/Root.hh>
using namespace RAT;

#include <iostream>
#include <iomanip>
#include <sstream>
#include <ctime>
using namespace std;

#include <Viewer/OrcaThread.hh>
#include <Viewer/DataStore.hh>
#include <Viewer/Semaphore.hh>
using namespace Viewer;
#include <Viewer/RIDS/Event.hh>

#include <ORViewerProcessor.h>

OrcaThread::OrcaThread( const std::string& port, 
                                Semaphore& semaphore )
  : fSemaphore( semaphore ), fPort( port ), fNumReceivedEvents(0)
{

}

OrcaThread::~OrcaThread()
{
  delete fOrca;
}

void
OrcaThread::Initialise()
{
  ORLogger::SetSeverity(ORLogger::kDebug);
  bool keepAliveSocket = true;
  unsigned int reconnectAttempts = 0; // default reconnect tries for sockets.
  unsigned int portToListenOn = 0;
  unsigned int maxConnections = 5; // default connections accepted by server
  unsigned long timeToSleep = 10; //default sleep time for sockets.
    
  string readerArg = fFileName;
  size_t iColon = readerArg.find(":");
  if (iColon == string::npos) {
    fOrcaFile = new ORFileReader;
    ((ORFileReader*) fOrcaFile)->AddFileToProcess(fFileName);
  }
  else {
    unsigned int num_tries = 0;
    if(keepAliveSocket) {
      char buff[64];
      if (reconnectAttempts) {
        sprintf(buff, "%d", (int)reconnectAttempts);
      }
      else {
        strcpy(buff, "infinite");
      }
      cout << "Setting Socket to stay alive: " << endl;
      cout << "Sleep time: " << (int)timeToSleep << " Reconnection attempts: " << buff << endl;
    }
    do {
      fOrcaFile = new ORSocketReader(readerArg.substr(0, iColon).c_str(),
      atoi(readerArg.substr(iColon+1).c_str()));
      if (fOrcaFile->OKToRead()) break;
      delete fOrcaFile;
      num_tries++;
      sleep(timeToSleep);
    } while (keepAliveSocket && (reconnectAttempts == 0 || num_tries < reconnectAttempts));
  }
  InitialiseRIDS();
}

void
OrcaThread::InitialiseRIDS()
{
  vector< pair< string, vector< string > > > dataNames;
  vector<string> uncalTypes; uncalTypes.push_back( "TAC" ); uncalTypes.push_back( "QHL" ); uncalTypes.push_back( "QHS" ); uncalTypes.push_back( "QLX" );
  dataNames.push_back( pair< string, vector< string > >( "UnCal", uncalTypes ) );
  RIDS::Event::Initialise( dataNames );
}

void
OrcaThread::Run()
{
  if(fOrcaFile->OKToRead()) {
// RAT::DS::Root* ds = new RAT::DS::Root( *reinterpret_cast<RAT::DS::Root*>( record ) );
// DataStore& events = DataStore::GetInstance();
// RIDS::Event* rids = new RIDS::Event();
// events.Add( rids );
// delete rids;
    std::cout << "OK to read!" <<std::endl;
    ORDataProcManager dataProcManager(fOrcaFile);
    ORViewerProcessor orcaViewer;
// orcaViewer.SetReader(this);
    dataProcManager.AddProcessor(&orcaViewer);
    dataProcManager.ProcessDataStream();
  }
  else {
    std::cout <<"Not OK to read :(" << std::endl;
    OrcaThread::Initialise();
  }
}
