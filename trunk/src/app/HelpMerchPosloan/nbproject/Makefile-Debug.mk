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
	${OBJECTDIR}/helpmerchposloan.o \
	${OBJECTDIR}/merchpostotalpro.o


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
LDLIBSOPTIONS=-L/usr/local/lib -L/usr/local/ocilib/lib -locilib -lhiredis -lzlog ../../lib/base/tools/dist/Debug/GNU-Linux/libtools.a ../../lib/trans/common/dist/Debug/GNU-Linux/libcommon.a

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${HOME}/bin/helpmerchposloan

${HOME}/bin/helpmerchposloan: ../../lib/base/tools/dist/Debug/GNU-Linux/libtools.a

${HOME}/bin/helpmerchposloan: ../../lib/trans/common/dist/Debug/GNU-Linux/libcommon.a

${HOME}/bin/helpmerchposloan: ${OBJECTFILES}
	${MKDIR} -p ${HOME}/bin
	${LINK.c} -o ${HOME}/bin/helpmerchposloan ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/helpmerchposloan.o: helpmerchposloan.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -I/usr/local/ocilib/include -I/usr/local/include/hiredis -I/usr/include -I/usr/local/include -I../../lib/base/tools -I../../lib/trans/common -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/helpmerchposloan.o helpmerchposloan.c

${OBJECTDIR}/merchpostotalpro.o: merchpostotalpro.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -I/usr/local/ocilib/include -I/usr/local/include/hiredis -I/usr/include -I/usr/local/include -I../../lib/base/tools -I../../lib/trans/common -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/merchpostotalpro.o merchpostotalpro.c

# Subprojects
.build-subprojects:
	cd ../../lib/base/tools && ${MAKE}  -f Makefile CONF=Debug
	cd ../../lib/trans/common && ${MAKE}  -f Makefile CONF=Debug

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}

# Subprojects
.clean-subprojects:
	cd ../../lib/base/tools && ${MAKE}  -f Makefile CONF=Debug clean
	cd ../../lib/trans/common && ${MAKE}  -f Makefile CONF=Debug clean

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
