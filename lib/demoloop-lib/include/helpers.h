#pragma once

#include <iostream>
#include <string>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <GL/glew.h>
#include "glm/glm.hpp"
#include "common/math.h"
#include "hsl.h"

GLuint loadTexture(const std::string &path);
void renderTexture(GLuint texture, float x, float y, float w, float h);
void renderTexture(GLuint texture, float x, float y, float z, float w, float h);

/*
 * Log an SDL error with some error message to the output stream of our choice
 * @param os The output stream to write the message too
 * @param msg The error message to write, format will be msg error: SDL_GetError()
 */
void logSDLError(std::ostream &os, const std::string &msg);

/*
 * Loads an image into a texture on the rendering device
 * @param file The image file to load
 * @param ren The renderer to load the texture onto
 * @return the loaded texture, or nullptr if something went wrong.
 */
SDL_Texture* loadTexture(const std::string &file, SDL_Renderer *ren);

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
void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, int x, int y, int w, int h);

/*
 * Draw an SDL_Texture to an SDL_Renderer at position x, y, preserving
 * the texture's width and height
 * @param tex The source texture we want to draw
 * @param rend The renderer we want to draw too
 * @param x The x coordinate to draw too
 * @param y The y coordinate to draw too
 */
void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, int x, int y);

/*
 * Render the message we want to display to a texture for drawing
 * @param fontFile The path to the font we want to load
 * @param fontSize The size we want the font to be
 * @return A TTF_Font from the font file, or nullptr if something went wrong
 */
TTF_Font* loadFont(const std::string &fontFile, int fontSize);

/*
 * Render the message we want to display to a texture for drawing
 * @param message The message we want to display
 * @param font The font we want to use to render the text
 * @param color The color we want the text to be
 * @param renderer The renderer to load the texture in
 * @return An SDL_Texture containing the rendered message, or nullptr if something went wrong
 */
SDL_Texture* renderText(const std::string &message, TTF_Font *font, SDL_Color color, SDL_Renderer *renderer);

char* filetobuf(char *file);

void applyMatrix(demoloop::Vertex &v, const glm::mat4 &m);
void applyMatrix(demoloop::Triangle &t, const glm::mat4 &m);
void applyColor(demoloop::Vertex &v, const RGB &c);
void applyColor(demoloop::Triangle &t, const RGB &c);
