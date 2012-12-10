#include <RAT/DS/EV.hh>

#include <vector>
using namespace std;

#include <Viewer/RIDS/EV.hh>
using namespace Viewer::RIDS;

EV::EV( UInt_t* iMtcRecord , 
        std::vector<double> fViewerData1,
        std::vector<double> fViewerData2,
        std::vector<double> fViewerData3,
        std::vector<double> fViewerData4) 
{
  fClock50 = 23;
  fTriggerWord = 21;
  for( unsigned int ihit = 0; ihit < fViewerData1.size(); ihit++) 
  {
    fTruthHits.push_back(PMTHit(ihit,(double)fViewerData1[ihit],(double)fViewerData2[ihit],(double)fViewerData3[ihit],(double)fViewerData4[ihit]));
//    fTruthHits.push_back(PMTHit(0,0,0,0));
  }
}

EV::EV( RAT::DS::EV& rEV )
{
  fClock50 = rEV.GetClockCount50();
  fTriggerWord = rEV.GetTrigType();
  fGTID = rEV.GetEventID();
  for( unsigned int ipmt = 0; ipmt < rEV.GetPMTAllTruthCount(); ipmt++ )
    fTruthHits.push_back( PMTHit( rEV.GetPMTAllTruth( ipmt ) ) );
  for( unsigned int ipmt = 0; ipmt < rEV.GetPMTAllUnCalCount(); ipmt++ )
    fUnCalHits.push_back( PMTHit( rEV.GetPMTAllUnCal( ipmt ) ) );
  for( unsigned int ipmt = 0; ipmt < rEV.GetPMTAllCalCount(); ipmt++ )
    fCalHits.push_back( PMTHit( rEV.GetPMTAllCal( ipmt ) ) );
  fTime = Time( rEV.GetClockCount10() );
}

EV::~EV()
{
  
}

vector<PMTHit>
EV::GetHitData( EDataSource source ) const
{
  switch( source )
    {
    case eMC:
    case eScript:
      // This should not happen....
      break;
    case eTruth:
      return fTruthHits;
    case eUnCal:
      return fUnCalHits;
    case eCal:
      return fCalHits;
    }
  // Return empty vector if get here...
  return vector<PMTHit>();
}
