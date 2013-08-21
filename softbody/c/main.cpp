/**
 * SoftBody implementation for CPP v0.2.1-m (With MidPoint integration) 
 * (c) Brian R. Cowan, 2007 (http://www.briancowan.net/) 
 * Examples at http://www.briancowan.net/unity/fx/
 *
 * Code provided as-is. You agree by using this code that I am not liable for any damage
 * it could possibly cause to you, your machine, or anything else. And the code is not meant
 * to be used for any medical uses or to run nuclear reactors or robots or such and so. 
 */
 
#include <math.h>
#include <stdlib.h> 
#include <time.h>
#include <iostream>


#include "SoftBody.h"
#include "Util.h"

#define kWindowWidth      1024 
#define kWindowHeight     768

SoftBody *softBody;
#define checkImageWidth 64
#define checkImageHeight 64
static GLubyte checkImage[checkImageHeight][checkImageWidth][4];

static GLuint texName;


void display(void)
{ 

  float nt=secs();
  deltaTime=nt-frmt;
  frmt=nt;
  softBody->Update();
  
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

  glLoadIdentity();  
  gluLookAt(4, 1, 1, 0.0, -2.0, 0.0, 0.0, 1.0, 0.0);

  glBlendFunc (GL_ONE, GL_ZERO);
    glEnable(GL_TEXTURE_2D);
   glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
   glBindTexture(GL_TEXTURE_2D, texName);


  glVertexPointer(3,GL_FLOAT,24,softBody->_allP);
  glTexCoordPointer(2,GL_FLOAT,0,softBody->_allUV);
  
  glDrawElements(GL_TRIANGLES, softBody->tLength , GL_UNSIGNED_INT, softBody->_allT);

  glBegin(GL_QUADS);
   glTexCoord2f(0.0, 0.0); glVertex3f(-31.0, softBody->yPlane, 15.0);
   glTexCoord2f(0.0, 1.0); glVertex3f(-31.0, softBody->yPlane, -31.0);
   glTexCoord2f(1.0, 1.0); glVertex3f(5.0, softBody->yPlane, -31.0);
   glTexCoord2f(1.0, 0.0); glVertex3f(5.0, softBody->yPlane, 15.0);

  
   glEnd();
    glDisable(GL_TEXTURE_2D);
 glBlendFunc (GL_SRC_COLOR, GL_SRC_COLOR);
glColor4f(1.0, 1.0, 0.0, 1);
//glShadeModel (GL_FLAT);
 sphere *s=softBody->spheres;
  for(int i=0;i<softBody->sLength;i++)
  {  
    glPushMatrix();
    glTranslatef(s[i][0],s[i][1],s[i][2]);
 	glutSolidSphere(s[i][3]-.012,20,20);
	glPopMatrix();
	
 
  }
  
  
  
  glFlush ();
 
  glutSwapBuffers();
  glutPostRedisplay();

} 

void reshape (int w, int h)
{
   glViewport (0, 0, (GLsizei) w, (GLsizei) h); 
   glMatrixMode (GL_PROJECTION);
   glLoadIdentity ();
   gluPerspective(60.0, (GLfloat) w/(GLfloat) h, 1.0, 30.0);
   
   glMatrixMode (GL_MODELVIEW);
   glLoadIdentity();
}

void keyboard (unsigned char key, int x, int y)
{
   switch (key) {
      case 27:  /*  Escape key  */
         exit(0);
         break;
      
      default:
         break;
   }
}


//Adapted from 'the red book'
void makeCheckImage(void)
{
   int i, j, c;
    
   for (i = 0; i < checkImageHeight; i++) {
      for (j = 0; j < checkImageWidth; j++) {
         c = ((((i&0x2)==0)^((j&0x2))==0))*4+1;
         checkImage[i][j][0] = (GLubyte) c*j+sin(((float)j)/10.0)*5.0;
         checkImage[i][j][1] = (GLubyte) c*(64-j)+cos(((float)j)/10.0)*5.0;
         checkImage[i][j][2] = (GLubyte) c*i+cos(((float)i)/10.0)*5.0;
         checkImage[i][j][3] = (GLubyte) 255;
      }
   }
}

int init()
{
   glClearColor (0.0, 0.0, 0.0, 0.0);
   GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
   GLfloat mat_shininess[] = { 50.0 };
   GLfloat light_position[] = { 1.0, 1.0, 1.0, 0.0 };
   glClearColor (0.0, 0.0, 0.0, 0.0);
   glShadeModel (GL_SMOOTH);

   glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
   glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
   glLightfv(GL_LIGHT0, GL_POSITION, light_position);

   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);
   glEnable(GL_BLEND);
   glEnable(GL_DEPTH_TEST);

   makeCheckImage();
   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

   glGenTextures(1, &texName);
   glBindTexture(GL_TEXTURE_2D, texName);

   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, 
                   GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 
                   GL_NEAREST);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, checkImageWidth, 
                checkImageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 
                checkImage);
}

int main(int argc, char** argv) 
{ 
 

  glutInit(&argc, argv); 
  glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH); 
  glutInitWindowSize (kWindowWidth, kWindowHeight); 
  glutInitWindowPosition (100, 100); 
  glutCreateWindow ("Bouncing Bob");
  
  init();

  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboard);
  
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  
  softBody=new SoftBody();
  softBody->Start();
 
  glutMainLoop();

  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  
  delete softBody;
  
  return 0;
}
