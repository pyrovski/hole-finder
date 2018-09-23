CXXFLAGS=-O2 -std=c++11 -Wall

all: zeros

zeros: zeros.cc

install: zeros
	install -t ${HOME}/local/bin zeros
