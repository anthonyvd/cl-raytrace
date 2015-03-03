INCLUDE_PATH = -I/opt/AMDAPP/include

LIB_PATH = -L/usr/local/lib -L/opt/AMDAPP/lib/x86_64
LIBS = -lMagick++ -lMagickCore -lOpenCL

FLAGS = --std=c++11 -g -Wfatal-errors

MAGICK_FLAGS = `Magick++-config --cxxflags --cppflags`
MAGICK_LD_FLAGS = `Magick++-config --ldflags --libs`

all: main.cpp
	g++ $(MAGICK_FLAGS) -o raytrace $(INCLUDE_PATH) $(LIB_PATH) $(FLAGS) main.cpp $(MAGICK_LD_FLAGS) $(LIBS)
