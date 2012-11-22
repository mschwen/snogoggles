#include <RAT/DS/Root.hh>
#include <RAT/DS/Run.hh>
using namespace RAT;

#include <ORFileReader.hh>
#include <ORSocketReader.hh>
#include <ORDataProcManager.hh>

#include <TTree.h>
#include <TFile.h>
using namespace ROOT;

#include <sstream>
#include <iostream>
using namespace std;

#include <Viewer/LoadRootFileThread.hh>
#include <Viewer/LoadOrcaFileThread.hh>
#include <Viewer/DataStore.hh>
#include <Viewer/Semaphore.hh>
using namespace Viewer;

void
LoadOrcaFileThread::Run()
{
  DataStore& events = DataStore::GetInstance();

  if( fTree == NULL )
    {
      LoadRootFile();
      events.SetRun( fRun );
      fTree->GetEntry( fMCEvent );

      fOrcaFile = new ORFileReader;
     ((ORFileReader*) fOrcaFile)->AddFileToProcess(fFileName);

      events.Add( fDS );
      fSemaphore.Signal();
      fMCEvent++;
      return;
    }
  if( fMCEvent >= fTree->GetEntries() )
    {
      fFile->Close();
      delete fFile;
      delete fDS;
      delete fRun;

      Kill();
    }

  else
    {
       LoadNextEvent();
    }
}

bool
LoadOrcaFileThread::LoadNextEvent()
{
  if(fOrcaFile->OKToRead()) {
    std::cout << "OK to read!" <<std::endl;
    ORDataProcManager dataProcManager(fOrcaFile);
    dataProcManager.AddProcessor(&orcaViewer);
    dataProcManager.ProcessDataStream();
  }
  else {
    std::cout <<"Not OK to read :(" << std::endl;
  }

}




//      fFile = new ORFileReader;
//     ((ORFileReader*) fFile)->AddFileToProcess(fFileName);
//      LoadNextEvent();
//      events.SetRun( fRun );
//      fSemaphore.Signal();
//      fMCEvent++;
//      return;
//    }
//  bool success = LoadNextEvent();
//  fMCEvent++;
//  cout << "Loaded " << fMCEvent << " events." << endl;  
//  if( !success )
//    {
//      delete fFile;
//      fRootFile->Close();
//      delete fRootFile;
//      delete fRun;
//     Kill();
//    }
//}

//bool
//LoadOrcaFileThread::LoadNextEvent()
//{
//#ifdef __ORCA
//  if(fFile->OKToRead()) {
//    std::cout << "OK to read!" <<std::endl;
//  }
//  else {
//    std::cout <<"Not OK to read :(" << std::endl;
//  }
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
//}

void
LoadOrcaFileThread::LoadRootFile()
{
  fFile = new TFile( "/home/mschwen/Temp.root", "READ" );

  fTree = (TTree*)fFile->Get( "T" );
  fDS = new RAT::DS::Root();
  fTree->SetBranchAddress( "ds", &fDS );

  fRunTree = (TTree*)fFile->Get( "runT" );
  fRun = new RAT::DS::Run();
  fRunTree->SetBranchAddress( "run", &fRun );
  fRunTree->GetEntry();
}

