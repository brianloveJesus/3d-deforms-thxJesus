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



float secs()
{
   return (((float)clock())/(float)CLOCKS_PER_SEC);
}



