#include "demoloop.h"
#include "res_path.h"
#ifdef EMSCRIPTEN
  #include <AL/al.h>
  #include <AL/alc.h>
#else
  #include <al.h>
  #include <alc.h>
#endif
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

using namespace std;
using namespace demoloop;

class Test : public Demoloop {
public:
  Test() : Demoloop(150, 150, 150) {
      int major, minor;
      alcGetIntegerv(NULL, ALC_MAJOR_VERSION, 1, &major);
      alcGetIntegerv(NULL, ALC_MAJOR_VERSION, 1, &minor);

      printf("ALC version: %i.%i\n", major, minor);
      printf("Default device: %s\n", alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER));

      ALCdevice* device = alcOpenDevice(NULL);
      ALCcontext* context = alcCreateContext(device, NULL);
      alcMakeContextCurrent(context);

      printf("OpenAL version: %s\n", alGetString(AL_VERSION));
      printf("OpenAL vendor: %s\n", alGetString(AL_VENDOR));
      printf("OpenAL renderer: %s\n", alGetString(AL_RENDERER));

      ALfloat listenerPos[] = {0.0, 0.0, 0.0};
      ALfloat listenerVel[] = {0.0, 0.0, 0.0};
      ALfloat listenerOri[] = {0.0, 0.0, -1.0, 0.0, 1.0, 0.0};

      alListenerfv(AL_POSITION, listenerPos);
      alListenerfv(AL_VELOCITY, listenerVel);
      alListenerfv(AL_ORIENTATION, listenerOri);

      ALuint buffers[1];
      alGenBuffers(1, buffers);

      FILE* source = fopen((getResourcePath() + "test_openal/hopeneverstops.wav").c_str(), "rb");
      fseek(source, 0, SEEK_END);
      int size = ftell(source);
      fseek(source, 0, SEEK_SET);

      unsigned char* buffer = (unsigned char*) malloc(size);
      fread(buffer, size, 1, source);
      fclose(source);

      unsigned offset = 12; // ignore the RIFF header
      offset += 8; // ignore the fmt header
      offset += 2; // ignore the format type

      unsigned channels = buffer[offset + 1] << 8;
      channels |= buffer[offset];
      offset += 2;
      printf("Channels: %u\n", channels);

      unsigned frequency = buffer[offset + 3] << 24;
      frequency |= buffer[offset + 2] << 16;
      frequency |= buffer[offset + 1] << 8;
      frequency |= buffer[offset];
      offset += 4;
      printf("Frequency: %u\n", frequency);

      offset += 6; // ignore block size and bps

      unsigned bits = buffer[offset + 1] << 8;
      bits |= buffer[offset];
      offset += 2;
      printf("Bits: %u\n", bits);

      ALenum format = 0;
      if(bits == 8)
      {
        if(channels == 1)
          format = AL_FORMAT_MONO8;
        else if(channels == 2)
          format = AL_FORMAT_STEREO8;
      }
      else if(bits == 16)
      {
        if(channels == 1)
          format = AL_FORMAT_MONO16;
        else if(channels == 2)
          format = AL_FORMAT_STEREO16;
      }

      offset += 8; // ignore the data chunk

      printf("Start offset: %d\n", offset);

      alBufferData(buffers[0], format, &buffer[offset], size - offset, frequency);

      ALuint sources[1];
      alGenSources(1, sources);
      alSourcei(sources[0], AL_BUFFER, buffers[0]);
      alSourcei(sources[0], AL_LOOPING, AL_TRUE);
      alSourcePlay(sources[0]);
  }

  ~Test() {
  }

  void Update(float dt) {
  }

private:
};

int main(int, char**){
  Test test;
  test.Run();

  return 0;
}
