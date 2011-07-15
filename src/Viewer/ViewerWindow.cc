#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>

#include <string>
#include <iostream>
using namespace std;

#include <Viewer/ViewerWindow.hh>
#include <Viewer/ImageManager.hh>
#include <Viewer/Configuration.hh>
#include <Viewer/ConfigurationTable.hh>
#include <Viewer/Coord.hh>
#include <Viewer/Rect.hh>
#include <Viewer/UIEvent.hh>
#include <Viewer/RainbowPalette.hh>
using namespace Viewer;

ViewerWindow* ViewerWindow::fViewer = NULL;

ViewerWindow::ViewerWindow()
{
  ColourPalette::gPalette = new RainbowPalette();
}

void
ViewerWindow::Initialise()
{
  // Load the configuration
  try
    {
      stringstream configFileName;
      configFileName << getenv( "VIEWERROOT" ) << "/snogoggles.xml";
      Configuration loadConfig( configFileName.str(), false );  
      int resX = loadConfig.GetI( "resX" ); 
      int resY = loadConfig.GetI( "resY" );
      Coord::SetWindowResolution( resX, resY );
      Coord::SetWindowSize( resX, resY );
      fWindowApp = new sf::RenderWindow( sf::VideoMode( resX, resY ), "SNO Goggles" );
      DrawSplash();
      fFrameManager.Initialise();
      fFrameManager.LoadConfiguration( loadConfig );
    }
  catch( Configuration::NoFileError& e )
    {
      sf::VideoMode fullScreen = sf::VideoMode::GetDesktopMode();
      fWindowApp = new sf::RenderWindow( sf::VideoMode::GetDesktopMode(), "SNO Goggles" ); 
      DrawSplash();
      Coord::SetWindowResolution( fullScreen.Width, fullScreen.Height );
      Coord::SetWindowSize( fullScreen.Width, fullScreen.Height );
      fFrameManager.Initialise();
    }
}

void
ViewerWindow::DrawSplash()
{
  // Draw a splash background
  ImageManager& im = ImageManager::GetInstance();
  sf::Sprite sp = im.NewSprite( "Logo.png" );
  sf::Vector2<double> windowResolution = Coord::GetWindowResolution();
  sp.SetPosition( windowResolution.x / 2 - sp.GetSize().x / 2.0, windowResolution.y / 2 - sp.GetSize().y / 2.0 );

  fWindowApp->Clear( sf::Color( 255, 255, 255 ) );
  fWindowApp->Draw( sp );
  fWindowApp->Display();
}

void
ViewerWindow::Run()
{
  while( fWindowApp->IsOpened() )
    {
      EventLoop();
      RenderLoop();
    }
}

void
ViewerWindow::Destruct()
{
  stringstream configFileName;
  configFileName << getenv( "VIEWERROOT" ) << "/snogoggles.xml";
  Configuration saveConfig( configFileName.str(), true );
  saveConfig.SetI( "resX", static_cast<int>( Coord::GetWindowResolution().x ) );
  saveConfig.SetI( "resY", static_cast<int>( Coord::GetWindowResolution().y ) );
  fFrameManager.SaveConfiguration( saveConfig );
  saveConfig.SaveConfiguration();
  
  delete fWindowApp;
}

void
ViewerWindow::EventLoop()
{
  sf::Event event;
  while( fWindowApp->PollEvent( event ) )
    {
      switch( event.Type )
	{
// First ViewerWindow Specific Events
	case sf::Event::Closed:
	  fWindowApp->Close();
	  break;
	case sf::Event::Resized:
	  Coord::SetWindowSize( event.Size.Width, event.Size.Height );
	  break;
	case sf::Event::LostFocus:
	case sf::Event::GainedFocus:
	  break;

// Now events to use and then pass to frames
	case sf::Event::KeyPressed:
	  if( event.Key.Code == sf::Key::Escape )
	    {
	      fWindowApp->Close();
	      break;
	    }
//Drop through
	default:
	  Rect windowRect;
	  windowRect.SetFromResolutionRect( sf::Rect<double>( 0.0, 0.0, Coord::GetWindowResolution().x, Coord::GetWindowResolution().y ) );
	  UIEvent uiEvent( event, windowRect );
	  fFrameManager.NewEvent( uiEvent );
	}
    }
  // Now get Frames to deal with events
  fFrameManager.EventLoop();
}

void
ViewerWindow::RenderLoop()
{
  fWindowApp->SetActive();
  SetGlobalGLStates();

  // SetGlobalGLStates clears the background colour to white.
  // This SFML call is unnecessary.
  //fWindowApp->Clear( sf::Color( 255, 255, 255 ) );

  fFrameManager.Render3d();
  fWindowApp->SaveGLStates(); // This call seems to be necessary.
  fFrameManager.Render2d( *fWindowApp );
  fFrameManager.RenderGUI( *fWindowApp );
  fWindowApp->RestoreGLStates(); // Matches the save call above.

  fWindowApp->Display();
}

void 
ViewerWindow::SetGlobalGLStates()
{
  glClearColor(1.f, 1.f, 1.f, 1.f); // Sets the clear color to white.
  glClearDepth(1.f); // Sets the depth buffer clear to 1.
  glClearStencil(0); // Sets the stencil buffer clear to 0.

  // Clears the color, depth and stencil buffers.
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );

  // Enable Z-buffer read and write
  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);
}
