#pragma once

#include <iostream>
#include <string>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

/*
 * Log an SDL error with some error message to the output stream of our choice
 * @param os The output stream to write the message too
 * @param msg The error message to write, format will be msg error: SDL_GetError()
 */
void logSDLError(std::ostream &os, const std::string &msg){
  os << msg << " error: " << SDL_GetError() << std::endl;
}
/*
 * Loads an image into a texture on the rendering device
 * @param file The image file to load
 * @param ren The renderer to load the texture onto
 * @return the loaded texture, or nullptr if something went wrong.
 */
SDL_Texture* loadTexture(const std::string &file, SDL_Renderer *ren){
  SDL_Texture *texture = IMG_LoadTexture(ren, file.c_str());
  if (texture == nullptr){
    logSDLError(std::cerr, "LoadTexture");
  }
  return texture;
}
/*
 * Draw an SDL_Texture to an SDL_Renderer at position x, y, with some desired
 * width and height
 * @param tex The source texture we want to draw
 * @param rend The renderer we want to draw too
 * @param x The x coordinate to draw too
 * @param y The y coordinate to draw too
 * @param w The width of the texture to draw
 * @param h The height of the texture to draw
 */
void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, int x, int y, int w, int h){
  //Setup the destination rectangle to be at the position we want
  SDL_Rect dst;
  dst.x = x;
  dst.y = y;
  dst.w = w;
  dst.h = h;
  SDL_RenderCopy(ren, tex, NULL, &dst);
}

/*
 * Draw an SDL_Texture to an SDL_Renderer at position x, y, preserving
 * the texture's width and height
 * @param tex The source texture we want to draw
 * @param rend The renderer we want to draw too
 * @param x The x coordinate to draw too
 * @param y The y coordinate to draw too
 */
void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, int x, int y){
  int w, h;
  SDL_QueryTexture(tex, NULL, NULL, &w, &h);
  renderTexture(tex, ren, x, y, w, h);
}

/*
 * Render the message we want to display to a texture for drawing
 * @param fontFile The path to the font we want to load
 * @param fontSize The size we want the font to be
 * @return A TTF_Font from the font file, or nullptr if something went wrong
 */
TTF_Font* loadFont(const std::string &fontFile, int fontSize) {
  //Open the font
  TTF_Font *font = TTF_OpenFont(fontFile.c_str(), fontSize);
  if (font == nullptr){
    logSDLError(std::cerr, "TTF_OpenFont");
    return nullptr;
  }
  return font;
}

/*
 * Render the message we want to display to a texture for drawing
 * @param message The message we want to display
 * @param font The font we want to use to render the text
 * @param color The color we want the text to be
 * @param renderer The renderer to load the texture in
 * @return An SDL_Texture containing the rendered message, or nullptr if something went wrong
 */
SDL_Texture* renderText(const std::string &message, TTF_Font *font, SDL_Color color, SDL_Renderer *renderer)
{
  //We need to first render to a surface as that's what TTF_RenderText returns, then
  //load that surface into a texture
  SDL_Surface *surf = TTF_RenderText_Blended(font, message.c_str(), color);
  if (surf == nullptr){
    logSDLError(std::cerr, "TTF_RenderText");
    return nullptr;
  }
  SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surf);
  if (texture == nullptr){
    logSDLError(std::cerr, "CreateTexture");
  }
  //Clean up the surface and font
  SDL_FreeSurface(surf);
  return texture;
}
