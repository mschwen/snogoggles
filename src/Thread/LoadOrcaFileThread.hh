////////////////////////////////////////////////////////////////////////
/// \class LoadORCAFileThread
///
/// \brief   Loads Raw ORCA files
///
/// \author  Mike Schwendener (mschwen@snolab.ca)
///
/// REVISION HISTORY:\n
///     20/10/12 : Mike Schwendener
///   - First Revision, clone of LoadZdabFileThread. \n
///
/// \detail  Load unbuilt PMT and MTC data and stuff integrated events
///
////////////////////////////////////////////////////////////////////////
#ifndef __Viewer_LoadOrcaFileThread__
#define __Viewer_LoadOrcaFileThread__

#include <TTree.h>
#include <TFile.h>

#include <string>

#include <Viewer/Thread.hh>

#include <ORFileReader.hh>

namespace RAT
{
namespace DS
{
  class Root;
  class Run;
}
}


namespace Viewer
{
  class Semaphore;

class LoadOrcaFileThread : public Thread
{
public:
  inline LoadOrcaFileThread( const std::string& fileName, Semaphore& semaphore );
  
  virtual ~LoadOrcaFileThread() {};
  
  virtual void
  Run();
private:
  void LoadRootFile();
  void LoadOrcaFile();
  bool LoadNextEvent();

  std::string fFileName;
  
  int fMCEvent;
  Semaphore& fSemaphore;

  /// Main ORCA file to load from
  ORFileReader* fOrcaFile;

  /// Root file part for the PMTProperties 
  TFile* fFile;
  TTree* fTree;
  TFile* fRootFile;
  TTree* fRunTree;
  RAT::DS::Root* fDS;
  RAT::DS::Run* fRun;
};

LoadOrcaFileThread::LoadOrcaFileThread( const std::string& fileName, Semaphore& semaphore )
  : fSemaphore( semaphore ), fFileName( fileName )
{ 
  fTree = NULL;
  fDS = NULL;
  fFile = NULL; 
  fRootFile = NULL;
  fRun = NULL; 
  fMCEvent = 0; 
} 


} //::Viewer

#endif
