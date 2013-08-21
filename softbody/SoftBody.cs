/**
 * SoftBody implementation for Unity v0.2.1 
 * (c) Brian R. Cowan, 2007 (http://www.briancowan.net/) 
 * Examples at http://www.briancowan.net/unity/fx/
 *
 * Code provided as-is. You agree by using this code that I am not liable for any damage
 * it could possibly cause to you, your machine, or anything else. And the code is not meant
 * to be used for any medical uses or to run nuclear reactors or robots or such and so. 
 * 
 * Should be easily portable to any other language, all Unity Specific code is labeled so,
 * adapt it to any other environment. To use, attach the script to an empty object with a
 * Mesh Renderer and Mesh Filter. Translate it wherever and adjust parameters as desired, 
 * create some spheres that are uniformally scaled, and tag them with a "SoftBody" tag. Set
 * the yPlane to some distance (usually below 0) that you would like the plane to end at. Feel
 * free to put a plane there as well. Some parameters will not affect the cube while the engine 
 * is running. Positioning/Transforming/Rotating the object after initialization will have 
 * strange results. Only one dimension may be set to a resolution of 1, and then the surface
 * will only show one side of the cloth (I prefer to simulate it with a small width and a 3
 * resolution for the this side).
 *  
 * To make the class more editable, The Layout of the file is:
 * 1) Public Properties (setable in the editor)
 * 2) Update and Start (what you will be most interested in modifying)
 * 3) Utility Functions, that interface the code of the algorithm with Unity
 * 4) Algorithm classes, should have minimal unity dependance
 * 5) Algorithm variables (protected and private)
 * 6) Algorithm functions, iterate over the algorithm
 * 7) Table(s), used to calculate in the algorithm
 *
 * Id love to see any project you use the code in.
 *
 * Mail any comments to: brian@briancowan.net (Vector426 on freenode.net's #otee-unity )
 *
 * Cheers & God bless
 */
using UnityEngine;
using System.Collections;

public class SoftBody : MonoBehaviour {
	
	//private one sided only
	private bool oneSided=false;
	
	//Total mass of object	
	public float mass=1f;
	
	//Properties, the height, width, and length of the SoftBody (currently don't use scale)
	public Vector3 size=new Vector3(1,1,1);
	
	//Properties, the resolution of the mesh in X,Y,Z
	//For cloth, setting Y to two or three and the others to 16+ gives generally good results 
	public int resolutionX=7;
	public int resolutionY=7;
	public int resolutionZ=7;
	
	//Public interface to set the YPlane distance to the ground;
	public float yPlane=3.0f;		
	
	//Should the springs between the masses push and pull, or pull only?
	public bool dTwoWay=true;
	
	//Gravity, won't affect much if the masses are already going at the maximum speed
	public float gravity=.02f;
    
    //Velocity magnitude capping, if not set the system will eventually destabalize as it is
    //currently implemented
	public float velocityCap=5f;
	
    //Default elasticity of connections, lower=stretchier, higher=more rigid
 	public float defaultK=2.5f;

    //The amount to step every frame, setting to 0 will use deltaTime
    //(setting to deltaTime can give a wider variation in results every time it is replayed)
    public float timeStep=.01f; 
	
	//Unity Start
	void Start () {		
		
		
		//engine specific pre start
		preStart();
		
	    //Algorithm start
	    initialize();
	    
	    //implementation/engine specific
	    startEngine();	    
    	
	}	

	//Update is called once per frame, try using FixedUpdate
	void Update () {
		ftime=(timeStep>0 ? timeStep : Time.deltaTime);
		
		//If you want to fix the edge corners (useful for Flags and other bouncy effects)
	//	for(int i=0;i<_lattice.Length;i++) { _lattice[i].fix=true; }		
		/*_lattice[0].fix=true;
		_lattice[(_dimX-1)*_dimY*(_dimZ)].fix=true;
		_lattice[_dimX*_dimY*(_dimZ-1)].fix=true;
		_lattice[(_dimX-1)*_dimY].fix=true;				
		*/
		
		//Engine specific updates for mesh 
		preUpdate();
		
	    //Not implementation specific
	    perFrame();
	    
	    //implementation/engine specific
	    renderMesh();
	}


	/*Unity Specific pre algorithm initialization*/
	private float lt; //Last time message was displayed
	void preStart()
	{
		//Set internal variables, so if they are changed while in progress nothing will happen
		//Also make sure that we only have one dimension with a 1 unit marker
		_dimX=(resolutionX>0 ? resolutionX : 1);
		_dimY=(resolutionY>1 ? resolutionY : (_dimX > 1 ? 1 : 2));
		_dimZ=(resolutionZ>1 ? resolutionZ : (_dimX > 1 && _dimY > 1 ? 1 : 2));
		_sX=size.x;
		_sY=size.y;
		_sZ=size.z;
		
		//Not to be changed...
		transform.localScale=new Vector3(1,1,1);
		transform.eulerAngles=new Vector3(0,0,0);

		//Start time counter at 0 for message
		lt=0f;
		
		//Set onesided if something has a 0 dim vector
		if(_dimX==1 || _dimY==1 || _dimZ==1) {oneSided=true; }
	}
		
	
    	
	/*Unity Specific starting of engine
	 *After the algorithm initiates the mass lattice, this method created the
	 *references from the triangles to the bordering vertices.
	 *
	 */
	void startEngine()
	{
		int i,j;
				
		Mesh mesh=new Mesh();
		((MeshFilter) GetComponent("MeshFilter")).mesh=mesh;
		int pl=oneSided ? _dimX*_dimY*_dimZ : (_dimX*_dimY+_dimX*_dimZ+_dimY*_dimZ)*2-(_dimX+_dimY+_dimZ);
		_allP=new Vector3[pl];
    	_allUV=new Vector2[pl];
    	_pP=new int[pl];
    	
    	for(i=0;i<_allP.Length;i++)
    	{
    		_allP[i]=new Vector3(0,0,0);
    		_allUV[i]=new Vector2(0,0);
    		_pP[i]=0;
    	}
    	
    	//How many triangles will there be overall? Assumes at most one dimension with a 1 dim vector
    	
    	
    	int dom;
		dom=(_dimX>1 ? 
				(_dimY>1 ? 
					(_dimZ>1 ? 
						(((_dimX-1)*(_dimY-1)) + ((_dimX-1)*(_dimZ-1)) + ((_dimZ-1)*(_dimY-1)))*2 
						: ((_dimX-1)*(_dimY-1))*2)
					: ((_dimX-1)*(_dimZ-1))*2) 
				: ((_dimY-1)*(_dimZ-1))*2);
		
				
    	_allT=new int[3*dom*2];
    	
    	for(i=0;i<_allT.Length;i++)
    	{
    		_allT[i]=0;
    	}
    	
    	int pc=0;
    	
    	//Go through all the points, and assign their positions in vertex buffer, and UV coordinates
    	for(i=0;i<_lattice.Length;i++)
		{
			Vector3 p=_lattice[i].position;
			
			float ux=(float)_lattice[i].px/(float)_dimX;
			float uy=(float)_lattice[i].py/(float)_dimY;
			float uz=(float)_lattice[i].pz/(float)_dimZ;
			if(_lattice[i].pz>0 && _lattice[i].pz<_dimZ-1) {
				if(_lattice[i].px>0 && _lattice[i].px<_dimX-1) {
					if(_lattice[i].py>0 && _lattice[i].py<_dimY-1) {
						//Not a bordering point						
						_lattice[i].pP=0;
					} else {
						_lattice[i].pP=pc;
						_allP[pc]=new Vector3(p.x,p.y,p.z);
						_allUV[pc]=new Vector2(ux,uz);
						_pP[pc++]=i;						
						
					}
				} else {
					_lattice[i].pP=pc;
					_allP[pc]=new Vector3(p.x,p.y,p.z);
					_allUV[pc]=new Vector2(uy,uz);
					_pP[pc++]=i;
					
				}
			} else {
				_lattice[i].pP=pc;
				_allP[pc]=new Vector3(p.x,p.y,p.z);
				_allUV[pc]=new Vector2(ux,uy);	
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
						_allT[tp++]=_lattice[i*ymul+j].pP;	_allT[tp++]=_lattice[(i+1)*ymul+j].pP;	_allT[tp++]=_lattice[i*ymul+j+1].pP;
						_allT[tp++]=_lattice[i*ymul+j+1].pP;_allT[tp++]=_lattice[(i+1)*ymul+j].pP;	_allT[tp++]=_lattice[(i+1)*ymul+j+1].pP;
					}
				}
    		}
			
			addt=(_dimZ-1)*_dimY*_dimX;
			for(i=0;i<_dimY-1;i++) {
				for(j=0;j<_dimX-1;j++) {
					_allT[tp++]=_lattice[i*ymul+j+addt].pP;	 _allT[tp++]=_lattice[i*ymul+j+1+addt].pP;		_allT[tp++]=_lattice[(i+1)*ymul+j+addt].pP;
					_allT[tp++]=_lattice[i*ymul+j+1+addt].pP;_allT[tp++]=_lattice[(i+1)*ymul+j+1+addt].pP;	_allT[tp++]=_lattice[(i+1)*ymul+j+addt].pP;
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
							_allT[tp++]=_lattice[i*zmul+j].pP;	_allT[tp++]=_lattice[i*zmul+j+1].pP;_allT[tp++]=_lattice[(i+1)*zmul+j].pP;
							_allT[tp++]=_lattice[i*zmul+j+1].pP;_allT[tp++]=_lattice[(i+1)*zmul+j+1].pP;_allT[tp++]=_lattice[(i+1)*zmul+j].pP;
						}
					}
				}
				addt=(_dimX)*(_dimY-1);
				for(i=0;i<_dimZ-1;i++) {
					for(j=0;j<_dimX-1;j++) {
						_allT[tp++]=_lattice[i*zmul+j+addt].pP;	 _allT[tp++]=_lattice[(i+1)*zmul+j+addt].pP;_allT[tp++]=_lattice[i*zmul+j+1+addt].pP;
						_allT[tp++]=_lattice[i*zmul+j+1+addt].pP;_allT[tp++]=_lattice[(i+1)*zmul+j+addt].pP;_allT[tp++]=_lattice[(i+1)*zmul+j+1+addt].pP;
					}
				}
			} 
			if(_dimY>1){
				//y&x
				if(!oneSided){
					for(i=0;i<_dimZ-1;i++) {
						for(j=0;j<_dimY-1;j++) {
							_allT[tp++]=_lattice[i*zmul+j*ymul].pP;		_allT[tp++]=_lattice[(i+1)*zmul+j*ymul].pP;_allT[tp++]=_lattice[i*zmul+(j+1)*ymul].pP;
							_allT[tp++]=_lattice[i*zmul+(j+1)*ymul].pP;	_allT[tp++]=_lattice[(i+1)*zmul+j*ymul].pP;_allT[tp++]=_lattice[(i+1)*zmul+(j+1)*ymul].pP;
						}
					}
				}
				addt=(_dimX-1);
				for(i=0;i<_dimZ-1;i++) {
					for(j=0;j<_dimY-1;j++) {
						_allT[tp++]=_lattice[i*zmul+j*ymul+addt].pP;_allT[tp++]=_lattice[i*zmul+(j+1)*ymul+addt].pP;	_allT[tp++]=_lattice[(i+1)*zmul+j*ymul+addt].pP;
						
						_allT[tp++]=_lattice[i*zmul+(j+1)*ymul+addt].pP;_allT[tp++]=_lattice[(i+1)*zmul+(j+1)*ymul+addt].pP;_allT[tp++]=_lattice[(i+1)*zmul+j*ymul+addt].pP;
					}
				}
			}
		}

        //Assign to unity mesh
    	mesh.vertices=_allP;
    	mesh.uv=_allUV;
    	mesh.triangles=_allT;
    	
    	
    	
	}
	
	/*Unity Specific positioning of Collision Objects
	 *Currently only spheres and yPlane. Will inverse transform their positions,
	 *it is set in such a way that Spheres may be moved/removed/added every frame.
	 */
	void preUpdate()
	{
		
    	GameObject[] spheres =  GameObject.FindGameObjectsWithTag ("Softbody");    	
    	s=new float[spheres.Length][];
    	for(int i=0;i<spheres.Length;i++)
    	{
    		s[i]=new float[4];    		
    		Vector3 ip=transform.InverseTransformPoint(spheres[i].transform.position);
    		s[i][0]=ip.x;
    		s[i][1]=ip.y;
    		s[i][2]=ip.z;    		
    		s[i][3]=spheres[i].transform.lossyScale.x*.5f+.01f;
    	}
    	
    	_yFix=yPlane-transform.position.y;
	}
	
	/*Unity and Sample Specific
	 *Code to update the displayed mesh with the calculated lattice
	 */
	private void renderMesh()
	{
		int i;	
									
		Mesh mesh=((MeshFilter) GetComponent("MeshFilter")).mesh;
		
		for(i=0;i<_allP.Length;i++)
		{
			_allP[i]=_lattice[_pP[i]].position;
		}
				
	    mesh.vertices = _allP;//fv ;
	    
	    mesh.RecalculateNormals();	
	    
	    mesh.RecalculateBounds();
	//  if a message is desired to be displayed once every second
	/*	if(lt+1<Time.time) {
			lt=Time.time;
			String str="T:"+_allT.Length/3+" V:"+_allP.Length+" L:"+_lattice[0].force+" FPS:"+(int)(1f/Time.deltaTime);			
			Debug.Log(Time.time+" - "+str);
		}*/
		
	}
	

    //Class for one mass point in the soft body
    public class cMass
    {
    	//One Connection to another mass
    	public class cmConnect
    	{
    		//The original and connected to masses
    		//This is done so the connected to mass can be updated with the inverse of the
    		//calculated force without recalculating it when it is revisited
    		public cMass cm;
    		public cMass alter;
    		
    		//Rest length of connection
    		public float length;
    		
    		//Springyness
    		public float k;
    		
    		//Two way? (For jello/pillow/folds in cloth)
    		public bool twoWay;    		
    		
    		//axis id of hinge
    		public int axis;
    		
    		//alternate (reversed) axis of hinge
    		public int alterAxis;
    		
    		//Last frame connection calculated at (if it is calculated from the reverse connection, then don't recalc)
    		public int lptr;    		
    		
    		//Calculates the connection variables
    		public cmConnect(cMass cm,int axis,cMass alter)
    		{
    			this.lptr=0;
    			this.cm=cm;
    			this.alter=alter;
    			
    			//Use Softbody defaults at first    			
    			k=cm.myBody.defaultK;
    			twoWay=cm.myBody.dTwoWay;    			
    			this.axis=axis;
    			
    			//Calculate the rest lengths in x/y/z to the next mass
    			float dx=(cm.myBody._dimX>1 ? cm.myBody._sX/((float)cm.myBody._dimX-1f) : 0);
    			float dy=(cm.myBody._dimY>1 ? cm.myBody._sY/((float)cm.myBody._dimY-1f) : 0);
    			float dz=(cm.myBody._dimZ>1 ? cm.myBody._sZ/((float)cm.myBody._dimZ-1f) : 0);
    			
    			//The alternate axis is always the total amount of axis-this axis
    			this.alterAxis=(SoftBody.maxCon-1)-axis;    			
    			
    			//Calculate the length, taking the square of the steps in x,y,z in that axis
    			int[,] aa=SoftBody.allAxis;
    			this.length=Mathf.Sqrt( dx*dx*(aa[axis,0]*aa[axis,0]) + dy*dy*(aa[axis,1]*aa[axis,1]) + dz*dz*(aa[axis,2]*aa[axis,2]));
    						
    		}
    		
    		//Calculate the force that is exerted by this spring on both the connected masses
    		//Uses a general Newtonian formula f=kd
    		public void addForce()
    		{
    			this.lptr=cm.myBody.fptr;
    			Vector3 dir=alter.position-cm.position;
    			float tLength=dir.magnitude;
    			
    			float tDelta=(tLength-this.length); 
    			 
    			//Only calculate if the spring is a two way, or stretched beyond rest length
    			if(tDelta>0 || twoWay) { 
    				
	    			float forcem=(k*(tDelta));	    			
	    			Vector3 nForce=dir*forcem;
	    			
	    			cm.force=cm.force+nForce;
	    			cmConnect acmC=alter._cons[alterAxis];
	    			
	    			//add inverse to alternate mass if it hasn't been calculated through this axis
	    			//if(acmC==null) {Debug.Log(axis+"&"+alterAxis+" - "+cm.px+","+cm.py+","+cm.pz);} Debug purposes
	    			if( acmC.lptr<this.lptr) {
	    				acmC.lptr=this.lptr;
		    			alter.force=alter.force-nForce;
	    			}
    			} else{
    				//set inverse as calculated, even though nothing was added
    				cmConnect acmC=alter._cons[alterAxis];
	    			if( acmC.lptr<this.lptr) {
	    				acmC.lptr=this.lptr;
	    			}
    			}

    		}    		
    		
    	}
    	
    	//SoftBody this mass belongs to
    	SoftBody myBody;
    	
    	//discrete coordinates in the lattice
    	public int px;
    	public int py;
    	public int pz;
    	

    	//Newtonian variables in threespace
    	public Vector3 force;
    	public Vector3 velocity;
    	public Vector3 position;
    	
    	//Pointer into the displayed triangle list
    	public int pP;
    	
    	//Is it fixed? (unmoving)
    	public bool fix;
    	   	
    	//All the connections to other masses (Refer to SoftBody.allAxis to see their relation)
    	public cmConnect[] _cons;    	
    	
    	
    	//Initialize the mass, its starting position and forces, and allocate for Connections
    	public cMass(SoftBody myBody, int px,int py, int pz)
    	{
    		float vx=0;float vy=0;float vz=0;
    		

    		if(myBody._dimX>1) { vx=-(myBody._sX*.5f)+((myBody._sX/((float)myBody._dimX-1f))*((float)px)); }
    		if(myBody._dimY>1) { vy=-(myBody._sY*.5f)+((myBody._sY/((float)myBody._dimY-1f))*((float)py)); }
    		if(myBody._dimZ>1) { vz=-(myBody._sZ*.5f)+((myBody._sZ/((float)myBody._dimZ-1f))*((float)pz)); }  
    		  		    		
    		force=new Vector3(0,0,0);
    		velocity=new Vector3(0,0,0);
    		position=new Vector3(vx,vy,vz);
    		
    		this.px=px; 
    		this.py=py; 
    		this.pz=pz;
    		
    		this.myBody=myBody;
    		
    		
    		this.fix=false;
    		
    		_cons=new cmConnect[SoftBody.maxCon]; 		   		
    		
    	}
    	
    	//Calculate the forces across all the connections to this object, but only connections that haven't already
    	//been calculated for (due to inverse axis)
    	public void calcCons()
    	{
    		for(int j=0;j<SoftBody.maxCon;j++)
			{
				cmConnect c=this._cons[j];
				if(c!=null && c.lptr<myBody.fptr) {
					c.addForce();
					
				}					
			}
			
    	}
    	
    	
    	//Once forces have been calculated, add forces to velocity, cap it, and add velocity to position, then check collisions
    	public void doMovement()
    	{
    		
    		if(!this.fix) {
	    		float t=this.myBody.ftime;
	    		
                //If there are more masses, each one would weigh less so force would be more effective
				this.velocity+=t*this.force*this.myBody._mEach;
				
				//No mega numeric instability explosions!
				if(this.velocity.magnitude>myBody.velocityCap) { this.velocity=this.velocity.normalized*myBody.velocityCap; }
		
    			this.position+=t*this.velocity;
    			
    			//check collisions, which will move the mass if collided
    			doCollision();
    		}
    			
    			
    	}
    	
    	//Collision interaction
    	public void doCollision()
    	{
    		
    		float[][] s=myBody.s;
    		
    		
   			//zfloor check, takes in account friction and floor bouncyness	
   			if(this.position.y<myBody._yFix) {
   				this.position.y=myBody._yFix;
	  			this.velocity.x=this.velocity.x*myBody.dFriction;
	  			this.velocity.z=this.velocity.z*myBody.dFriction;
	  			this.velocity.y=-this.velocity.y*myBody.dCaucho;
	  		}
  			//Sphere collisions, inexact but they work
		    for(int i=0;i<s.Length;i++)
		    {
		    	
			    Vector3 c=new Vector3(s[i][0],s[i][1],s[i][2]);
			    //Vector3 op=this.position;
			    Vector3 d=this.position-c;
			    float r=s[i][3];
			    float dst=d.magnitude;
			    if(dst<r)
			    {
				    Vector3 u=d.normalized;	
				    
				    float dot=Vector3.Dot(this.velocity.normalized,u);
				    float fric=(myBody.dFriction*this.force.magnitude*dot);
				    this.velocity=((1f-dot)*this.velocity*fric)+(dot*d.normalized*this.velocity.magnitude)*myBody.dCaucho;	
				    this.position=c+(u*r+this.velocity*(r-dst));
			    }
  			}
			    
			    
    		
    	}
    	
    	//Initiate all the connections for this mass
    	public int makeLattice()
    	{
    		cMass tm;
    		int count=0;
    		int i;
          	int[,] aa=SoftBody.allAxis;            
            for(i=0;i<SoftBody.maxCon;i++) 
            {

            	tm=myBody.findMass(px+aa[i,0],py+aa[i,1],pz+aa[i,2]);
            	if(tm!=null) {_cons[i]=new cmConnect(this,i,tm); count++; } else {_cons[i]=null;}
            }
            
            return count;
    			
    		
    	}
    	
    	
    }
	

	//This is the lattice of masses that make up the Soft Body
	private cMass[] _lattice;
	
	

	//Internal: Dimensions of connections
	protected int _dimX,_dimY,_dimZ;
	
    //frame pointer, to see if a connection has already been calculated for
	protected int fptr;
	
	//Distance in time to calculate for
	protected float ftime;
	
	//Internal Distance to ground, calculated every frame from transform 
	protected float _yFix;
	
	//Default Springyness damper/amplifier against hard surfaces
	protected float dCaucho=.3f;
	
	//Default Friction against hard surfaces
	protected float dFriction=.2f;
	
	//Internal: The size in X and Y and Z
	protected float _sX,_sY,_sZ;
	
	
	//To use to multiply force by (amount of masses/total mass)
	protected float _mEach;
	
	//Spheres that will be calculated for
	protected float[][] s;
	
	//All the points, UVs, Triangles, and reverse lookup of points
	protected Vector3[] _allP;
	protected Vector2[] _allUV;
	protected int[] _allT;
	protected int[] _pP;	

	//Give coordinates into lattice, find a mass
	protected cMass findMass(int x,int y,int z)
	{
		if(x<0 || y<0 || z<0 || x>=_dimX || y>=_dimY || z>=_dimZ) { return null; }
		else {return _lattice[z*_dimX*_dimY + y*_dimX + x];}
		
	}
	
	//Every frame, clear the forces, and add gravity otherwise things may become a little unstable...
	protected void clearForces()
	{
		for(int i=0;i<_lattice.Length;i++)
		{
			//For Windy looking thing
			//_lattice[i].force.x=.0006f+Mathf.Sin(Time.time)*.0002f;_lattice[i].force.y=-gravity;_lattice[i].force.z=Mathf.Sin(Time.time+_lattice[i].position.x*2)*.0008f;
			_lattice[i].force.x=0f;_lattice[i].force.y=-gravity;_lattice[i].force.z=0f;

		}
	}
	
	//Algorithm specific per-frame calculations
	void perFrame()
	{		
		fptr++;
		clearForces();
		for(int i=0;i<_lattice.Length;i++)
		{
			cMass ptr=_lattice[i];
			ptr.calcCons(); //calculate addition of all the forces
			ptr.doMovement(); //move according to time and calculated force, includes collision detection			
		}
	}
	
	
	//Initialize algorithm, including the mass lattice
	void initialize() 
	{
		int jx,jy,jz;
		//float vx,vy,vz;
		int i;
		int massCount=_dimX*_dimY*_dimZ;
        
        //How much to multiply forces by
        _mEach=(_dimX*_dimY*_dimZ)/mass;
        
        //Initialize frame counter to 0
        fptr=0;
        
		_lattice=new cMass[massCount];
		i=0;
		for(jz=0;jz<_dimZ;jz++)
		{
			for(jy=0;jy<_dimY;jy++)
			{
				for(jx=0;jx<_dimX;jx++)
				{
					_lattice[i++]=new cMass(this,jx,jy,jz);
					
				}
			}			
		}
		
		i=0; while(i<massCount) { _lattice[i++].makeLattice(); }
		
		
		
	}
	
	//The connection axis, each row represents the relative distance
	//in x,y,z units of the mass connected via the connection. The inverse
	//axis can be calculated by maxCon-index.
	static public int maxCon=74;

    static protected int[,] allAxis=new int[,]
	{{0,+1,-2},
	{0,+1,+2},
	{0,+2,-2},
	{0,+2,+2},
	{0,+2,-1},
	{0,+2,+1},
	
	{+1,0,-2},
	{+1,0,+2},
	{+2,0,-2},
	{+2,0,+2},
	{+2,0,-1},
	{+2,0,+1},
	
	{-2,-2,-2},
	{-2,+2,-2},
	{+2,-2,-2},
	{+2,+2,-2},
	
	{-1,-1,-1},
	{-1,+1,-1},
	{+1,-1,-1},
	{+1,+1,-1},
	
	{-1,0,-1},
	{+1,0,-1},
	{0,-1,-1},
	{0,+1,-1},
	
	{-2,-2,0},
	{+2,-2,0},
	{+1,-2,0},
	{-1,-2,0},
	
	{-2,-1,0},
	{-2,+1,0},  
	    		
	{0,-2,0},
	{-2,0,0},
		    	
	{-1,-1,0},
	{+1,-1,0},
		   		
	{0,0,-1},
	{0,-1,0},
	{-1,0,0},
	{+1,0,0},
	{0,+1,0},
	{0,0,+1},
	   		    		
	{-1,+1,0},
	{+1,+1,0},
			
	{+2,0,0},
	{0,+2,0},
	 	    		
	{+2,-1,0},
	{+2,+1,0},
	
	{+1,+2,0},
	{-1,+2,0},
	
	{-2,+2,0},
	{+2,+2,0},
	
	{0,-1,+1},
	{0,+1,+1},
	{-1,0,+1},
	{+1,0,+1},
	
	{-1,-1,+1},
	{-1,+1,+1},
	{+1,-1,+1},
	{+1,+1,+1},
	
	{-2,-2,+2},
	{-2,+2,+2},
	{+2,-2,+2},
	{+2,+2,+2},
	
	{-2,0,-1},
	{-2,0,+1},
	{-2,0,-2},
	{-2,0,+2},
	{-1,0,-2},
	{-1,0,+2},
	
	{0,-2,-1},
	{0,-2,+1},
	{0,-2,-2},
	{0,-2,+2},
	{0,-1,-2},
	{0,-1,+2}};

	
	
	
}
