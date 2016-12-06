#pragma once

#include <string>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

#ifdef EMSCRIPTEN
  #include <AL/al.h>
  #include <AL/alc.h>
#else
  #include <al.h>
  #include <alc.h>
#endif

namespace demoloop
{

class Source
{
public:

  Source(const std::string &path);
  ~Source();

  void play();
  void update();

private:
  OggVorbis_File vf;
  vorbis_info *vi;
  ALuint source;
  ALuint buffers[16];
  char pcmout[16*1024];
  int eof=0;
  int current_section;
  ALenum mFormat = 0;
};

}
