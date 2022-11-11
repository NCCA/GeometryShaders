#include <QMouseEvent>
#include <QGuiApplication>

#include "NGLScene.h"
#include <ngl/NGLStream.h>
#include <ngl/NGLInit.h>
#include <ngl/VAOFactory.h>
#include <iostream>
#include <ngl/Util.h>
#include <ngl/Random.h>
#include <ngl/ShaderLib.h>
#include <ngl/Quaternion.h>
#include <iostream>
const char *GridViz = "GridViz";
NGLScene::NGLScene()
{
  // re-size the widget to that of the parent (in this case the GLFrame passed in on construction)
  setTitle("VectorViz");
  m_numPoints = 1000;
}

NGLScene::~NGLScene()
{
  std::cout << "Shutting down NGL, removing VAO's and Shaders\n";
}

void NGLScene::resizeGL(int _w, int _h)
{
  m_project = ngl::perspective(45.0f, static_cast<float>(_w) / _h, 0.05f, 350.0f);
  m_win.width = static_cast<int>(_w * devicePixelRatio());
  m_win.height = static_cast<int>(_h * devicePixelRatio());
}

void NGLScene::createVectors(size_t _num)
{
  m_pos.resize(_num);
  m_dir.resize(_num);
  for (size_t i = 0; i < m_numPoints; ++i)
  {
    m_pos[i] = ngl::Random::getRandomVec3() * 10;
    m_dir[i] = ngl::Random::getRandomVec3() * 12;
  }

  m_vao = ngl::vaoFactoryCast<ngl::MultiBufferVAO>(ngl::VAOFactory::createVAO(ngl::multiBufferVAO, GL_POINTS));
  m_vao->bind();
  // need to set initial data slot for Vertex this will be index 0
  m_vao->setData(ngl::MultiBufferVAO::VertexData(m_numPoints * sizeof(ngl::Vec3), m_pos[0].m_x));
  m_vao->setVertexAttributePointer(0, 3, GL_FLOAT, 0, 0);
  // need to set initial data slot for colour this will be index 1
  m_vao->setData(ngl::MultiBufferVAO::VertexData(m_numPoints * sizeof(ngl::Vec3), m_dir[0].m_x));
  m_vao->setVertexAttributePointer(1, 3, GL_FLOAT, 0, 0);
  m_vao->setNumIndices(m_numPoints);
  m_vao->unbind();
}

void NGLScene::initializeGL()
{
  // we must call that first before any other GL commands to load and link the
  // gl commands from the lib, if that is not done program will crash
  ngl::NGLInit::initialize();
  glClearColor(0.7f, 0.7f, 0.7f, 1.0f); // Grey Background
  // enable depth testing for drawing
  glEnable(GL_DEPTH_TEST);
  // enable multisampling for smoother drawing
  glEnable(GL_MULTISAMPLE);
  createVectors(m_numPoints);
  ngl::ShaderLib::loadShader(GridViz, "shaders/VectorVizVertex.glsl", "shaders/VectorVizFragment.glsl", "shaders/VectorVizGeometry.glsl");
  glPointSize(10);
  ngl::Vec3 from(25, 25, 25);
  ngl::Vec3 to(0, 0, 0);
  ngl::Vec3 up(0, 1, 0);
  m_view = ngl::lookAt(from, to, up);
  ngl::ShaderLib::use(GridViz);
  ngl::ShaderLib::setUniform("thickness", 8.0f);
  ngl::ShaderLib::setUniform("thickness2", 2.0f);
  ngl::ShaderLib::setUniform("viewportSize", ngl::Vec2(m_win.width, m_win.height));

  startTimer(10);
}

void NGLScene::paintGL()
{
  // clear the screen and depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glViewport(0, 0, m_win.width, m_win.height);
  ngl::ShaderLib::use(GridViz);
  auto view = ngl::lookAt({20, 20, 20}, {0, 0, 0}, ngl::Vec3::up());
  auto project = ngl::perspective(45.0f, 1.0f, 0.1f, 300.0f);
  // Rotation based on the mouse position for our global transform
  auto rotX = ngl::Mat4::rotateX(m_win.spinXFace);
  auto rotY = ngl::Mat4::rotateY(m_win.spinYFace);
  // multiply the rotations
  ngl::Mat4 mouseGlobalTX = rotY * rotX;
  // add the translations
  mouseGlobalTX.m_m[3][0] = m_modelPos.m_x;
  mouseGlobalTX.m_m[3][1] = m_modelPos.m_y;
  mouseGlobalTX.m_m[3][2] = m_modelPos.m_z;
  ngl::ShaderLib::setUniform("MVP", m_project * m_view * mouseGlobalTX);
  ngl::ShaderLib::setUniform("viewportSize", ngl::Vec2(m_win.width, m_win.height));

  m_vao->bind();
  m_vao->draw();
  m_vao->unbind();
}

//----------------------------------------------------------------------------------------------------------------------

void NGLScene::keyPressEvent(QKeyEvent *_event)
{
  // this method is called every time the main window recives a key event.
  // we then switch on the key value and set the camera in the GLWindow
  switch (_event->key())
  {
  // escape key to quite
  case Qt::Key_Escape:
    QGuiApplication::exit(EXIT_SUCCESS);
    break;
  case Qt::Key_Space:
    m_win.spinXFace = 0;
    m_win.spinYFace = 0;
    m_modelPos.set(ngl::Vec3::zero());

    break;
  default:
    break;
  }
  // finally update the GLWindow and re-draw

  update();
}

void NGLScene::updateDirection()
{
  static float s = 0;
  ngl::Quaternion q(ngl::Vec3(s, s, s));
  ngl::Mat3 tx = q.toMat4();
  std::vector<ngl::Vec3> txDir(m_dir.size());
  std::transform(std::begin(m_dir), std::end(m_dir), std::begin(txDir),
                 [tx](auto v)
                 { return tx * v; });
  s += 1.0f;
  m_vao->bind();
  // need to set initial data slot for colour this will be index 1
  m_vao->setData(1, ngl::MultiBufferVAO::VertexData(m_numPoints * sizeof(ngl::Vec3), txDir[0].m_x));
  m_vao->setVertexAttributePointer(1, 3, GL_FLOAT, 0, 0);
  m_vao->unbind();
}

void NGLScene::timerEvent(QTimerEvent *_t)
{
  updateDirection();
  update();
}