#include <SFML/Window/Event.hpp>

#include <Viewer/Frame.hh>
using namespace Viewer;

void 
Frame::RenderGUI( sf::RenderWindow& windowApp )
{
  fGUIManager.Render( windowApp, fFrameCoord );
}

void 
Frame::NewEvent( sf::Event& event )
{

}
   
void 
Frame::Initialise( ConfigurationTable& configTable )
{

}

void 
Frame::SaveConfiguration( ConfigurationTable& configTable )
{

}
