/**
 * SoftBody implementation for CPP v0.2.1-m (With MidPoint integration) 
 * (c) Brian R. Cowan, 2007 (http://www.briancowan.net/) 
 * Examples at http://www.briancowan.net/unity/fx/
 *
 * Code provided as-is. You agree by using this code that I am not liable for any damage
 * it could possibly cause to you, your machine, or anything else. And the code is not meant
 * to be used for any medical uses or to run nuclear reactors or robots or such and so. 
 */
#ifndef __SOFTBODY_H__
#include "Util.h"

extern float deltaTime;
extern float frmt;
extern int maxCon;
extern int allAxis[][3];

class SoftBody;
class CMConnect;
class CMass;

//One Connection to another mass
class CMConnect
{
    friend class CMass;
	
public:
	//The original and connected to masses
	//This is done so the connected to mass can be updated with the inverse of the
	//calculated force without recalculating it when it is revisited
	CMass *cm;
	CMass *alter;
	
	//Rest length of connection
	float length;
	
	//Springyness
	float k;
	
	//Two way? (For jello/pillow/folds in cloth)
	bool twoWay;    		
	
	//axis id of hinge
	int axis;
	
	//alternate (reversed) axis of hinge
	int alterAxis;
	
	//Last frame connection calculated at (if it is calculated from the reverse connection, then don't recalc)
	int lptr,lptr_h;    		
	
	//Calculates the connection variables
	CMConnect(CMass *cm,int axis,CMass *alter);

		
	//Calculate the force that is exerted by this spring on both the connected masses
	//Uses a general Newtonian formula f=kd
	void addForce();
	void addForce_h();
	
};
	
 //Class for one mass point in the soft body
class CMass
{
	
	friend class SoftBody;
	friend class CMConnect;
	
	//SoftBody this mass belongs to
	SoftBody *myBody;
	
	//discrete coordinates in the lattice
	int px;
	int py; 
	int pz;
	

	//Newtonian variables in threespace
	Vector3 force;
	Vector3 force_h;
	Vector3 velocity;
	Vector3 velocity_h;
	Vector3 position;
	Vector3 position_h;
	
	//Pointer into the displayed triangle list
	int pP;
	
	//Is it fixed? (unmoving)
	bool fix;
		
	//All the connections to other masses (Refer to SoftBody.allAxis to see their relation)
	CMConnect **_cons;    	
	
	
	//Initialize the mass, its starting position and forces, and allocate for Connections
	CMass(SoftBody *myBody, int px,int py, int pz);
	~CMass();
	
	//Calculate the forces across all the connections to this object, but only connections that haven't already
	//been calculated for (due to inverse axis)
	void calcCons();
	void calcCons_h();
	
	
	//Once forces have been calculated, add forces to velocity, cap it, and add velocity to position, then check collisions
	void doMovement();
	void doMovement_h();
	
		
	//Collision interaction
	void doCollision();
	void doCollision_h();
	
	
	
	//Initiate all the connections for this mass
	int makeLattice();
	
	
	
};


class SoftBody {
	friend class CMass;
	friend class CMConnect;
	
private:	
	//This is the lattice of masses that make up the Soft Body
	CMass **_lattice;
	
	//private one sided only
	byte oneSided;

protected:
	
	//Internal: Dimensions of connections
	int _dimX,_dimY,_dimZ;
	
    //frame pointer, to see if a connection has already been calculated for
	int fptr;
	
	//Distance in time to calculate for
	float ftime;
	
	//Internal Distance to ground, calculated every frame from transform 
	float _yFix;
	
	//Default Springyness damper/amplifier against hard surfaces
	float dCaucho;
	
	//Default Friction against hard surfaces
	float dFriction;
	
	//Internal: The size in X and Y and Z
	float _sX,_sY,_sZ;	
	
	//To use to multiply force by (amount of masses/total mass)
	float _mEach;
	
public:
	//Spheres that will be calculated for
	sphere *spheres;

	//All the points, UVs, Triangles, and reverse lookup of points
	Vector3 *_allP;
	Vector2 *_allUV;
	int *_allT;
	int *_pP;
	
	//Total mass of object	
	float mass;
	
	//Properties, the height, width, and length of the SoftBody (currently don't use scale)
	Vector3 size;
	
	//Properties, the resolution of the mesh in X,Y,Z
	//For cloth, setting Y to two or three and the others to 16+ gives generally good results 
	int resolutionX;
	int resolutionY;
	int resolutionZ;
	
	//Public interface to set the YPlane distance to the ground;
	float yPlane;		
	
	//Should the springs between the masses push and pull, or pull only?
	bool dTwoWay;
	
	//Gravity, won't affect much if the masses are already going at the maximum speed
	float gravity;
    
    //Velocity magnitude capping, if not set the system will eventually destabalize as it is
    //currently implemented
	float velocityCap;
	
    //Default elasticity of connections, lower=stretchier, higher=more rigid
 	float defaultK;

    //The amount to step every frame, setting to 0 will use deltaTime
    //(setting to deltaTime can give a wider variation in results every time it is replayed)
    float timeStep; 
	
	int pLength,tLength,lLength,sLength;
	
	SoftBody();
	~SoftBody();
	
	//Object Start
	void Start ();	

	//Update is called once per frame, try using FixedUpdate
	void Update () ;
	


	/*Unity Specific pre algorithm initialization*/
	float lt; //Last time message was displayed
	void preStart();
	
		
	
    	
	/*Unity Specific starting of engine
	 *After the algorithm initiates the mass lattice, this method created the
	 *references from the triangles to the bordering vertices.
	 *
	 */
	
	void startEngine();
	
	
	/*Unity Specific positioning of Collision Objects
	 *Currently only spheres and yPlane. Will inverse transform their positions,
	 *it is set in such a way that Spheres may be moved/removed/added every frame.
	 */
	void preUpdate();
	
	
	/*Unity and Sample Specific
	 *Code to update the displayed mesh with the calculated lattice
	 */
	void renderMesh();	

   
	//Give coordinates into lattice, find a mass
	CMass * findMass(int x,int y,int z);
	
	
	//Every frame, clear the forces, and add gravity otherwise things may become a little unstable...
	void clearForces();	
	
	
	//Algorithm specific per-frame calculations
	void perFrame();	
	
	
	//Initialize algorithm, including the mass lattice
	void initialize(); 
	
};

#endif
