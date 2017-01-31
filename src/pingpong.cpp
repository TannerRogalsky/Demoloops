#include "demoloop.h"
#include "graphics/3d_primitives.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <iostream>
#include <btBulletDynamicsCommon.h>
using namespace std;
using namespace demoloop;

namespace Utils
{
  glm::mat4 convertBulletTransformToGLM(btTransform& transform)
  {
    float data[16];
    transform.getOpenGLMatrix(data);
    return glm::make_mat4(data);
  }
  btTransform convertGLMTransformToBullet(glm::mat4 transform)
  {
    const float* data = glm::value_ptr(transform);
    btTransform bulletTransform;
    bulletTransform.setFromOpenGLMatrix(data);
    return bulletTransform;
  }
  btVector3 convertGLMVectorToBullet(glm::vec3 vector)
  {
    return btVector3(vector.x,vector.y,vector.z);
  }
  glm::vec3 convertBulletVectorToGLM(btVector3& vector)
  {
    return glm::vec3(vector.getX(),vector.getY(),vector.getZ());
  }
}

float t = 0;
const float farPlane = 1000;

bool intersectsWith(const glm::vec3 &bmin, const glm::vec3 &bmax, const glm::vec3 sphere) {
    float dmin = 0;

    if (sphere.x < bmin.x) {
        dmin += powf(sphere.x - bmin.x, 2);
    } else if (sphere.x > bmax.x) {
        dmin += powf(sphere.x - bmax.x, 2);
    }

    if (sphere.y < bmin.y) {
        dmin += powf(sphere.y - bmin.y, 2);
    } else if (sphere.y > bmax.y) {
        dmin += powf(sphere.y - bmax.y, 2);
    }

    if (sphere.z < bmin.z) {
        dmin += powf(sphere.z - bmin.z, 2);
    } else if (sphere.z > bmax.z) {
        dmin += powf(sphere.z - bmax.z, 2);
    }

    // cout << dmin << endl;

    return dmin == 0; // less than sphere radius
}

class PingPong : public Demoloop {
public:
  PingPong() : Demoloop(150, 150, 150), bbox(cube(0, 0, 0, 1)), ball(sphere(1)) {
    glm::mat4 perspective = glm::perspective(static_cast<float>(DEMOLOOP_M_PI) / 4.0f, (float)width / (float)height, 0.1f, farPlane);
    gl.getProjection() = perspective;

    bboxLines = bbox.getLines();

    // BULLET STUFF
    broadphase = new btDbvtBroadphase();

    collisionConfiguration = new btDefaultCollisionConfiguration();
    dispatcher = new btCollisionDispatcher(collisionConfiguration);

    solver = new btSequentialImpulseConstraintSolver;

    dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
    dynamicsWorld->setGravity(btVector3(0, -10, 0));

    groundShape = new btStaticPlaneShape(btVector3(0, 1, 0), 1);
    fallShape = new btSphereShape(1);

    groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, -1, 0)));
    btRigidBody::btRigidBodyConstructionInfo
            groundRigidBodyCI(0, groundMotionState, groundShape, btVector3(0, 0, 0));
    groundRigidBody = new btRigidBody(groundRigidBodyCI);
    groundRigidBody->setRestitution(0.5);
    dynamicsWorld->addRigidBody(groundRigidBody);


    fallMotionState =
            new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 50, 0)));
    btScalar mass = 1;
    btVector3 fallInertia(0, 0, 0);
    fallShape->calculateLocalInertia(mass, fallInertia);
    btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(mass, fallMotionState, fallShape, fallInertia);
    fallRigidBody = new btRigidBody(fallRigidBodyCI);
    fallRigidBody->setRestitution(1.0);
    dynamicsWorld->addRigidBody(fallRigidBody);
  }

  ~PingPong() {
    dynamicsWorld->removeRigidBody(fallRigidBody);
    delete fallRigidBody->getMotionState();
    delete fallRigidBody;

    dynamicsWorld->removeRigidBody(groundRigidBody);
    delete groundRigidBody->getMotionState();
    delete groundRigidBody;


    delete fallShape;

    delete groundShape;


    delete dynamicsWorld;
    delete solver;
    delete collisionConfiguration;
    delete dispatcher;
    delete broadphase;
  }

  void Update(float dt) {
    t += dt;

    dynamicsWorld->stepSimulation(dt, 10);
    btTransform trans;
    fallRigidBody->getMotionState()->getWorldTransform(trans);
    std::cout << "sphere height: " << trans.getOrigin().getY() << std::endl;


    const glm::vec3 eye = glm::rotate(glm::vec3(0, 0, 100), t * 0, {0, 1, 0});
    const glm::vec3 target = {0, 0, 0};
    const glm::vec3 up = {0, 1, 0};
    glm::mat4 camera = glm::lookAt(eye, target, up);

    GL::TempTransform t1(gl);
    t1.get() = camera;


    glm::mat4 bboxMatrix;
    bboxMatrix = glm::translate(bboxMatrix, {0, 0, 0});
    bboxMatrix = glm::scale(bboxMatrix, {width / 100, height / 100, 6});
    setColor(0, 0, 0);
    gl.lines(bboxLines.data(), bboxLines.size(), bboxMatrix);
    // bbox.draw(bboxMatrix);

    setColor(255, 0, 0);
    ball.draw(Utils::convertBulletTransformToGLM(trans));
  }

private:
  Mesh bbox;
  Mesh ball;
  std::vector<Vertex> bboxLines;

  btBroadphaseInterface* broadphase;
  btDefaultCollisionConfiguration* collisionConfiguration;
  btCollisionDispatcher* dispatcher;
  btSequentialImpulseConstraintSolver* solver;
  btDiscreteDynamicsWorld* dynamicsWorld;
  btCollisionShape* groundShape;
  btCollisionShape* fallShape;

  btDefaultMotionState* groundMotionState;
  btRigidBody* groundRigidBody;
  btDefaultMotionState* fallMotionState;
  btRigidBody* fallRigidBody;
};

int main(int, char**){
  PingPong test;
  test.Run();

  return 0;
}
