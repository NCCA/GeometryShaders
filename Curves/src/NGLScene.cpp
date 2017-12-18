#include <QMouseEvent>
#include <QApplication>

#include "NGLScene.h"
#include <ngl/Transformation.h>
#include <ngl/NGLInit.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/ShaderLib.h>
#include <ngl/SimpleIndexVAO.h>


NGLScene::NGLScene()
{
  setTitle("Simple Geometry Shader");

}


NGLScene::~NGLScene()
{
  std::cout<<"Shutting down NGL, removing VAO's and Shaders\n";
}

void NGLScene::resizeGL( int _w, int _h )
{
  m_project=ngl::perspective(45.0f,static_cast<float>( _w ) / _h, 0.05f, 350.0f );
  m_win.width  = static_cast<int>( _w * devicePixelRatio() );
  m_win.height = static_cast<int>( _h * devicePixelRatio() );
}



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
  ngl::Vec3 from(5,5,5);
  ngl::Vec3 to(0,0,0);
  ngl::Vec3 up(0,1,0);
  m_view=ngl::lookAt(from,to,up);

  // grab an instance of shader manager
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  shader->loadShader("CurveShader","shaders/curveVertex.glsl",
                     "shaders/curveFragment.glsl",
                     "shaders/curveGeometry.glsl");
  shader->use("CurveShader");
  shader->setUniform("steps",0.01f);
  GLuint id=shader->getProgramID("CurveShader");
  m_subroutines[0] = glGetSubroutineIndex(id, GL_GEOMETRY_SHADER, "bezier");
  m_subroutines[1] = glGetSubroutineIndex(id, GL_GEOMETRY_SHADER, "lerpCurve");


  m_vao.reset(ngl::VAOFactory::createVAO(ngl::simpleIndexVAO,GL_LINE_STRIP_ADJACENCY));
  createVAO();
  glEnable(GL_DEPTH_TEST); // for removal of hidden surfaces

}

void NGLScene::createVAO()
{
  std::vector<ngl::Vec3> controlPoints={
    ngl::Vec3(0.0f, 4.0f,-2.0f),
    ngl::Vec3(0.0f, 3.0f,1.0f),
    ngl::Vec3(0.0f,-3.0f,1.0f),
    ngl::Vec3(0.0f,-4.0f,-2.0f),

    ngl::Vec3(-1.0f, 4.0f,-2.0f),
    ngl::Vec3(-1.0f, 3.0f,1.0f),
    ngl::Vec3(-1.0f,-3.0f,1.0f),
    ngl::Vec3(-1.0f,-4.0f,-2.0f),

    ngl::Vec3(1.0f, 4.0f,-4.0f),
    ngl::Vec3(2.0f, 2.0f,1.0f),
    ngl::Vec3(1.0f,-3.0f,-1.0f),
    ngl::Vec3(1.0f,-2.0f,-5.0f),

  };
  std::vector<GLshort> index={0,1,2,3,9999,4,5,6,7,9999,
                             8,9,10,11};
  m_vao->bind();

    // in this case we are going to set our data as the vertices above

   m_vao->setData(ngl::SimpleIndexVAO::VertexData(
                                                    controlPoints.size()*sizeof(ngl::Vec3),
                                                    controlPoints[0].m_x,
                                                    index.size()*sizeof(GLshort),&index[0],
                                                    GL_UNSIGNED_SHORT));
    // data is 24 bytes apart ( two Vec3's) first index
    // is 0 second is 3 floats into the data set (i.e. vec3 offset)
    m_vao->setVertexAttributePointer(0,3,GL_FLOAT,0,0);
    m_vao->setNumIndices(index.size());
    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(9999);

   // now unbind
    m_vao->unbind();
}

void NGLScene::loadMatricesToShader()
{
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  shader->use("CurveShader");
  shader->setUniform("MVP",m_project*m_view*m_mouseGlobalTX);
  glUniformSubroutinesuiv(GL_GEOMETRY_SHADER,1,&m_subroutines[m_activeSubroutine]);

}
void NGLScene::loadMatricesToColourShader(const ngl::Vec4 &_colour)
{
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  shader->use(ngl::nglColourShader);
  shader->setUniform("Colour",_colour);
  shader->setUniform("MVP",m_project*m_view*m_mouseGlobalTX);
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

   loadMatricesToShader();
   m_vao->bind();
   m_vao->setMode(GL_LINE_STRIP_ADJACENCY);
   m_vao->draw();
   glPointSize(14.0f);
   loadMatricesToColourShader(ngl::Vec4(1.0f,1.0f,0.0f,1.0f));
   m_vao->setMode(GL_POINTS);
   m_vao->draw();
   loadMatricesToColourShader(ngl::Vec4(1.0f,0.0f,0.0f,1.0f));

   m_vao->setMode(GL_LINE_STRIP);
   m_vao->draw();


   m_vao->unbind();

}

//----------------------------------------------------------------------------------------------------------------------

void NGLScene::keyPressEvent(QKeyEvent *_event)
{
  switch (_event->key())
  {
    case Qt::Key_Escape : QApplication::quit(); break;
    case Qt::Key_1 : m_activeSubroutine =0; break;
    case Qt::Key_2 : m_activeSubroutine =1; break;

  }
 update();
}
