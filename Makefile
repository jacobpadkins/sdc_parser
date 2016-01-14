#############################
## Makefile - Jacob Adkins ##
#############################

# compilation definitions
CXX = g++
CXXFLAGS = -Wall -std=c++0x

# makefile targets
all : o.o

o.o : Main_no_db.cpp 
	${CXX} $^ ${CXXFLAGS} -o $@

clean :
	\rm -f *.o *.txt *.exe

###### End of Makefile ######
