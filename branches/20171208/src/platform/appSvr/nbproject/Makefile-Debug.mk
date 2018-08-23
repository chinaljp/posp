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
	${OBJECTDIR}/appsvr.o \
	${OBJECTDIR}/dylib.o \
	${OBJECTDIR}/message.o \
	${OBJECTDIR}/param.o


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
LDLIBSOPTIONS=-L/usr/local/ocilib/lib -L/usr/local/lib -L${HOME}/lib -lhiredis -locilib -lm -lzlog -ldl ../../lib/base/tools/dist/Debug/GNU-Linux/libtools.a

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${HOME}/bin/appsvr

${HOME}/bin/appsvr: ../../lib/base/tools/dist/Debug/GNU-Linux/libtools.a

${HOME}/bin/appsvr: ${OBJECTFILES}
	${MKDIR} -p ${HOME}/bin
	${LINK.c} -o ${HOME}/bin/appsvr ${OBJECTFILES} ${LDLIBSOPTIONS} -Wl,--export-dynamic

${OBJECTDIR}/appsvr.o: appsvr.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Werror -I/usr/local/ocilib/include -I/usr/local/include/hiredis -I/usr/local/include -I../../lib/base/tools -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/appsvr.o appsvr.c

${OBJECTDIR}/dylib.o: dylib.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Werror -I/usr/local/ocilib/include -I/usr/local/include/hiredis -I/usr/local/include -I../../lib/base/tools -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/dylib.o dylib.c

${OBJECTDIR}/message.o: message.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Werror -I/usr/local/ocilib/include -I/usr/local/include/hiredis -I/usr/local/include -I../../lib/base/tools -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/message.o message.c

${OBJECTDIR}/param.o: param.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Werror -I/usr/local/ocilib/include -I/usr/local/include/hiredis -I/usr/local/include -I../../lib/base/tools -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/param.o param.c

# Subprojects
.build-subprojects:
	cd ../../lib/base/tools && ${MAKE}  -f Makefile CONF=Debug

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}

# Subprojects
.clean-subprojects:
	cd ../../lib/base/tools && ${MAKE}  -f Makefile CONF=Debug clean

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
