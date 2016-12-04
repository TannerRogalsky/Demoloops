#pragma once

#include <string>
#include <SDL.h>

std::string getResourcePath(const std::string &subDir = ""){
#ifdef _WIN32
  const char PATH_SEP = '\\';
#else
  const char PATH_SEP = '/';
#endif
  static std::string baseRes;
  if (baseRes.empty()){
    char *basePath = SDL_GetBasePath();
    if (basePath){
      baseRes = basePath;
      SDL_free(basePath);
    }
    else {
      logSDLError("SDL_GetBasePath");
      return "";
    }
    //We replace the last bin/ with res/ to get the the resource path
    size_t pos = baseRes.rfind("build");
    baseRes = baseRes.substr(0, pos) + "res" + PATH_SEP;
  }
  return subDir.empty() ? baseRes : baseRes + subDir + PATH_SEP;
}
