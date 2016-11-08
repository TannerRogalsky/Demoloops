#include "helpers.h"

void logSDLError(std::ostream &os, const std::string &msg){
  os << msg << " error: " << SDL_GetError() << std::endl;
}

SDL_Texture* loadTexture(const std::string &file, SDL_Renderer *ren){
  SDL_Texture *texture = IMG_LoadTexture(ren, file.c_str());
  if (texture == nullptr){
    logSDLError(std::cerr, "LoadTexture");
  }
  return texture;
}

void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, int x, int y, int w, int h){
  //Setup the destination rectangle to be at the position we want
  SDL_Rect dst;
  dst.x = x;
  dst.y = y;
  dst.w = w;
  dst.h = h;
  SDL_RenderCopy(ren, tex, NULL, &dst);
}

void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, int x, int y){
  int w, h;
  SDL_QueryTexture(tex, NULL, NULL, &w, &h);
  renderTexture(tex, ren, x, y, w, h);
}

TTF_Font* loadFont(const std::string &fontFile, int fontSize) {
  //Open the font
  TTF_Font *font = TTF_OpenFont(fontFile.c_str(), fontSize);
  if (font == nullptr){
    logSDLError(std::cerr, "TTF_OpenFont");
    return nullptr;
  }
  return font;
}

SDL_Texture* renderText(const std::string &message, TTF_Font *font, SDL_Color color, SDL_Renderer *renderer){
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

/* A simple function that will read a file into an allocated char pointer buffer */
char* filetobuf(char *file)
{
    FILE *fptr;
    long length;
    char *buf;

    fptr = fopen(file, "rb"); /* Open file for reading */
    if (!fptr) /* Return NULL on failure */
        return NULL;
    fseek(fptr, 0, SEEK_END); /* Seek to the end of the file */
    length = ftell(fptr); /* Find out how many bytes into the file we are */
    buf = (char*)malloc(length+1); /* Allocate a buffer for the entire length of the file and a null terminator */
    fseek(fptr, 0, SEEK_SET); /* Go back to the beginning of the file */
    fread(buf, length, 1, fptr); /* Read the contents of the file in to the buffer */
    fclose(fptr); /* Close the file */
    buf[length] = 0; /* Null terminator */

    return buf; /* Return the buffer */
}

void applyMatrix(demoloop::Vertex &v, const demoloop::Matrix4 &m) {
  const float *e = m.getElements();

  const float x = (v.x * e[0]) + (v.y * e[4]) + (v.z * e[8]) + e[12];
  const float y = (v.x * e[1]) + (v.y * e[5]) + (v.z * e[9]) + e[13];
  const float z = (v.x * e[2]) + (v.y * e[6]) + (v.z * e[10]) + e[14];

  v.x = x;
  v.y = y;
  v.z = z;
}

void applyMatrix(demoloop::Triangle &t, const demoloop::Matrix4 &m) {
  applyMatrix(t.a, m);
  applyMatrix(t.b, m);
  applyMatrix(t.c, m);
}

void applyColor(demoloop::Vertex &v, const RGB &c) {
  v.r = c.r;
  v.g = c.g;
  v.b = c.b;
}

void applyColor(demoloop::Triangle &t, const RGB &c) {
  applyColor(t.a, c);
  applyColor(t.b, c);
  applyColor(t.c, c);
}
