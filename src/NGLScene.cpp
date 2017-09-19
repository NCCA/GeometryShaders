#include <QMouseEvent>
#include <QApplication>

#include "NGLScene.h"
#include <ngl/Camera.h>
#include <ngl/Light.h>
#include <ngl/Transformation.h>
#include <ngl/Material.h>
#include <ngl/NGLInit.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/ShaderLib.h>


NGLScene::NGLScene()
{
  setTitle("Simple Geometry Shader");
  m_normalSize=0.1f;
  m_modelName="cylinder";

}


NGLScene::~NGLScene()
{
  std::cout<<"Shutting down NGL, removing VAO's and Shaders\n";
}

void NGLScene::resizeGL( int _w, int _h )
{
  m_cam.setShape( 45.0f, static_cast<float>( _w ) / _h, 0.05f, 350.0f );
  m_win.width  = static_cast<int>( _w * devicePixelRatio() );
  m_win.height = static_cast<int>( _h * devicePixelRatio() );
}

constexpr auto Phong="Phong";
constexpr auto normalShader="normalShader";


void NGLScene::initializeGL()
{
  // we must call this first before any other GL commands to load and link the
  // gl commands from the lib, if this is not done program will crash
  ngl::NGLInit::instance();

  glClearColor(0.4f, 0.4f, 0.4f, 1.0f);			   // Grey Background
  // enable depth testing for drawing
  glEnable(GL_DEPTH_TEST);
  // enable multisampling for smoother drawing
  glEnable(GL_MULTISAMPLE);
  ngl::Vec3 from(0,0,3);
  ngl::Vec3 to(0,0,0);
  ngl::Vec3 up(0,1,0);

  m_cam.set(from,to,up);
  // set the shape using FOV 45 Aspect Ratio based on Width and Height
  // The final two are near and far clipping planes of 0.5 and 10
  m_cam.setShape(45,(float)720.0/576.0,0.5,150);

  // grab an instance of shader manager
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  constexpr auto PhongVertex="PhongVertex";
  constexpr auto PhongFragment="PhongFragment";

  shader->createShaderProgram(Phong);

  shader->attachShader(PhongVertex,ngl::ShaderType::VERTEX);
  shader->attachShader(PhongFragment,ngl::ShaderType::FRAGMENT);
  shader->loadShaderSource(PhongVertex,"shaders/PhongVertex.glsl");
  shader->loadShaderSource(PhongFragment,"shaders/PhongFragment.glsl");

  shader->compileShader(PhongVertex);
  shader->compileShader(PhongFragment);
  shader->attachShaderToProgram(Phong,PhongVertex);
  shader->attachShaderToProgram(Phong,PhongFragment);

  shader->linkProgramObject(Phong);
  (*shader)[Phong]->use();

  // now pass the modelView and projection values to the shader
  shader->setUniform("Normalize",1);
  shader->setUniform("viewerPos",m_cam.getEye().toVec3());

  // now set the material and light values
  ngl::Material m(ngl::STDMAT::POLISHEDSILVER);
  m.loadToShader("material");
  ngl::Mat4 iv;
  iv=m_cam.getProjectionMatrix();
  //iv.transpose();

  /// now setup a basic 3 point lighting system
  ngl::Light key(ngl::Vec3(2,1,3),ngl::Colour(1,1,1,1),ngl::LightModes::POINTLIGHT);
  key.setTransform(iv);
  key.enable();
  key.loadToShader("light[0]");
  ngl::Light fill(ngl::Vec3(-2,1.5,3),ngl::Colour(1,1,1,1),ngl::LightModes::POINTLIGHT);
  fill.setTransform(iv);
  fill.enable();
  fill.loadToShader("light[1]");

  ngl::Light back(ngl::Vec3(2,1,-2),ngl::Colour(1,1,1,1),ngl::LightModes::POINTLIGHT);
  back.setTransform(iv);
  back.enable();
  back.loadToShader("light[2]");

  constexpr auto normalVertex="normalVertex";
  constexpr auto normalFragment="normalFragment";
  constexpr auto normalGeo="normalGeo";

  shader->createShaderProgram(normalShader);

  shader->attachShader(normalVertex,ngl::ShaderType::VERTEX);
  shader->attachShader(normalFragment,ngl::ShaderType::FRAGMENT);
  shader->loadShaderSource(normalVertex,"shaders/normalVertex.glsl");
  shader->loadShaderSource(normalFragment,"shaders/normalFragment.glsl");

  shader->compileShader(normalVertex);
  shader->compileShader(normalFragment);
  shader->attachShaderToProgram(normalShader,normalVertex);
  shader->attachShaderToProgram(normalShader,normalFragment);

  shader->attachShader(normalGeo,ngl::ShaderType::GEOMETRY);
  shader->loadShaderSource(normalGeo,"shaders/normalGeo.glsl");
  shader->compileShader(normalGeo);
  shader->attachShaderToProgram(normalShader,normalGeo);
  shader->linkProgramObject(normalShader);
  shader->use(normalShader);
  // now pass the modelView and projection values to the shader
  shader->setUniform("normalSize",0.1f);
  shader->setUniform("vertNormalColour",1.0f,1.0f,0.0f,1.0f);
  shader->setUniform("faceNormalColour",1.0f,0.0f,0.0f,1.0f);

  shader->setUniform("drawFaceNormals",true);
  shader->setUniform("drawVertexNormals",true);

  glEnable(GL_DEPTH_TEST); // for removal of hidden surfaces
  ngl::VAOPrimitives *prim=ngl::VAOPrimitives::instance();
  prim->createSphere("sphere",0.8f,40);
  prim->createCylinder("cylinder",0.2f,2.0f,40,40);
  prim->createCone("cone",0.8f,2.0,40,40);
  prim->createTorus("torus",0.2f,1.0f,20,20);

}


void NGLScene::loadMatricesToShader()
{
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  (*shader)[Phong]->use();
  ngl::Mat4 MV;
  ngl::Mat4 MVP;
  ngl::Mat3 normalMatrix;
  ngl::Mat4 M;
  M=m_transformStack.getMatrix()*m_mouseGlobalTX;
  MV=M*m_cam.getViewMatrix() ;
  MVP= MV*m_cam.getProjectionMatrix();
  normalMatrix=MV;
  normalMatrix.inverse();
  shader->setUniform("MV",MV);
  shader->setUniform("MVP",MVP);
  shader->setUniform("normalMatrix",normalMatrix);
  shader->setUniform("M",M);
}
void NGLScene::loadMatricesToNormalShader()
{
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  (*shader)[normalShader]->use();
  ngl::Mat4 MV;
  ngl::Mat4 MVP;

  MVP=m_transformStack.getMatrix()*m_mouseGlobalTX*m_cam.getVPMatrix();
  shader->setUniform("MVP",MVP);

}

void NGLScene::paintGL()
{
  // clear the screen and depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glViewport(0,0,m_win.width,m_win.height);
  // Rotation based on the mouse position for our global transform
   ngl::Mat4 rotX;
   ngl::Mat4 rotY;
   // create the rotation matrices
   rotX.rotateX(m_win.spinXFace);
   rotY.rotateY(m_win.spinYFace);
   // multiply the rotations
   m_mouseGlobalTX=rotY*rotX;
   // add the translations
   m_mouseGlobalTX.m_m[3][0] = m_modelPos.m_x;
   m_mouseGlobalTX.m_m[3][1] = m_modelPos.m_y;
   m_mouseGlobalTX.m_m[3][2] = m_modelPos.m_z;

  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  ngl::VAOPrimitives *prim=ngl::VAOPrimitives::instance();

  (*shader)[normalShader]->use();
  shader->setUniform("normalSize",m_normalSize);
  loadMatricesToNormalShader();
  prim->draw(m_modelName);
  glPointSize(4.0);
  glLineWidth(4.0);

  (*shader)[Phong]->use();

  loadMatricesToShader();
  prim->draw(m_modelName);

}

//----------------------------------------------------------------------------------------------------------------------

void NGLScene::keyPressEvent(QKeyEvent *_event)
{
  static bool fn=true;
  static bool vn=true;
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();

  switch (_event->key())
  {
    case Qt::Key_Plus : m_normalSize+=0.01; break;
    case Qt::Key_Minus : m_normalSize-=0.01; break;
    case Qt::Key_1 :
      fn^=true;
      (*shader)["normalShader"]->use();
      shader->setUniform("drawFaceNormals",fn);
    break;
    case Qt::Key_2 :
      vn^=true;
      (*shader)["normalShader"]->use();
      shader->setUniform("drawVertexNormals",vn);
    break;
    case Qt::Key_S : m_modelName="sphere"; break;
    case Qt::Key_T : m_modelName="teapot"; break;
    case Qt::Key_C : m_modelName="cube"; break;
    case Qt::Key_Y : m_modelName="cylinder"; break;
    case Qt::Key_N : m_modelName="cone"; break;
    case Qt::Key_R : m_modelName="torus"; break;
    case Qt::Key_Escape : QApplication::quit(); break;


  }
 update();
}
