////////////////////////////////////////////////////////////////////////
/// \class LoadORCAFileThread
///
/// \brief Loads Raw ORCA files
///
/// \author Mike Schwendener (mschwen@snolab.ca)
///
/// REVISION HISTORY:\n
/// 20/10/12 : Mike Schwendener
/// - First Revision, clone of LoadZdabFileThread. \n
///
/// \detail Load unbuilt PMT and MTC data and stuff integrated events
///
////////////////////////////////////////////////////////////////////////
#ifndef __Viewer_OrcaThread__
#define __Viewer_OrcaThread__

#include <TTree.h>
#include <TFile.h>

#include <string>
#include <Viewer/Thread.hh>
#include <Viewer/DataStore.hh>

#include <OrcaRoot/ORFileReader.hh>
#include <OrcaRoot/ORSocketReader.hh>

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
  class OrcaThread : public Thread
  {
  public:
    inline OrcaThread( const std::string& fileName, Semaphore& semaphore );
    virtual ~OrcaThread() {};
    virtual void Run();

  private:
    std::string fFileName;
    Semaphore& fSemaphore;

    /// ORCA file to load from
    ORVReader* fOrcaFile;

  /// Root file part for the PMTProperties
    TFile* fFile;
    TTree* fTree;
    TFile* fRootFile;
    TTree* fRunTree;
    RAT::DS::Root* fDS;
    RAT::DS::Run* fRun;
  };

  OrcaThread::OrcaThread( const std::string& fileName, Semaphore& semaphore )
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
