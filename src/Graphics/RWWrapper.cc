#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#include <Viewer/RWWrapper.hh>
#include <Viewer/RectPtr.hh>
#include <Viewer/Rect.hh>
#include <Viewer/Sprite.hh>
#include <Viewer/PixelImage.hh>
#include <Viewer/Text.hh>
using namespace Viewer;

RWWrapper::RWWrapper( sf::RenderWindow& renderWindow )
  : fRenderWindow( renderWindow )
{

}

void 
RWWrapper::Draw( Sprite& object )
{
  sf::Sprite sfmlSprite( object.GetTexture() );
  sf::Rect<double> resPos = object.GetRect()->GetRect( Rect::eResolution );
  sfmlSprite.SetPosition( resPos.Left, resPos.Top );
  sfmlSprite.SetScale( resPos.Width / object.GetTexture().GetWidth(), resPos.Height / object.GetTexture().GetHeight() );
  DrawObject( sfmlSprite );
}

void 
RWWrapper::Draw( Text& object )
{
  sf::Text sfmlText( object.GetString() );
  sf::Rect<double> resPos = object.GetRect()->GetRect( Rect::eResolution );
  sf::Rect<float> textRect = sfmlText.GetGlobalBounds();
  sfmlText.SetPosition( resPos.Left, resPos.Top );
  const double fullHeight = resPos.Height;
  const double textAspectRatio = textRect.Width / textRect.Height;
  resPos.Height = resPos.Width / textAspectRatio;
  if( resPos.Height < fullHeight )
    resPos.Top += ( fullHeight - resPos.Height ) / 2.0;

  sfmlText.Scale( resPos.Width / textRect.Width, resPos.Height / textRect.Height );
  sfmlText.SetColor( object.GetColour() );
  DrawObject( sfmlText );
}

void
RWWrapper::Draw( PixelImage& object )
{
  sf::Sprite sfmlSprite( object.GetTexture() );
  sf::Rect<double> resPos = object.GetRect()->GetRect( Rect::eResolution );
  sfmlSprite.SetPosition( resPos.Left, resPos.Top );
  sfmlSprite.SetScale( resPos.Width / object.GetTexture().GetWidth(), resPos.Height / object.GetTexture().GetHeight() );
  DrawObject( sfmlSprite );
}

void 
RWWrapper::DrawObject( sf::Drawable& object )
{
  fRenderWindow.Draw( object );
}

sf::Time
RWWrapper::GetFrameTime()
{
  return fClock.Restart();
}
