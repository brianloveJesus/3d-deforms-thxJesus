/**
 * SoftBody implementation for CPP v0.2.1-m (With MidPoint integration) 
 * (c) Brian R. Cowan, 2007 (http://www.briancowan.net/) 
 * Examples at http://www.briancowan.net/unity/fx/
 *
 * Code provided as-is. You agree by using this code that I am not liable for any damage
 * it could possibly cause to you, your machine, or anything else. And the code is not meant
 * to be used for any medical uses or to run nuclear reactors or robots or such and so. 
 */

#include "SoftBody.h"

CMass::CMass(SoftBody *myBody, int px,int py, int pz)
{
	float vx=0;float vy=0;float vz=0;
	

	if(myBody->_dimX>1) { vx=-(myBody->_sX*.5f)+((myBody->_sX/((float)myBody->_dimX-1))*((float)px)); }
	if(myBody->_dimY>1) { vy=-(myBody->_sY*.5f)+((myBody->_sY/((float)myBody->_dimY-1))*((float)py)); }
	if(myBody->_dimZ>1) { vz=-(myBody->_sZ*.5f)+((myBody->_sZ/((float)myBody->_dimZ-1))*((float)pz)); }  
						
	force=Vector3(0,0,0);
	velocity=Vector3(0,0,0);
	position=Vector3(vx,vy,vz);
	velocity_h=Vector3(0,0,0);
	force_h=Vector3(0,0,0);
	position_h=Vector3(0,0,0);
	
	this->px=px; 
	this->py=py; 
	this->pz=pz;
	
	this->myBody=myBody;
	
	
	this->fix=false;
	
	_cons=new CMConnect*[maxCon]; 		   		
	
}

CMass::~CMass()
{
	for(int i=0;i<maxCon;i++)
	{
		if(_cons[i]!=null) { delete _cons[i]; }
	}
	delete[] _cons;
}

void CMass::calcCons()
{
	for(int j=0;j<maxCon;j++)
	{
		CMConnect *c=this->_cons[j];
		if(c!=null && c->lptr<myBody->fptr) {
			c->addForce();
			
		}					
	}
	
}

void CMass::calcCons_h()
{
	for(int j=0;j<maxCon;j++)
	{
		CMConnect *c=this->_cons[j];
		if(c!=null && c->lptr<myBody->fptr) {
			c->addForce_h();
			
		}					
	}	
}

//Once forces have been calculated, add forces to velocity, cap it, and add velocity to position, then check collisions
void CMass::doMovement()
{

	if(!this->fix) {
		float t=this->myBody->ftime;
		
		//If there are more masses, each one would weigh less so force would be more effective		
		this->velocity=this->velocity+(this->force*this->myBody->_mEach*t);	        		
		
		//No mega numeric instability explosions!
		if( (velocity.magnitude())>(myBody->velocityCap)) { 
			this->velocity=(velocity.normalized()) * (myBody->velocityCap); 
		}
		this->position=this->position+(this->velocity+this->velocity_h*.5f)*t;
		
		//check collisions, which will move the mass if collided
		doCollision();
	}
		
		
}
void CMass::doMovement_h()
{
	
	if(!this->fix) {
		float t=this->myBody->ftime;		

		this->velocity_h=this->velocity+(this->force_h*this->myBody->_mEach*(t*(float).5));
		if(this->velocity_h.magnitude()>myBody->velocityCap) { this->velocity_h=this->velocity_h.normalized()*myBody->velocityCap; }
   
		this->position_h=this->position+(this->velocity*(t*.5f));
		
		doCollision_h();
		
	}
		
		
}

void CMass::doCollision()
{
	
	sphere *s=myBody->spheres;
	
	
	//zfloor check, takes in account friction and floor bouncyness	
	if(this->position.y<myBody->_yFix) {
		this->position.y=myBody->_yFix;
		this->velocity.x=this->velocity.x*myBody->dFriction;
		this->velocity.z=this->velocity.z*myBody->dFriction;
		this->velocity.y=-this->velocity.y*myBody->dCaucho;
	}
	//Sphere collisions, inexact but they work
	for(int i=0;i<myBody->sLength;i++)
	{
		
		Vector3 c=Vector3(s[i][0],s[i][1],s[i][2]);
		//Vector3 op=this->position;
		Vector3 d=this->position-c;
		float r=s[i][3];
		float dst=d.magnitude();
		if(dst<r)
		{
			Vector3 u=d.normalized();	
			
			float dot=this->velocity.normalized().dot(u);
			float fric=(this->force.magnitude()*dot*myBody->dFriction);
			this->velocity=(this->velocity*fric*(1-dot))+(d.normalized()*dot*this->velocity.magnitude())*myBody->dCaucho;	
			this->position=this->velocity*(r-dst)+c+(u*r);
		}
	}
}
	
	
void CMass::doCollision_h()
{
	
	sphere *s=myBody->spheres;
	
	
	//zfloor check, takes in account friction and floor bouncyness	
	if(this->position_h.y<myBody->_yFix) {
		this->position_h.y=myBody->_yFix;
		this->velocity_h.x=this->velocity_h.x*myBody->dFriction;
		this->velocity_h.z=this->velocity_h.z*myBody->dFriction;
		this->velocity_h.y=-this->velocity_h.y*myBody->dCaucho;
	}
	//Sphere collisions, inexact but they work
	for(int i=0;i<myBody->sLength;i++)
	{
		
		Vector3 c=Vector3(s[i][0],s[i][1],s[i][2]);
		//Vector3 op=this->position;
		Vector3 d=this->position_h-c;
		float r=s[i][3];
		float dst=d.magnitude();
		if(dst<r)
		{
			Vector3 u=d.normalized();	
			
			float dot=this->velocity_h.normalized().dot(u);
			float fric=(this->force_h.magnitude()*dot*myBody->dFriction);
			this->velocity_h=((d.normalized()*this->velocity_h.magnitude()*dot)*myBody->dCaucho)+(this->velocity_h*fric*(1-dot));	
			this->position_h=this->velocity_h*(r-dst)+c+(u*r);
		}
	}

}


int CMass::makeLattice()
{
	CMass *tm;
	int count=0;
	int i;
	            
	for(i=0;i<maxCon;i++) 
	{
		int *aa=allAxis[i];
		tm=myBody->findMass(px+aa[0],py+aa[1],pz+aa[2]);
		if(tm!=null) {_cons[i]=new CMConnect(this,i,tm); count++; } else {_cons[i]=null;}
	}
	
	return count;
		
	
}