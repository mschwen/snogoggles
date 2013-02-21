////////////////////////////////////////////////////////////////////
/// \class EV
///
/// \brief  EV data structure
///          
/// \author Phil Jones <p.g.jones@qmul.ac.uk>
///
/// REVISION HISTORY:\n
///     07/05/12 : P.Jones - First Revision, new file. \n
///
/// \detail ROOT has many memory management issues, thus a ROOT 
///         independent data structure exists. This is the EV side 
///         data holder class.
///
////////////////////////////////////////////////////////////////////

#ifndef __Viewer_RIDS_EV__
#define __Viewer_RIDS_EV__

#include <vector>

#include <Viewer/RIDS/PMTHit.hh>
#include <Viewer/RIDS/Time.hh>

namespace RAT
{
namespace DS
{
  class EV;
}
}

namespace Viewer
{
namespace RIDS
{

class EV
{
public:
  EV(   std::vector<double> fViewerTruthData1,
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
        std::vector<double> fViewerUncalData4); 
  EV( RAT::DS::EV& rEV );
  ~EV();

  int GetGTID() const { return fGTID; }
  int GetCalNHits() const { return fCalHits.size(); }
  int GetUnCalNHits() const { return fUnCalHits.size(); }
  int GetTruthNHits() const { return fTruthHits.size(); }
  int GetTriggerWord() const { return fTriggerWord; }
  Time GetTime() const { return fTime; }
  /// Return a vector of PMTHits (data) by source, e.g. eCal
  std::vector<PMTHit> GetHitData( EDataSource source ) const;

private:
  EV();
  
  std::vector<PMTHit> fCalHits;
  std::vector<PMTHit> fUnCalHits;
  std::vector<PMTHit> fTruthHits;
  
  Time fTime; /// < The event time

  unsigned long long fClock50;
  int fTriggerWord;
  int fGTID;
};

} // namespace RIDS

} // namespace Viewer

#endif
