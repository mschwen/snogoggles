#include <RAT/DS/Root.hh>
#include <RAT/DS/Run.hh>
using namespace RAT;

//#ifdef __ORCA
#include <ORFileReader.hh>
#include <ORSocketReader.hh>
//using namespace OrcaRoot;
//#endif

#include <TTree.h>
#include <TFile.h>
using namespace ROOT;

#include <sstream>
#include <iostream>
using namespace std;

#include <Viewer/LoadOrcaFileThread.hh>
#include <Viewer/DataStore.hh>
#include <Viewer/Semaphore.hh>
using namespace Viewer;

void
LoadOrcaFileThread::Run()
{
#ifdef _ORCA_
  DataStore& events = DataStore::GetInstance();

  if( fFile == NULL )
    {
      LoadRootFile();
      fFile = new ORFileReader;
      ((ORFileReader*) fFile)->AddFileToProcess(fFileName);

      LoadNextEvent();
      events.SetRun( fRun );
      fSemaphore.Signal();
      fMCEvent++;
      return;
    }
  bool success = LoadNextEvent();
  fMCEvent++;
  cout << "Loaded " << fMCEvent << " events." << endl;  
  if( !success )
    {
      delete fFile;
      fRootFile->Close();
      delete fRootFile;
      delete fRun;
      Kill();
    }
#else
  Kill();
#endif
}

bool
LoadOrcaFileThread::LoadNextEvent()
{
//#ifdef __ORCA
  if(fFile->OKToRead()) {
    std::cout << "OK to read!" <<std::endl;
  }
  else {
    std::cout <<"Not OK to read :(" << std::endl;
  }
/*  try
    {
      TObject* record = fFile->next();
      if( record == NULL )
        return false;
      if( record->IsA() == RAT::DS::Root::Class() ) 
        {
          RAT::DS::Root* ds = new RAT::DS::Root( *reinterpret_cast<RAT::DS::Root*>( record ) );
          DataStore& events = DataStore::GetInstance();
          events.Add( ds );
          delete ds;
          return true;
        }
      else // Iterate through until an event is located
        return LoadNextEvent();
    }
  catch( ratzdab::unknown_record_error& e )
    {
      return LoadNextEvent(); // Carry on and try again...
    }*/
//#endif
}

void
LoadOrcaFileThread::LoadRootFile()
{
  stringstream fileLocation;
  fileLocation << getenv( "VIEWERROOT" ) << "/Temp.root";
  fRootFile = new TFile( fileLocation.str().c_str(), "READ" );
 
  fRunTree = (TTree*)fRootFile->Get( "runT" ); 
  fRun = new RAT::DS::Run();
  fRunTree->SetBranchAddress( "run", &fRun );
  fRunTree->GetEntry();
}
