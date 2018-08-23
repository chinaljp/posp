#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux
CND_DLIB_EXT=so
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/main.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-L/usr/local/ocilib/lib -L${HOME}/api2016/lib64 -L../../lib/base/kms -L../../lib/base/tools -L../../lib/base/hsm -L/usr/local/lib -lhiredis -lunionAPI2 -locilib -lzlog ../../lib/base/kms/dist/Debug/GNU-Linux/libkms.a ../../lib/base/hsm/dist/Debug/GNU-Linux/libhsm.a ../../lib/base/tools/dist/Debug/GNU-Linux/libtools.a

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${HOME}/bin/dec_show

${HOME}/bin/dec_show: ../../lib/base/kms/dist/Debug/GNU-Linux/libkms.a

${HOME}/bin/dec_show: ../../lib/base/hsm/dist/Debug/GNU-Linux/libhsm.a

${HOME}/bin/dec_show: ../../lib/base/tools/dist/Debug/GNU-Linux/libtools.a

${HOME}/bin/dec_show: ${OBJECTFILES}
	${MKDIR} -p ${HOME}/bin
	${LINK.c} -o ${HOME}/bin/dec_show ${OBJECTFILES} ${LDLIBSOPTIONS} -Wall

${OBJECTDIR}/main.o: main.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -I${HOME}/api2016/lib64 -Iusr/local/include -I/usr/local/ocilib/include -I../../lib/base/kms -I../../lib/base/hsm -I../../lib/base/tools -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main.o main.c

# Subprojects
.build-subprojects:
	cd ../../lib/base/kms && ${MAKE}  -f Makefile CONF=Debug
	cd ../../lib/base/hsm && ${MAKE}  -f Makefile CONF=Debug
	cd ../../lib/base/tools && ${MAKE}  -f Makefile CONF=Debug

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}

# Subprojects
.clean-subprojects:
	cd ../../lib/base/kms && ${MAKE}  -f Makefile CONF=Debug clean
	cd ../../lib/base/hsm && ${MAKE}  -f Makefile CONF=Debug clean
	cd ../../lib/base/tools && ${MAKE}  -f Makefile CONF=Debug clean

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
