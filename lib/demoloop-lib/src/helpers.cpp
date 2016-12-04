#include "helpers.h"
#include <SDL_image.h>
#include "graphics/gl.h"
#include "res_path.h"

void logSDLError(const char *msg){
  printf("%s error: %s\n", msg, SDL_GetError());
}

SDL_Texture* loadTexture(const std::string &file, SDL_Renderer *ren){
  SDL_Texture *texture = IMG_LoadTexture(ren, file.c_str());
  if (texture == nullptr){
    logSDLError("LoadTexture");
  }
  return texture;
}

GLuint loadTexture(const std::string &path) {
  GLuint texture;

  SDL_Surface *surf = IMG_Load((getResourcePath() + path).c_str());
  if (surf == nullptr) {
    logSDLError("IMG_Load");
  }
  glGenTextures(1,&texture);
  glBindTexture(GL_TEXTURE_2D,texture);
  auto fmt = surf->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB;
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surf->w,surf->h, 0, fmt, GL_UNSIGNED_BYTE,surf->pixels);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  SDL_FreeSurface(surf);

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

void renderTexture(GLuint texture, float x, float y, float w, float h) {
  renderTexture(texture, x, y, 1, w, h);
}

void renderTexture(GLuint texture, float x, float y, float z, float w, float h) {
  demoloop::Vertex vertices[6];
  vertices[0].x = x + 0;
  vertices[0].y = y + 0;
  vertices[0].z = z + 0;
  vertices[1].x = x + 0;
  vertices[1].y = y + h;
  vertices[1].z = z + 0;
  vertices[2].x = x + w;
  vertices[2].y = y + 0;
  vertices[2].z = z + 0;
  vertices[3].x = x + w;
  vertices[3].y = y + h;
  vertices[3].z = z + 0;
  vertices[4].x = x + 0;
  vertices[4].y = y + h;
  vertices[4].z = z + 0;
  vertices[5].x = x + w;
  vertices[5].y = y + 0;
  vertices[5].z = z + 0;

  vertices[0].s = 0;
  vertices[0].t = 0;
  vertices[1].s = 0;
  vertices[1].t = 1;
  vertices[2].s = 1;
  vertices[2].t = 0;
  vertices[3].s = 1;
  vertices[3].t = 1;
  vertices[4].s = 0;
  vertices[4].t = 1;
  vertices[5].s = 1;
  vertices[5].t = 0;

  demoloop::gl.bindTexture(texture);
  demoloop::gl.triangles(vertices, 6);
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
    logSDLError("TTF_OpenFont");
    return nullptr;
  }
  return font;
}

SDL_Texture* renderText(const std::string &message, TTF_Font *font, SDL_Color color, SDL_Renderer *renderer){
  //We need to first render to a surface as that's what TTF_RenderText returns, then
  //load that surface into a texture
  SDL_Surface *surf = TTF_RenderText_Blended(font, message.c_str(), color);
  if (surf == nullptr){
    logSDLError("TTF_RenderText");
    return nullptr;
  }
  SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surf);
  if (texture == nullptr){
    logSDLError("CreateTexture");
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

void applyMatrix(demoloop::Vertex &v, const glm::mat4 &m) {
  const float x = (v.x * m[0][0]) + (v.y * m[1][0]) + (v.z * m[2][0]) + m[3][0];
  const float y = (v.x * m[0][1]) + (v.y * m[1][1]) + (v.z * m[2][1]) + m[3][1];
  const float z = (v.x * m[0][2]) + (v.y * m[1][2]) + (v.z * m[2][2]) + m[3][2];

  v.x = x;
  v.y = y;
  v.z = z;
}

void applyMatrix(demoloop::Triangle &t, const glm::mat4 &m) {
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
