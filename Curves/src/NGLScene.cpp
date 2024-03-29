#include <QMouseEvent>
#include <QApplication>

#include "NGLScene.h"
#include <ngl/Transformation.h>
#include <ngl/NGLInit.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/Random.h>
#include <ngl/ShaderLib.h>
#include <ngl/SimpleIndexVAO.h>
#include <ngl/Util.h>
#include <iostream>

NGLScene::NGLScene()
{
  setTitle("Geometry Shader Curves Use 1-2 to switch mode, 3,4,5 for steps");
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

void NGLScene::initializeGL()
{
  // we must call this first before any other GL commands to load and link the
  // gl commands from the lib, if this is not done program will crash
  ngl::NGLInit::initialize();

  glClearColor(0.4f, 0.4f, 0.4f, 1.0f); // Grey Background
  // enable depth testing for drawing
  glEnable(GL_DEPTH_TEST);
  // enable multisampling for smoother drawing
  glEnable(GL_MULTISAMPLE);
  ngl::Vec3 from(25, 25, 25);
  ngl::Vec3 to(0, 0, 0);
  ngl::Vec3 up(0, 1, 0);
  m_view = ngl::lookAt(from, to, up);

  // grab an instance of shader manager
  ngl::ShaderLib::loadShader("CurveShader", "shaders/curveVertex.glsl",
                             "shaders/curveFragment.glsl",
                             "shaders/curveGeometry.glsl");
  ngl::ShaderLib::use("CurveShader");
  ngl::ShaderLib::setUniform("steps", 0.01f);
  GLuint id = ngl::ShaderLib::getProgramID("CurveShader");
  m_subroutines[0] = glGetSubroutineIndex(id, GL_GEOMETRY_SHADER, "bezier");
  m_subroutines[1] = glGetSubroutineIndex(id, GL_GEOMETRY_SHADER, "lerpCurve");

  m_vao = ngl::VAOFactory::createVAO(ngl::simpleIndexVAO, GL_LINE_STRIP_ADJACENCY);
  createVAO();
  glEnable(GL_DEPTH_TEST); // for removal of hidden surfaces
  startTimer(500);
}

void NGLScene::createVAO()
{
  ngl::Random::setSeed(time(nullptr));
  std::vector<ngl::Vec3> controlPoints = {
      ngl::Vec3(0.0f, 4.0f, -2.0f),
      ngl::Vec3(0.0f, 3.0f, 1.0f),
      ngl::Vec3(0.0f, -3.0f, 1.0f),
      ngl::Vec3(0.0f, -4.0f, -2.0f),

      ngl::Vec3(-1.0f, 4.0f, -2.0f),
      ngl::Vec3(-1.0f, 3.0f, 1.0f),
      ngl::Vec3(-1.0f, -3.0f, 1.0f),
      ngl::Vec3(-1.0f, -4.0f, -2.0f),

      ngl::Vec3(1.0f, 4.0f, -4.0f),
      ngl::Vec3(2.0f, 2.0f, 1.0f),
      ngl::Vec3(1.0f, -3.0f, -1.0f),
      ngl::Vec3(1.0f, -2.0f, -5.0f),
      ngl::Random::getRandomPoint(4, 4, 4),
      ngl::Random::getRandomPoint(4, 4, 4),
      ngl::Random::getRandomPoint(4, 4, 4),
      ngl::Random::getRandomPoint(4, 4, 4)};
  m_numIndices = 16;
  constexpr GLshort restart = 9999;
  std::vector<GLshort> index = {0, 1, 2, 3, restart,
                                4, 5, 6, 7, restart,
                                8, 9, 10, 11, restart,
                                12, 13, 14, 15};
  m_vao->bind();

  // in this case we are going to set our data as the vertices above

  m_vao->setData(ngl::SimpleIndexVAO::VertexData(
      controlPoints.size() * sizeof(ngl::Vec3),
      controlPoints[0].m_x,
      index.size(), &index[0],
      GL_UNSIGNED_SHORT));
  // data is 24 bytes apart ( two Vec3's) first index
  // is 0 second is 3 floats into the data set (i.e. vec3 offset)
  m_vao->setVertexAttributePointer(0, 3, GL_FLOAT, 0, 0);
  m_vao->setNumIndices(index.size());
  glEnable(GL_PRIMITIVE_RESTART);
  glPrimitiveRestartIndex(restart);

  // now unbind
  m_vao->unbind();
}

void NGLScene::createRandomVAO()
{
  m_numIndices = 10000 * 4;
  std::vector<ngl::Vec3> controlPoints(m_numIndices);
  constexpr GLuint restart = 999999;
  std::vector<GLuint> index;
  GLint idx = -1;
  for (size_t i = 0; i < controlPoints.size(); i += 4)
  {
    for (size_t p = 0; p < 4; ++p)
    {
      controlPoints[(i + p)] = ngl::Random::getRandomPoint(20, 20, 20);
      index.push_back(++idx);
    }
    index.push_back(restart);
  }

  m_vao->bind();

  // in this case we are going to set our data as the vertices above

  m_vao->setData(ngl::SimpleIndexVAO::VertexData(
      controlPoints.size() * sizeof(ngl::Vec3),
      controlPoints[0].m_x,
      index.size(), &index[0],
      GL_UNSIGNED_INT));
  // data is 24 bytes apart ( two Vec3's) first index
  // is 0 second is 3 floats into the data set (i.e. vec3 offset)
  m_vao->setVertexAttributePointer(0, 3, GL_FLOAT, 0, 0);
  m_vao->setNumIndices(index.size());
  glEnable(GL_PRIMITIVE_RESTART);
  glPrimitiveRestartIndex(restart);

  // now unbind
  m_vao->unbind();
}

void NGLScene::loadMatricesToShader()
{
  ngl::ShaderLib::use("CurveShader");
  ngl::ShaderLib::setUniform("MVP", m_project * m_view * m_mouseGlobalTX);
  glUniformSubroutinesuiv(GL_GEOMETRY_SHADER, 1, &m_subroutines[m_activeSubroutine]);
  ngl::ShaderLib::setUniform("steps", m_steps);
}
void NGLScene::loadMatricesToColourShader(const ngl::Vec4 &_colour)
{
  ngl::ShaderLib::use(ngl::nglColourShader);
  ngl::ShaderLib::setUniform("Colour", _colour);
  ngl::ShaderLib::setUniform("MVP", m_project * m_view * m_mouseGlobalTX);
}

void NGLScene::paintGL()
{
  // clear the screen and depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glViewport(0, 0, m_win.width, m_win.height);
  // Rotation based on the mouse position for our global transform
  auto rotX = ngl::Mat4::rotateX(m_win.spinXFace);
  auto rotY = ngl::Mat4::rotateY(m_win.spinYFace);
  // multiply the rotations
  m_mouseGlobalTX = rotY * rotX;
  // add the translations
  m_mouseGlobalTX.m_m[3][0] = m_modelPos.m_x;
  m_mouseGlobalTX.m_m[3][1] = m_modelPos.m_y;
  m_mouseGlobalTX.m_m[3][2] = m_modelPos.m_z;

  loadMatricesToShader();
  m_vao->bind();
  m_vao->setMode(GL_LINE_STRIP_ADJACENCY);
  m_vao->draw();
  if (m_drawCP)
  {
    glPointSize(14.0f);
    loadMatricesToColourShader(ngl::Vec4(1.0f, 1.0f, 0.0f, 1.0f));
    m_vao->setMode(GL_POINTS);
    m_vao->draw();
  }
  if (m_drawHull)
  {
    loadMatricesToColourShader(ngl::Vec4(1.0f, 0.0f, 0.0f, 1.0f));
    m_vao->setMode(GL_LINE_STRIP);
    m_vao->draw();
  }

  m_vao->unbind();
}

//----------------------------------------------------------------------------------------------------------------------

void NGLScene::keyPressEvent(QKeyEvent *_event)
{
  switch (_event->key())
  {
  case Qt::Key_Escape:
    QApplication::quit();
    break;
  case Qt::Key_1:
    m_activeSubroutine = 0;
    break;
  case Qt::Key_2:
    m_activeSubroutine = 1;
    break;
  case Qt::Key_3:
    m_steps = 0.5f;
    break;
  case Qt::Key_4:
    m_steps = 0.1f;
    break;
  case Qt::Key_5:
    m_steps = 0.01f;
    break;
  case Qt::Key_R:
    createRandomVAO();
    break;
  case Qt::Key_S:
    createVAO();
    break;
  case Qt::Key_C:
    m_drawCP ^= true;
    break;
  case Qt::Key_H:
    m_drawHull ^= true;
    break;
  }
  update();
}

void NGLScene::timerEvent(QTimerEvent *)
{

  float *buffer = m_vao->mapBuffer();
  ngl::NGLCheckGLError("map", __LINE__);

  if (buffer != nullptr)
  {
    for (size_t i = 0; i < m_numIndices; i++)
    {
      buffer[i] = ngl::Random::randomNumber(40);
    }
  }
  m_vao->unmapBuffer();
  update();
}
