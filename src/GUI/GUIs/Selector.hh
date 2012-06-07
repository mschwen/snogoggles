////////////////////////////////////////////////////////////////////////
/// \class Viewer::GUIs::Selector
///
/// \brief   Label with a check box next to it
///
/// \author  Phil Jones <p.jones22@physics.ox.ac.uk>
///
/// REVISION HISTORY:\n
///     30/06/11 : P.Jones - First Revision, new file. \n
///     07/06/12 : P.Jones - Second Revision, composite object.\n
///
/// \detail  As brief
///
////////////////////////////////////////////////////////////////////////

#ifndef __Viewer_GUIs_Selector__
#define __Viewer_GUIs_Selector__

#include <string>

#include <Viewer/GUI.hh>
#include <Viewer/RectPtr.hh>
#include <Viewer/Text.hh>
#include <Viewer/GUIManager.hh>

namespace Viewer
{ 
  class Button;
  class RWWrapper;

namespace GUIs
{

class Selector : public GUI
{
public:
  Selector( RectPtr rect, unsigned int guiID );
  virtual ~Selector();

  void Initialise( std::vector<std::string> options );

  GUIEvent NewEvent( const Event& event );
  void Render( RWWrapper& windowApp );
  
  inline void SetState( unsigned int state );
  inline unsigned int GetState() const;
  inline std::string GetStringState() const;
protected:
  GUIManager fGUIManager;
  std::vector<std::string> fOptions;
  Button* fLeft;
  Button* fRight;
  Text fText;
  unsigned int fState;
};

void
Selector::SetState( unsigned int state )
{
  fState = state;
}

unsigned int
Selector::GetState() const
{
  return fState;
}

std::string
Selector::GetStringState() const
{
  return fOptions[fState];
}

} // ::GUIs

} // ::Viewer

#endif
