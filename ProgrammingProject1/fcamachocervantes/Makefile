#
# Makefile
# Computer Networking Programing Assignments
#
#  Created by Phillip Romig on 4/3/12.
#  Copyright 2012 Colorado School of Mines. All rights reserved.
#

CXX = g++
LD = g++
CXXFLAGS = -g -std=c++11
LDFLAGS = -g 

#
# You should be able to add object files here without changing anything else
#
TARGET = echo_s
OBJ_FILES = ${TARGET}.o
INC_FILES = ${TARGET}.h


${TARGET}: ${OBJ_FILES}
	${LD} ${LDFLAGS} ${OBJ_FILES} -o $@

%.o : %.cc ${INC_FILES}
	${CXX} -c ${CXXFLAGS} -o $@ $<

#
# Please remember not to submit objects or binarys.
#
clean:
	rm -f core ${TARGET} ${OBJ_FILES}

#
# This might work to create the submission tarball in the formal I asked for.
#
submit:
	rm -f core project1 ${OBJ_FILES}
	mkdir `whoami`
	cp Makefile README.txt *.h *.cc `whoami`
	tar zcf `whoami`.tgz `whoami`
