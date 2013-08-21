#ifndef __SFTBD_UTIL_H__
#define __SFTBD_UTIL_H__

#include <math.h>

//This is for osx
//#include <GLUT/glut.h> /* Header File For The GLut Library*/
//#include <OpenGL/gl.h>

//this for linux
#include <GL/glut.h> /* Header File For The GLut Library*/
#include <GL/gl.h> /* Header File For The GLut Library*/


typedef unsigned char byte;
typedef GLfloat point[3]; 
typedef GLfloat sphere[4];
#define null 0


float secs();

class Vector2{
  public:
	float x,y;
	Vector2()
	{
		x=0;y=0;
	}
	Vector2(float xx,float yy)
	{
		this->x=xx;
		this->y=yy;
	}
	
};


class Vector3{
  
  public:
	float x,y,z;
	float * index[3];
	
	float operator[](int idx)
	{
	    return *index[idx];
		
	}
	
	void setIndex(int idx,float val)
	{
	    *index[idx]=val;		
	}
	
	
	Vector3 operator*(float t)
	{
		return Vector3(x*t,y*t,z*t);
	}
	
	Vector3 operator+(Vector3 p)
	{
		return Vector3(x+p.x,y+p.y,z+p.z);
	}
	Vector3 operator-(Vector3 p)
	{
		return Vector3(x-p.x,y-p.y,z-p.z);
	}
	
	Vector3()
	{
		x=0;y=0;z=0;
		index[0]=&x; index[1]=&y; index[2]=&z;
	}
	Vector3(float x,float y, float z)
	{
		this->x=x;
		this->y=y;
		this->z=z;
		index[0]=&(this->x); index[1]=&(this->y); index[2]=&(this->z);
	}
	
	float magnitude()
	{
		return sqrt(x*x+y*y+z*z);
	}
	Vector3 normalized()
	{
		float oom=1.0/magnitude();
		return Vector3(x*oom,y*oom,z*oom);
		
	}
	
	float dot(Vector3 other)
	{
	   return x*other.x+y*other.y+z*other.z;
	}
	
	
};

#endif
