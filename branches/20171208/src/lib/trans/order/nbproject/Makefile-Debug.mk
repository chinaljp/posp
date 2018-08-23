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
	${OBJECTDIR}/chkagent.o \
	${OBJECTDIR}/chkmerch.o \
	${OBJECTDIR}/chkterm.o \
	${OBJECTDIR}/order.o \
	${OBJECTDIR}/service.o


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
LDLIBSOPTIONS=-L/usr/local/lib -L/usr/local/ocilib/lib ../common/dist/Debug/GNU-Linux/libcommon.a ../../base/tools/dist/Debug/GNU-Linux/libtools.a

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${HOME}/lib/liborder.${CND_DLIB_EXT}

${HOME}/lib/liborder.${CND_DLIB_EXT}: ../common/dist/Debug/GNU-Linux/libcommon.a

${HOME}/lib/liborder.${CND_DLIB_EXT}: ../../base/tools/dist/Debug/GNU-Linux/libtools.a

${HOME}/lib/liborder.${CND_DLIB_EXT}: ${OBJECTFILES}
	${MKDIR} -p ${HOME}/lib
	${LINK.c} -o ${HOME}/lib/liborder.${CND_DLIB_EXT} ${OBJECTFILES} ${LDLIBSOPTIONS} -shared -fPIC

${OBJECTDIR}/chkagent.o: chkagent.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -I/usr/local/include/hiredis -I/usr/local/include -I/usr/local/ocilib/include -I../common -I/usr/local/ocilib/include -I../../base/tools -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/chkagent.o chkagent.c

${OBJECTDIR}/chkmerch.o: chkmerch.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -I/usr/local/include/hiredis -I/usr/local/include -I/usr/local/ocilib/include -I../common -I/usr/local/ocilib/include -I../../base/tools -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/chkmerch.o chkmerch.c

${OBJECTDIR}/chkterm.o: chkterm.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -I/usr/local/include/hiredis -I/usr/local/include -I/usr/local/ocilib/include -I../common -I/usr/local/ocilib/include -I../../base/tools -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/chkterm.o chkterm.c

${OBJECTDIR}/order.o: order.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -I/usr/local/include/hiredis -I/usr/local/include -I/usr/local/ocilib/include -I../common -I/usr/local/ocilib/include -I../../base/tools -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/order.o order.c

${OBJECTDIR}/service.o: service.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -I/usr/local/include/hiredis -I/usr/local/include -I/usr/local/ocilib/include -I../common -I/usr/local/ocilib/include -I../../base/tools -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/service.o service.c

# Subprojects
.build-subprojects:
	cd ../common && ${MAKE}  -f Makefile CONF=Debug
	cd ../../base/tools && ${MAKE}  -f Makefile CONF=Debug

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}

# Subprojects
.clean-subprojects:
	cd ../common && ${MAKE}  -f Makefile CONF=Debug clean
	cd ../../base/tools && ${MAKE}  -f Makefile CONF=Debug clean

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
