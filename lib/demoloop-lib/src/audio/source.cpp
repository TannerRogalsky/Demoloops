#include "audio/source.h"
#include "res_path.h"

namespace demoloop {

Source::Source(const std::string &path) {
  FILE* file = fopen((getResourcePath() + path).c_str(), "rb");
  ov_open_callbacks(file, &vf, NULL, 0, OV_CALLBACKS_DEFAULT);

  vi = ov_info(&vf, -1);
  mFormat = vi->channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;

  alGenBuffers(16, buffers);
  alGenSources (1, &source);

  for(int i = 0;i<16;++i) {
    unsigned long pos = 0;

    while(pos < sizeof(pcmout)) {
      long ret = ov_read(&vf, pcmout+pos, sizeof(pcmout)-pos, 0, 2, 1, &current_section);
      pos+=ret;
      if(ret == 0) {
        eof = 1;
        break;
      }
    }


    alBufferData(buffers[i], mFormat, pcmout, pos, vi->rate);
  }
  alSourceQueueBuffers(source, 16, buffers);
}

Source::~Source() {
  alDeleteSources(1, &source);
  alDeleteBuffers(16, buffers);
}

void Source::play() {
  alSourcePlay(source);
}

void Source::update() {
  if (!eof) {
    ALuint released[16];
    ALint count;
    alGetSourcei(source, AL_BUFFERS_PROCESSED, &count);
    alSourceUnqueueBuffers(source, count, released);

    for(int i = 0;i<count;++i) {
      unsigned long pos = 0;

      while(pos < sizeof(pcmout)) {
        long ret = ov_read(&vf, pcmout+pos, sizeof(pcmout)-pos, 0, 2, 1, &current_section);
        pos+=ret;
        if(ret == 0) {
          eof = 1;
          // alSourceStop(source);
          // current_section = 0;
          break;
      }
    }

    alBufferData(released[i], mFormat, pcmout, pos, vi->rate);

    }
    alSourceQueueBuffers(source, count, released);
  } else {
    ov_time_seek(&vf, 0);
    eof = 0;
  }
}

}
