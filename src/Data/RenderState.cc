using namespace std;

#include <Viewer/RenderState.hh>
#include <Viewer/ConfigurationTable.hh>
#include <Viewer/GUIProperties.hh>
#include <Viewer/RIDS/PMTHit.hh>
using namespace Viewer;

map< RIDS::EDataSource, string > RenderState::fsSourceNames;
map< RIDS::EDataType, string > RenderState::fsTypeNames;
map< RIDS::EDataSource, map< RIDS::EDataType, double > > RenderState::fsDefaultScalingMin;
map< RIDS::EDataSource, map< RIDS::EDataType, double > > RenderState::fsDefaultScalingMax;

void
RenderState::Initialise()
{
  fsTypeNames[RIDS::eTAC] = string("TAC_max");
  fsTypeNames[RIDS::eQHL] = string("QHL_sum");
  fsTypeNames[RIDS::eQHS] = string("QHS_sum");
  fsTypeNames[RIDS::eQLX] = string("QLX_sum");
  fsSourceNames[RIDS::eMC] = string("MC");
  fsSourceNames[RIDS::eTruth] = string("Data");
  fsSourceNames[RIDS::eUnCal] = string("Detector");
  fsSourceNames[RIDS::eCal] = string("Slots");
  fsSourceNames[RIDS::eScript] = string("Script");

  const ConfigurationTable* renderConfig = GUIProperties::GetInstance().GetConfiguration( "RenderState" );

  for( map< RIDS::EDataSource, string >::const_iterator sourceTer = fsSourceNames.begin(); sourceTer != fsSourceNames.end(); sourceTer++ )
    {
      const ConfigurationTable* configTable = renderConfig->GetTable( sourceTer->second );
      for( map< RIDS::EDataType, string >::const_iterator typeTer = fsTypeNames.begin(); typeTer != fsTypeNames.end(); typeTer++ )
        {
          fsDefaultScalingMin[sourceTer->first][typeTer->first] = 0.0;
          fsDefaultScalingMax[sourceTer->first][typeTer->first] = configTable->GetD( typeTer->second );
        }
    }
}

void
RenderState::ChangeState( RIDS::EDataSource source,
                          RIDS::EDataType type )
{
  fCurrentDataSource = source;
  fCurrentDataType = type;
  fCurrentScalingMin = fsDefaultScalingMin[source][type];
  fCurrentScalingMax = fsDefaultScalingMax[source][type];
  fChanged = true;
}

void 
RenderState::ChangeScaling( double fractionalMin,
                            double fractionalMax )
{
  fCurrentScalingMin = fractionalMin * ( fsDefaultScalingMax[fCurrentDataSource][fCurrentDataType] - fsDefaultScalingMin[fCurrentDataSource][fCurrentDataType] );
  fCurrentScalingMax = fractionalMax * ( fsDefaultScalingMax[fCurrentDataSource][fCurrentDataType] - fsDefaultScalingMin[fCurrentDataSource][fCurrentDataType] );
  fChanged = true;
}

vector<string>
RenderState::GetSourceStrings()
{
  vector<string> strings;
  for( map< RIDS::EDataSource, string>::iterator iTer = fsSourceNames.begin(); iTer != fsSourceNames.end(); iTer++ )
    strings.push_back( iTer->second );
  return strings;
}

vector<string>
RenderState::GetTypeStrings()
{
  vector<string> strings;
  for( map< RIDS::EDataType, string>::iterator iTer = fsTypeNames.begin(); iTer != fsTypeNames.end(); iTer++ )
    strings.push_back( iTer->second );
  return strings;
}

Colour
RenderState::GetDataColour( double data ) const
{
  return GUIProperties::GetInstance().GetColourPalette().GetColour( ( data - fCurrentScalingMin ) / ( fCurrentScalingMax - fCurrentScalingMin ) );
}

Colour
RenderState::GetHitColour( const RIDS::PMTHit& pmtHit ) const
{
  return GetDataColour( pmtHit.GetData( fCurrentDataType ) );
}
