#include <RAT/DS/Root.hh>

#include <ctime>
using namespace std;

#include <Viewer/RIDS/Event.hh>
#include <Viewer/RIDS/EV.hh>
#include <Viewer/RIDS/MC.hh>
using namespace Viewer::RIDS;

Event::Event(std::vector<double> fViewerTruthData1,
             std::vector<double> fViewerTruthData2,
             std::vector<double> fViewerTruthData3,
             std::vector<double> fViewerTruthData4,
             std::vector<double> fViewerCalData1,
             std::vector<double> fViewerCalData2,
             std::vector<double> fViewerCalData3,
             std::vector<double> fViewerCalData4,
             std::vector<double> fViewerUncalData1,
             std::vector<double> fViewerUncalData2,
             std::vector<double> fViewerUncalData3,
             std::vector<double> fViewerUncalData4)
{
  fMC = NULL;
  fEV = NULL;
  fEV = new EV(fViewerTruthData1,
               fViewerTruthData2,
               fViewerTruthData3,
               fViewerTruthData4,
               fViewerCalData1,
               fViewerCalData2,
               fViewerCalData3,
               fViewerCalData4,
               fViewerUncalData1,
               fViewerUncalData2,
               fViewerUncalData3,
               fViewerUncalData4);
  fRunID = 23;
  fSubRunID = 1;
  time_t now = time(0);
  struct tm* tm = localtime( &now );
  fTime = Time( tm );
}

Event::Event( RAT::DS::Root& rDS,
              unsigned int iEV )
{
  fMC = NULL;
  fEV = NULL;
  if( rDS.ExistMC() )
    fMC = new MC( *rDS.GetMC() );
  else
    fMC = NULL;
  if( rDS.GetEVCount() > iEV )
    fEV = new EV( *rDS.GetEV( iEV ) );
  else
    fEV = NULL;

  fRunID = rDS.GetRunID();
  fSubRunID = rDS.GetSubRunID();

  time_t now = time(0);
  struct tm* tm = localtime( &now );
  fTime = Time( tm );
}

Event::Event( const Event& rhs )
{
  fMC = NULL;
  fEV = NULL;
  *this = rhs;
}

Event& 
Event::operator=( const Event& rhs )
{
  if( &rhs == this )
    return *this;
  delete fMC;
  delete fEV;
  fMC = NULL;
  fEV = NULL;
  if( rhs.ExistMC() )
    fMC = new MC( *rhs.fMC );
  if( rhs.ExistEV() )
    fEV = new EV( *rhs.fEV );
  fRunID = rhs.fRunID;
  fSubRunID = rhs.fSubRunID;
  fTime = rhs.fTime;
  return *this;
}

Event::~Event()
{
  delete fMC;
  delete fEV;
}

vector<PMTHit> 
Event::GetHitData( EDataSource source ) const
{
  switch( source )
    {
    case eMC:
      if( ExistMC() )
        return fMC->GetHitData();
      break;
    case eTruth:
    case eUnCal:
    case eCal:
      if( ExistEV() )
        return fEV->GetHitData( source );
      break;
    case eScript:
      break;
    }
  // Return empty vector if get here...
  return vector<PMTHit>();
}
