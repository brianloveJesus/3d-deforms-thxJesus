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

#include "Util.h"
#include "SoftBody.h"

using namespace std;

float deltaTime=0.0;
float frmt=0.0;
float fc=0;

SoftBody::~SoftBody()
{
  delete[] spheres;
  delete[] _allP;
  delete[] _allT;
  delete[] _allUV;
  delete[] _pP;
  
  for(int i=0;i<lLength;i++)
  {
	delete _lattice[i];
  }
  
  delete[] _lattice;
  
}
	
SoftBody::SoftBody()
{
	mass=1;
	resolutionX=7;resolutionY=3;resolutionZ=3;
	yPlane=-2.0;
	dTwoWay=true;
	gravity=.004;
	velocityCap=5;
	defaultK=1.2;
	timeStep=0.01; //better slow (or fast) and steady
	size=Vector3(1.35,.24,.23);
	dCaucho=.54;	
	dFriction=.2;
	sLength=4;	
	spheres=new sphere[sLength];
	spheres[0][0]=-.3;spheres[0][1]=-.7;spheres[0][2]=-.2;spheres[0][3]=.5;
	spheres[1][0]=.1;spheres[1][1]=-1.0;spheres[1][2]=.8;spheres[1][3]=.3;
	spheres[2][0]=-.3;spheres[2][1]=-1.02;spheres[2][2]=.5;spheres[2][3]=.3;
	spheres[3][0]=.8;spheres[3][1]=-.9;spheres[3][2]=.3;spheres[3][3]=.4;
	

	
	oneSided=false;
}
//Object Start
void SoftBody::Start () {		
	
	
	//engine specific pre start
	preStart();
	
	//Algorithm start
	initialize();
	
	//implementation/engine specific
	startEngine();	    
	
}	

//Update is called once per frame, try using FixedUpdate
void SoftBody::Update () 
{
	ftime=(timeStep>0 ? timeStep : deltaTime);
        fc++;	
	//If you want to fix the edge corners (useful for Flags and other bouncy effects)
	/*_lattice[0]->fix=true;
	_lattice[(_dimX-1)*_dimY*(_dimZ)]->fix=true;
	_lattice[_dimX*_dimY*(_dimZ-1)]->fix=true;
	_lattice[(_dimX-1)*_dimY]->fix=true;				
	*/
	
	for(int i=0;i<sLength;i++)
	{
	  spheres[i][3]+=sin(frmt*15)*.0015;//frmt*(15+(cos(frmt*.05)*10))*.2+i)*.00051;
	  //spheres[i][3]/=1.00001;
	 // spheres[i][1]+=sin(frmt*10+i)*.001;
	 // spheres[i][0]+=cos(frmt*10+i)*.001;
	  }
	  yPlane+=cos(frmt*(15+(cos(frmt*.05))) )*.002;
	//Engine specific updates for mesh 
	preUpdate();
	
	//Not implementation specific
	perFrame();
	
	//implementation/engine specific
	renderMesh();
}


/*Unity Specific pre algorithm initialization*/
float lt; //Last time message was displayed
void SoftBody::preStart()
{
	//Set internal variables, so if they are changed while in progress nothing will happen
	//Also make sure that we only have one dimension with a 1 unit marker
	_dimX=(resolutionX>0 ? resolutionX : 1);
	_dimY=(resolutionY>1 ? resolutionY : (_dimX > 1 ? 1 : 2));
	_dimZ=(resolutionZ>1 ? resolutionZ : (_dimX > 1 && _dimY > 1 ? 1 : 2));
	_sX=size.x;
	_sY=size.y;
	_sZ=size.z;
	
	//Start time counter at 0 for message
	lt=0;
	
	//Set onesided if something has a 0 dim vector
	if(_dimX==1 || _dimY==1 || _dimZ==1) {oneSided=true; }
}
		
	
    	
/*Unity Specific starting of engine
 *After the algorithm initiates the mass lattice, this method created the
 *references from the triangles to the bordering vertices.
 *
 */

void SoftBody::startEngine()
{
	int i,j;
	
	int pl=oneSided ? _dimX*_dimY*_dimZ : (_dimX*_dimY+_dimX*_dimZ+_dimY*_dimZ)*2-(_dimX+_dimY+_dimZ);
	_allP=new Vector3[pl];
	_allUV=new Vector2[pl];
	_pP=new int[pl];
	pLength=pl;
 	
	//How many triangles will there be overall? Assumes at most one dimension with a 1 dim vector   	
	int dom;
	dom=(_dimX>1 ? 
			(_dimY>1 ? 
				(_dimZ>1 ? 
					(((_dimX-1)*(_dimY-1)) + ((_dimX-1)*(_dimZ-1)) + ((_dimZ-1)*(_dimY-1)))*2 
					: ((_dimX-1)*(_dimY-1))*2)
				: ((_dimX-1)*(_dimZ-1))*2) 
			: ((_dimY-1)*(_dimZ-1))*2);
	
	tLength=3*dom*2;
			
	_allT=new int[tLength];
	
	for(i=0;i<tLength;i++)
	{
		_allT[i]=0;
	}
	
	int pc=0;
	
	//Go through all the points, and assign their positions in vertex buffer, and UV coordinates
	for(i=0;i<lLength;i++)
	{
		Vector3 *p=&(_lattice[i]->position);
		
		float ux=(float)_lattice[i]->px/(float)_dimX;
		float uy=(float)_lattice[i]->py/(float)_dimY;
		float uz=(float)_lattice[i]->pz/(float)_dimZ;
		if(_lattice[i]->pz>0 && _lattice[i]->pz<_dimZ-1) {
			if(_lattice[i]->px>0 && _lattice[i]->px<_dimX-1) {
				if(_lattice[i]->py>0 && _lattice[i]->py<_dimY-1) {
					//Not a bordering point						
					_lattice[i]->pP=0;
				} else {
					_lattice[i]->pP=pc;
					_allP[pc]=Vector3(p->x,p->y,p->z);
					_allUV[pc].x=ux;_allUV[pc].y=uz;						
					_pP[pc++]=i;						
					
				}
			} else {
				_lattice[i]->pP=pc;
				_allP[pc]=Vector3(p->x,p->y,p->z);
				_allUV[pc]=Vector2(uy,uz);					
				_pP[pc++]=i;
				
			}
		} else {
			_lattice[i]->pP=pc;
			_allP[pc]=Vector3(p->x,p->y,p->z);
			_allUV[pc]=Vector2(ux,uy);	
			_pP[pc++]=i;
						
		}
	}
	
	//Go through faces of cube and connect the triangles to the points in the Vertex buffer
	int tp=0;
	int ymul=_dimX;
	int zmul=_dimX*_dimY;
	int addt=0;

	if(_dimY>1 && _dimX>1) {
		if(!oneSided){
			for(i=0;i<_dimY-1;i++) {
				for(j=0;j<_dimX-1;j++) {
					_allT[tp++]=_lattice[i*ymul+j]->pP;	_allT[tp++]=_lattice[(i+1)*ymul+j]->pP;	_allT[tp++]=_lattice[i*ymul+j+1]->pP;
					_allT[tp++]=_lattice[i*ymul+j+1]->pP;_allT[tp++]=_lattice[(i+1)*ymul+j]->pP;	_allT[tp++]=_lattice[(i+1)*ymul+j+1]->pP;
				}
			}
		}
		
		addt=(_dimZ-1)*_dimY*_dimX;
		for(i=0;i<_dimY-1;i++) {
			for(j=0;j<_dimX-1;j++) {
				_allT[tp++]=_lattice[i*ymul+j+addt]->pP;	 _allT[tp++]=_lattice[i*ymul+j+1+addt]->pP;		_allT[tp++]=_lattice[(i+1)*ymul+j+addt]->pP;
				_allT[tp++]=_lattice[i*ymul+j+1+addt]->pP;_allT[tp++]=_lattice[(i+1)*ymul+j+1+addt]->pP;	_allT[tp++]=_lattice[(i+1)*ymul+j+addt]->pP;
			}
		}
	}
	
	//Sides
	if(_dimZ>1) {
		if(_dimX>1) {
			//z&x
			if(!oneSided){
				for(i=0;i<_dimZ-1;i++) {
					for(j=0;j<_dimX-1;j++) {						
						_allT[tp++]=_lattice[i*zmul+j]->pP;	_allT[tp++]=_lattice[i*zmul+j+1]->pP;_allT[tp++]=_lattice[(i+1)*zmul+j]->pP;
						_allT[tp++]=_lattice[i*zmul+j+1]->pP;_allT[tp++]=_lattice[(i+1)*zmul+j+1]->pP;_allT[tp++]=_lattice[(i+1)*zmul+j]->pP;
					}
				}
			}
			addt=(_dimX)*(_dimY-1);
			for(i=0;i<_dimZ-1;i++) {
				for(j=0;j<_dimX-1;j++) {
					_allT[tp++]=_lattice[i*zmul+j+addt]->pP;	 _allT[tp++]=_lattice[(i+1)*zmul+j+addt]->pP;_allT[tp++]=_lattice[i*zmul+j+1+addt]->pP;
					_allT[tp++]=_lattice[i*zmul+j+1+addt]->pP;_allT[tp++]=_lattice[(i+1)*zmul+j+addt]->pP;_allT[tp++]=_lattice[(i+1)*zmul+j+1+addt]->pP;
				}
			}
		} 
		if(_dimY>1){
			//y&x
			if(!oneSided){
				for(i=0;i<_dimZ-1;i++) {
					for(j=0;j<_dimY-1;j++) {
						_allT[tp++]=_lattice[i*zmul+j*ymul]->pP;		_allT[tp++]=_lattice[(i+1)*zmul+j*ymul]->pP;_allT[tp++]=_lattice[i*zmul+(j+1)*ymul]->pP;
						_allT[tp++]=_lattice[i*zmul+(j+1)*ymul]->pP;	_allT[tp++]=_lattice[(i+1)*zmul+j*ymul]->pP;_allT[tp++]=_lattice[(i+1)*zmul+(j+1)*ymul]->pP;
					}
				}
			}
			addt=(_dimX-1);
			for(i=0;i<_dimZ-1;i++) {
				for(j=0;j<_dimY-1;j++) { 
					_allT[tp++]=_lattice[i*zmul+j*ymul+addt]->pP;_allT[tp++]=_lattice[i*zmul+(j+1)*ymul+addt]->pP;	_allT[tp++]=_lattice[(i+1)*zmul+j*ymul+addt]->pP;
					
					_allT[tp++]=_lattice[i*zmul+(j+1)*ymul+addt]->pP;_allT[tp++]=_lattice[(i+1)*zmul+(j+1)*ymul+addt]->pP;_allT[tp++]=_lattice[(i+1)*zmul+j*ymul+addt]->pP;
				}
			}
		}
	}

		
	
}
	
/*Unity Specific positioning of Collision Objects
 *Currently only spheres and yPlane. Will inverse transform their positions,
 *it is set in such a way that Spheres may be moved/removed/added every frame.
 */
void SoftBody::preUpdate()
{
		
	_yFix=yPlane;
}
	
/*Unity and Sample Specific
 *Code to update the displayed mesh with the calculated lattice
 */
void SoftBody::renderMesh()
{
 
	int i;	
								
	
	
	for(i=0;i<pLength;i++)
	{
		_allP[i]=_lattice[_pP[i]]->position;
	}
			

	if(lt+1<frmt) {
		lt=frmt;
		cout <<fc<<" FPS\n";
		fc=0;
	}
	
}
	

   
		//Give coordinates into lattice, find a mass
CMass * SoftBody::findMass(int x,int y,int z)
{
	if(x<0 || y<0 || z<0 || x>=_dimX || y>=_dimY || z>=_dimZ) { return null; }
	else {return _lattice[z*_dimX*_dimY + y*_dimX + x];}
	
}

//Every frame, clear the forces, and add gravity otherwise things may become a little unstable...
void SoftBody::clearForces()
{
	for(int i=0;i<lLength;i++)
	{
		_lattice[i]->force.x=0;_lattice[i]->force.y=-gravity;_lattice[i]->force.z=0;
		_lattice[i]->force_h.x=0;_lattice[i]->force_h.y=-gravity;_lattice[i]->force_h.z=0;

	}
}
	
	
	
//Algorithm specific per-frame calculations
void SoftBody::perFrame()
{		
	fptr++;
	clearForces();
	for(int i=0;i<lLength;i++)
	{
		CMass *ptr=_lattice[i];
		ptr->calcCons_h(); //calculate addition of all the forces
		ptr->doMovement_h(); //move according to time and calculated force, includes collision detection			
	}
	for(int i=0;i<lLength;i++)
	{
		CMass *ptr=_lattice[i];
		ptr->calcCons(); //calculate addition of all the forces
		ptr->doMovement(); //move according to time and calculated force, includes collision detection			
	}

}
	
	
//Initialize algorithm, including the mass lattice
void SoftBody::initialize() 
{
	int jx,jy,jz;
	//float vx,vy,vz;
	int i;
	int massCount=_dimX*_dimY*_dimZ;
	
	//How much to multiply forces by
	_mEach=(float)(_dimX*_dimY*_dimZ)/mass;
	
	//Initialize frame counter to 0
	fptr=0;
	lLength=massCount;
	_lattice=new CMass*[lLength];
	i=0;
	for(jz=0;jz<_dimZ;jz++)
	{
		for(jy=0;jy<_dimY;jy++)
		{
			for(jx=0;jx<_dimX;jx++)
			{
				_lattice[i++]=new CMass(this,jx,jy,jz);
				
			}
		}			
	}
	
	i=0; while(i<massCount) { _lattice[i++]->makeLattice(); }
	
	
	
}



