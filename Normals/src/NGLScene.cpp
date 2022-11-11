#include <QMouseEvent>
#include <QApplication>

#include "NGLScene.h"
#include <ngl/Transformation.h>
#include <ngl/NGLInit.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/ShaderLib.h>
#include <iostream>
NGLScene::NGLScene()
{
  setTitle("Simple Geometry Shader");
  m_normalSize = 0.1f;
  m_modelName = "cylinder";
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

constexpr auto Phong = "Phong";
constexpr auto normalShader = "normalShader";

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
  ngl::Vec3 from(0, 0, 3);
  ngl::Vec3 to(0, 0, 0);
  ngl::Vec3 up(0, 1, 0);

  m_view = ngl::lookAt(from, to, up);
  // set the shape using FOV 45 Aspect Ratio based on Width and Height
  // The final two are near and far clipping planes of 0.5 and 10
  m_project = ngl::perspective(45, (float)720.0 / 576.0, 0.5, 150);

  constexpr auto PhongVertex = "PhongVertex";
  constexpr auto PhongFragment = "PhongFragment";

  ngl::ShaderLib::createShaderProgram(Phong);

  ngl::ShaderLib::attachShader(PhongVertex, ngl::ShaderType::VERTEX);
  ngl::ShaderLib::attachShader(PhongFragment, ngl::ShaderType::FRAGMENT);
  ngl::ShaderLib::loadShaderSource(PhongVertex, "shaders/PhongVertex.glsl");
  ngl::ShaderLib::loadShaderSource(PhongFragment, "shaders/PhongFragment.glsl");

  ngl::ShaderLib::compileShader(PhongVertex);
  ngl::ShaderLib::compileShader(PhongFragment);
  ngl::ShaderLib::attachShaderToProgram(Phong, PhongVertex);
  ngl::ShaderLib::attachShaderToProgram(Phong, PhongFragment);

  ngl::ShaderLib::linkProgramObject(Phong);
  ngl::ShaderLib::use(Phong);

  // now pass the modelView and projection values to the shader
  ngl::ShaderLib::setUniform("Normalize", 1);
  ngl::ShaderLib::setUniform("viewerPos", from);
  /// now setup a basic 3 point lighting system
  m_key.position = ngl::Vec4(13, 2, 2);
  ngl::ShaderLib::setUniform("light[0].position", m_key.position);
  ngl::ShaderLib::setUniform("light[0].ambient", m_key.ambient);
  ngl::ShaderLib::setUniform("light[0].diffuse", m_key.diffuse);
  ngl::ShaderLib::setUniform("light[0].specular", m_key.specular);

  m_fill.position = ngl::Vec4(-13.0f, 1.5f, 2.0f);
  ngl::ShaderLib::setUniform("light[1].position", m_fill.position);
  ngl::ShaderLib::setUniform("light[1].ambient", m_fill.ambient);
  ngl::ShaderLib::setUniform("light[1].diffuse", m_fill.diffuse);
  ngl::ShaderLib::setUniform("light[1].specular", m_fill.specular);

  m_back.position = ngl::Vec4(0.0f, 1.0f, -12.0f);
  ngl::ShaderLib::setUniform("light[2].position", m_back.position);
  ngl::ShaderLib::setUniform("light[2].ambient", m_back.ambient);
  ngl::ShaderLib::setUniform("light[2].diffuse", m_back.diffuse);
  ngl::ShaderLib::setUniform("light[2].specular", m_back.specular);

  ngl::ShaderLib::setUniform("material.ambient", 0.329412f, 0.223529f, 0.027451f, 0.0f);
  ngl::ShaderLib::setUniform("material.diffuse", 0.780392f, 0.568627f, 0.113725f, 0.0f);
  ngl::ShaderLib::setUniform("material.specular", 0.992157f, 0.941176f, 0.807843f, 0.0f);
  ngl::ShaderLib::setUniform("material.shininess", 57.8974f);

  constexpr auto normalVertex = "normalVertex";
  constexpr auto normalFragment = "normalFragment";
  constexpr auto normalGeo = "normalGeo";

  ngl::ShaderLib::createShaderProgram(normalShader);

  ngl::ShaderLib::attachShader(normalVertex, ngl::ShaderType::VERTEX);
  ngl::ShaderLib::attachShader(normalFragment, ngl::ShaderType::FRAGMENT);
  ngl::ShaderLib::loadShaderSource(normalVertex, "shaders/normalVertex.glsl");
  ngl::ShaderLib::loadShaderSource(normalFragment, "shaders/normalFragment.glsl");

  ngl::ShaderLib::compileShader(normalVertex);
  ngl::ShaderLib::compileShader(normalFragment);
  ngl::ShaderLib::attachShaderToProgram(normalShader, normalVertex);
  ngl::ShaderLib::attachShaderToProgram(normalShader, normalFragment);

  ngl::ShaderLib::attachShader(normalGeo, ngl::ShaderType::GEOMETRY);
  ngl::ShaderLib::loadShaderSource(normalGeo, "shaders/normalGeo.glsl");
  ngl::ShaderLib::compileShader(normalGeo);
  ngl::ShaderLib::attachShaderToProgram(normalShader, normalGeo);
  ngl::ShaderLib::linkProgramObject(normalShader);
  ngl::ShaderLib::use(normalShader);
  // now pass the modelView and projection values to the shader
  ngl::ShaderLib::setUniform("normalSize", 0.1f);
  ngl::ShaderLib::setUniform("vertNormalColour", 1.0f, 1.0f, 0.0f, 1.0f);
  ngl::ShaderLib::setUniform("faceNormalColour", 1.0f, 0.0f, 0.0f, 1.0f);

  ngl::ShaderLib::setUniform("drawFaceNormals", true);
  ngl::ShaderLib::setUniform("drawVertexNormals", true);

  glEnable(GL_DEPTH_TEST); // for removal of hidden surfaces
  ngl::VAOPrimitives::createSphere("sphere", 0.8f, 40);
  ngl::VAOPrimitives::createCylinder("cylinder", 0.2f, 2.0f, 40, 40);
  ngl::VAOPrimitives::createCone("cone", 0.8f, 2.0, 40, 40);
  ngl::VAOPrimitives::createTorus("torus", 0.2f, 1.0f, 20, 20);
}

void NGLScene::loadMatricesToShader()
{
  ngl::ShaderLib::use(Phong);
  ngl::Mat4 MV;
  ngl::Mat4 MVP;
  ngl::Mat3 normalMatrix;
  ngl::Mat4 M;
  M = m_mouseGlobalTX * m_transformStack.getMatrix();
  MV = m_view * M;
  MVP = m_project * MV;
  normalMatrix = MV;
  normalMatrix.inverse().transpose();
  ngl::ShaderLib::setUniform("MV", MV);
  ngl::ShaderLib::setUniform("MVP", MVP);
  ngl::ShaderLib::setUniform("normalMatrix", normalMatrix);
  ngl::ShaderLib::setUniform("M", M);
}
void NGLScene::loadMatricesToNormalShader()
{
  ngl::ShaderLib::use(normalShader);
  ngl::Mat4 MV;
  ngl::Mat4 MVP;

  MVP = m_project * m_view *
        m_mouseGlobalTX *
        m_transformStack.getMatrix();

  ngl::ShaderLib::setUniform("MVP", MVP);
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

  ngl::ShaderLib::use(normalShader);
  ngl::ShaderLib::setUniform("normalSize", m_normalSize);
  loadMatricesToNormalShader();
  ngl::VAOPrimitives::draw(m_modelName);
  glPointSize(4.0f);
  glLineWidth(4.0f);

  ngl::ShaderLib::use(Phong);

  loadMatricesToShader();
  ngl::VAOPrimitives::draw(m_modelName);
}

//----------------------------------------------------------------------------------------------------------------------

void NGLScene::keyPressEvent(QKeyEvent *_event)
{
  static bool fn = true;
  static bool vn = true;

  switch (_event->key())
  {
  case Qt::Key_Plus:
    m_normalSize += 0.01f;
    break;
  case Qt::Key_Minus:
    m_normalSize -= 0.01f;
    break;
  case Qt::Key_1:
    fn ^= true;
    ngl::ShaderLib::use("normalShader");
    ngl::ShaderLib::setUniform("drawFaceNormals", fn);
    break;
  case Qt::Key_2:
    vn ^= true;
    ngl::ShaderLib::use("normalShader");
    ngl::ShaderLib::setUniform("drawVertexNormals", vn);
    break;
  case Qt::Key_S:
    m_modelName = "sphere";
    break;
  case Qt::Key_T:
    m_modelName = "teapot";
    break;
  case Qt::Key_C:
    m_modelName = "cube";
    break;
  case Qt::Key_Y:
    m_modelName = "cylinder";
    break;
  case Qt::Key_N:
    m_modelName = "cone";
    break;
  case Qt::Key_R:
    m_modelName = "torus";
    break;
  case Qt::Key_Escape:
    QApplication::quit();
    break;
  }
  update();
}
