# Makefile for RTGP project - MacOS environment
# author: Davide Gadia - readapted by Vincenzo Lombardo
# Real-Time Graphics Programming - a.a. 2021/2022
# Master degree in Computer Science
# Universita' degli Studi di Milano

# name of the file
FILENAME = 3d-fluid-simulation

# Xcode compiler
CXX = clang++

# Include path
IDIR = ../include

# Libraries path
LDIR = ../libs/mac-arm64-all

# MacOS frameworks
MACFW = -framework OpenGL -framework IOKit -framework Cocoa -framework CoreVideo

# compiler flags:
CXXFLAGS  = -g -O0 -x c++ -mmacosx-version-min=11.1 -Wall -Wno-invalid-offsetof -std=c++11 -I$(IDIR)

# linker flags:
LDFLAGS = -L$(LDIR) -lglfw3 -lassimp -lz -lminizip -lkubazip -lbz2d -lIrrlicht -lpoly2tri -lpolyclipping -lturbojpeg -lpng16d -lpugixml $(MACFW)

SOURCES = ../include/glad/glad.c fluid-sim.cpp main.cpp


TARGET = $(FILENAME).out

.PHONY : all
all:
	cd src && $(CXX) $(CXXFLAGS) $(LDFLAGS) $(SOURCES) -o ../$(TARGET)

.PHONY : clean
clean :
	-rm $(TARGET)
	-rm -R $(TARGET).dSYM
