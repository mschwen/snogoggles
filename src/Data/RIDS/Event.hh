////////////////////////////////////////////////////////////////////
/// \class Event
///
/// \brief  Event data structure
///          
/// \author Phil Jones <p.g.jones@qmul.ac.uk>
///
/// REVISION HISTORY:\n
///     07/05/12 : P.Jones - First Revision, new file. \n
///
/// \detail ROOT has many memory management issues, thus a ROOT 
///         independent data structure exists. This is the event 
///         data holder class.
///
////////////////////////////////////////////////////////////////////

#ifndef __Viewer_RIDS_Event__
#define __Viewer_RIDS_Event__

#include <vector>

#include <Viewer/RIDS/RIDS.hh>
#include <Viewer/RIDS/PMTHit.hh>
#include <Viewer/RIDS/Time.hh>
#include <Rtypes.h>
namespace RAT
{
namespace DS
{
  class Root;
  class PackedEvent;
}
}

namespace Viewer
{
namespace RIDS
{
  class MC;
  class EV;

class Event
{
public:
  Event( RAT::DS::Root& rDS, unsigned int iEV );
  Event( UInt_t* iMtcRecord  , std::vector<double> fViewerData1,  std::vector<double> fViewerData2,  std::vector<double> fViewerData3,  std::vector<double> fViewerData4);
  Event( const Event& rhs );
  Event& operator=( const Event& rhs );
  ~Event();

  bool ExistMC() const { return fMC != NULL; }
  bool ExistEV() const { return fEV != NULL; }
  MC& GetMC() const { return *fMC; }
  EV& GetEV() const { return *fEV; }
  int GetRunID() const { return fRunID; }
  int GetSubRunID() const { return fSubRunID; }
  Time GetTime() const { return fTime; }
  /// Return a vector of PMTHits (data) by source, e.g. eCal
  std::vector<PMTHit> GetHitData( EDataSource source ) const;

private:
  Event();

  Time fTime; /// < Time viewer created the event (useful for performance tracking)

  EV* fEV; /// < EV side event data
  MC* fMC; /// < Monte Carlo side event data, optional
  int fRunID; /// < Event run ID
  int fSubRunID; /// < Event sub run ID
};

} // namespace RIDS

} // namespace Viewer

#endif
