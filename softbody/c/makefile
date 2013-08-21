#/**
# * SoftBody implementation for CPP v0.2.1-m (With MidPoint integration) 
# * (c) Brian R. Cowan, 2007 (http://www.briancowan.net/) 
# * Examples at http://www.briancowan.net/unity/fx/
# *
# * Code provided as-is. You agree by using this code that I am not liable for any damage
# * it could possibly cause to you, your machine, or anything else. And the code is not meant
# * to be used for any medical uses or to run nuclear reactors or robots or such and so. 
# */

#the following are for osx
#LIBPATH +=-L"/System/Library/Frameworks/OpenGL.framework/Libraries"
#FRAMEWORK = -framework OpenGL 
#FRAMEWORK += -framework GLUT

COMPILERFLAGS = -Wall -O2
CC= g++ 
CFLAGS = $(COMPILERFLAGS) 
#osx remove lglut
LIBRARIES = -lGL -lGLU -lglut -lm -lobjc -lstdc++ 

OBJECTS = Util.o main.o CMass.o CMConnect.o allAxis.o SoftBody.o 
All: SoftBody 

SoftBody: allAxis.o $(OBJECTS) 
	$(CC) $(FRAMEWORK) $(CFLAGS) -o $@ $(LIBPATH) $(OBJECTS) $(LIBRARIES) 	
	./SoftBody
