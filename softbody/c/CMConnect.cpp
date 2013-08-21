/**
 * SoftBody implementation for CPP v0.2.1-m (With MidPoint integration) 
 * (c) Brian R. Cowan, 2007 (http://www.briancowan.net/) 
 * Examples at http://www.briancowan.net/unity/fx/
 *
 * Code provided as-is. You agree by using this code that I am not liable for any damage
 * it could possibly cause to you, your machine, or anything else. And the code is not meant
 * to be used for any medical uses or to run nuclear reactors or robots or such and so. 
 */

#include "Util.h"
#include "SoftBody.h"

CMConnect::CMConnect(CMass *cm,int axis,CMass *alter)
{
	lptr=0;
	lptr_h=0;
	this->cm=cm;
	this->alter=alter;
	this->axis=axis;
		
	//Use Softbody defaults at first    			
	k=cm->myBody->defaultK;
	twoWay=cm->myBody->dTwoWay;    			
	
	//Calculate the rest lengths in x/y/z to the next mass
	float dx=(cm->myBody->_dimX>1 ? cm->myBody->_sX/((float)cm->myBody->_dimX-1) : 0);
	float dy=(cm->myBody->_dimY>1 ? cm->myBody->_sY/((float)cm->myBody->_dimY-1) : 0);
	float dz=(cm->myBody->_dimZ>1 ? cm->myBody->_sZ/((float)cm->myBody->_dimZ-1) : 0);
	
	//The alternate axis is always the total amount of axis-this axis
	this->alterAxis=(maxCon-1)-axis;    			
	
	//Calculate the length, taking the square of the steps in x,y,z in that axis
	int *aa=allAxis[axis];
	this->length=sqrt( dx*dx*(aa[0]*aa[0]) + dy*dy*(aa[1]*aa[1]) + dz*dz*(aa[2]*aa[2]));
				
}

void CMConnect::addForce()
{


	this->lptr=cm->myBody->fptr;
	Vector3 dir=alter->position_h-cm->position_h;
	float tLength=dir.magnitude();
	
	float tDelta=(tLength-this->length); 
	 
	//Only calculate if the spring is a two way, or stretched beyond rest length
	if(tDelta>0 || twoWay) { 
		
		float forcem=(k*(tDelta));	    			
		Vector3 nForce=dir*forcem;
		
		cm->force=cm->force+nForce;
		CMConnect *acmC=alter->_cons[alterAxis];
		
		//add inverse to alternate mass if it hasn't been calculated through this axis
		//if(acmC==null) {Debug.Log(axis+"&"+alterAxis+" - "+cm.px+","+cm.py+","+cm.pz);} Debug purposes
		if( acmC->lptr<this->lptr) {
			acmC->lptr=this->lptr;
			alter->force=alter->force-nForce;
		}
	} else{
		//set inverse as calculated, even though nothing was added
		CMConnect *acmC=alter->_cons[alterAxis];
		if( acmC->lptr<this->lptr) {
			acmC->lptr=this->lptr;
		}
	}

}

void CMConnect::addForce_h()
{  			
	this->lptr_h=cm->myBody->fptr;
	Vector3 dir=alter->position-cm->position;
	float tLength=dir.magnitude();
	
	float tDelta=(tLength-length); 
	 
	//Only calculate if the spring is a two way, or stretched beyond rest length
	if(tDelta>0 || twoWay) { 
		
		float forcem=(k*(tDelta));	    			
		Vector3 nForce=dir*forcem;
		
		cm->force_h=cm->force_h+nForce;
		CMConnect *acmC=alter->_cons[alterAxis];
		
		//add inverse to alternate mass if it hasn't been calculated through this axis
		//if(acmC==null) {Debug.Log(axis+"&"+alterAxis+" - "+cm.px+","+cm.py+","+cm.pz);} Debug purposes
		if( acmC->lptr_h<this->lptr_h) {
			acmC->lptr_h=this->lptr_h;
			alter->force_h=alter->force_h-nForce;
		}
	} else{
		//set inverse as calculated, even though nothing was added
		CMConnect *acmC=alter->_cons[alterAxis];
		if( acmC->lptr_h<this->lptr_h) {
			acmC->lptr_h=this->lptr_h;
		}
	}

}
	

