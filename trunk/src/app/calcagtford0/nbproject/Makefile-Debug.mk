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
CND_PLATFORM=GNU-None
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
	${OBJECTDIR}/calcrate.o \
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
LDLIBSOPTIONS=-L/usr/local/lib -L/home/posp/lib -L/usr/local/ocilib/lib -L../../lib/trans/common -L../../lib/base/tools -locilib -lzlog -lhiredis ../../lib/trans/common/dist/Debug/GNU-None/libcommon.a ../../lib/base/tools/dist/Debug/GNU-None/libtools.a

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${HOME}/bin/calcagtford0

${HOME}/bin/calcagtford0: ../../lib/trans/common/dist/Debug/GNU-None/libcommon.a

${HOME}/bin/calcagtford0: ../../lib/base/tools/dist/Debug/GNU-None/libtools.a

${HOME}/bin/calcagtford0: ${OBJECTFILES}
	${MKDIR} -p ${HOME}/bin
	${LINK.c} -o ${HOME}/bin/calcagtford0 ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/calcrate.o: calcrate.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Werror -I/usr/local/include/hiredis -I/usr/local/include -I/usr/local/ocilib/include -I../../lib/base/tools -I../../lib/trans/common -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/calcrate.o calcrate.c

${OBJECTDIR}/message.o: message.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Werror -I/usr/local/include/hiredis -I/usr/local/include -I/usr/local/ocilib/include -I../../lib/base/tools -I../../lib/trans/common -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/message.o message.c

${OBJECTDIR}/param.o: param.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Werror -I/usr/local/include/hiredis -I/usr/local/include -I/usr/local/ocilib/include -I../../lib/base/tools -I../../lib/trans/common -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/param.o param.c

# Subprojects
.build-subprojects:
	cd ../../lib/trans/common && ${MAKE}  -f Makefile CONF=Debug
	cd ../../lib/base/tools && ${MAKE}  -f Makefile CONF=Debug

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}

# Subprojects
.clean-subprojects:
	cd ../../lib/trans/common && ${MAKE}  -f Makefile CONF=Debug clean
	cd ../../lib/base/tools && ${MAKE}  -f Makefile CONF=Debug clean

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
