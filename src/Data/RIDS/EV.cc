#include <RAT/DS/EV.hh>

#include <vector>
using namespace std;

#include <Viewer/RIDS/EV.hh>
using namespace Viewer::RIDS;

EV::EV( std::vector<double> fViewerTruthData1,
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
  fClock50 = 23;
  fTriggerWord = 21;
  for( unsigned int ihit = 0; ihit < fViewerTruthData1.size(); ihit++) 
  {
    fTruthHits.push_back(PMTHit(ihit,(double)fViewerTruthData1[ihit],
       (double)fViewerTruthData2[ihit],
       (double)fViewerTruthData3[ihit],
       (double)fViewerTruthData4[ihit]));
    fCalHits.push_back(PMTHit(ihit,(double)fViewerCalData1[ihit],
       (double)fViewerCalData2[ihit],
       (double)fViewerCalData3[ihit],
       (double)fViewerCalData4[ihit]));
    fUnCalHits.push_back(PMTHit(ihit,(double)fViewerUncalData1[ihit],
       (double)fViewerUncalData2[ihit],
       (double)fViewerUncalData3[ihit],
       (double)fViewerUncalData4[ihit]));
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
