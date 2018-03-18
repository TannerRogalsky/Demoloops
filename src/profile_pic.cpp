#include "demoloop.h"
#include "helpers.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
using namespace std;
using namespace demoloop;

float t = 0;
const float CYCLE_LENGTH = 10;
const uint32_t arms = 5;
const uint32_t trisPerArm = 20;
const uint32_t numTris = arms * trisPerArm;
// const uint32_t numTris = 20;

const float RADIUS = 0.05;
const float interval = DEMOLOOP_M_PI * 2 / 3;
const Triangle triangle = {
  {RADIUS * cosf(interval * 0), RADIUS * sinf(interval * 0), 1},
  {RADIUS * cosf(interval * 1), RADIUS * sinf(interval * 1), 1},
  {RADIUS * cosf(interval * 2), RADIUS * sinf(interval * 2), 1}
};

class ProfilePic : public Demoloop {
public:
  ProfilePic() : Demoloop(CYCLE_LENGTH, 960, 1280, 255, 255, 255) {
    fg_texture = loadTexture("profile_pic/tanner_fg.png");
  }

  ~ProfilePic() {
    glDeleteTextures(1,&fg_texture);
  }

  void Update() {
    const float cycle_ratio = getCycleRatio();

    const glm::vec3 eye = glm::vec3(0, 0, 5);
    const glm::vec3 up = glm::vec3(0, 1, 0);
    const glm::vec3 target = glm::vec3(0, 0, 0);
    glm::mat4 camera = glm::lookAt(eye, target, up);

    {
      GL::TempTransform t1(gl);
      t1.get() = camera;

      for (uint32_t i = 0; i < numTris; ++i) {
        const float t = i;
        const float i_cycle_ratio = fmod(t / numTris + cycle_ratio, 1);

        glm::vec3 p(0, 0, 1);
        p = glm::rotate(p, i_cycle_ratio * (float)DEMOLOOP_M_PI * 2, glm::vec3(0, 0, 1));
        p = glm::rotate(p, i_cycle_ratio * (float)DEMOLOOP_M_PI * 2, glm::vec3(0, 1, 0));
        p = glm::rotate(p, sinf(i_cycle_ratio * (float)DEMOLOOP_M_PI * 2), glm::vec3(1, 0, 0));
        p = glm::rotate(p, cycle_ratio * (float)DEMOLOOP_M_PI * 2, glm::vec3(0, 1, 0));
        glm::mat4 m = glm::lookAt(p, {0, 0, 0}, up);
        m = glm::translate(m, {0, 1, 0});
        m = glm::scale(m, {1.5, 1.5, 1.5});

        triangles[i] = triangle;
        applyMatrix(triangles[i], m);
        applyColor(triangles[i], hsl2rgb(t / numTris, 1, 0.5));
      }

      GL::TempProjection p1(gl);
      p1.get() = glm::perspective(static_cast<float>(DEMOLOOP_M_PI) / 4.0f, (float)width / (float)height, 0.1f, 100.0f);

      gl.bindTexture(gl.getDefaultTexture());
      gl.triangles(triangles, numTris);
    }

    {
      GL::TempTransform t1(gl);
      t1.get() = glm::translate(t1.get(), {0, 0, 1.7});
      renderTexture(fg_texture, 0, 0, 0, width, height);
    }
  }

private:
  Triangle triangles[numTris];
  GLuint fg_texture;
};

int main(int, char**){
  ProfilePic test;
  test.Run();

  return 0;
}
