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
	${OBJECTDIR}/dbsingnkey.o \
	${OBJECTDIR}/pufasign.o


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
LDLIBSOPTIONS=-L/usr/local/lib -L/usr/local/ocilib/lib -L../lib/base/kms -L${HOME}/api2016/lib64 -locilib -lhiredis -lzlog -lunionAPI2 ../../lib/base/kms/dist/Debug/GNU-Linux/libkms.a ../../lib/base/tools/dist/Debug/GNU-Linux/libtools.a ../../lib/trans/common/dist/Debug/GNU-Linux/libcommon.a

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${HOME}/bin/pufasign

${HOME}/bin/pufasign: ../../lib/base/kms/dist/Debug/GNU-Linux/libkms.a

${HOME}/bin/pufasign: ../../lib/base/tools/dist/Debug/GNU-Linux/libtools.a

${HOME}/bin/pufasign: ../../lib/trans/common/dist/Debug/GNU-Linux/libcommon.a

${HOME}/bin/pufasign: ${OBJECTFILES}
	${MKDIR} -p ${HOME}/bin
	${LINK.c} -o ${HOME}/bin/pufasign ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/dbsingnkey.o: dbsingnkey.c nbproject/Makefile-${CND_CONF}.mk
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Werror -I/usr/local/include/hiredis -I/usr/local/include -I/usr/local/ocilib/include -I../../lib/base/tools -I../../lib/trans/common -I../lib/base/kms -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/dbsingnkey.o dbsingnkey.c

${OBJECTDIR}/pufasign.o: pufasign.c nbproject/Makefile-${CND_CONF}.mk
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Werror -I/usr/local/include/hiredis -I/usr/local/include -I/usr/local/ocilib/include -I../../lib/base/tools -I../../lib/trans/common -I../lib/base/kms -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/pufasign.o pufasign.c

# Subprojects
.build-subprojects:
	cd ../../lib/base/kms && ${MAKE}  -f Makefile CONF=Debug
	cd ../../lib/base/tools && ${MAKE}  -f Makefile CONF=Debug
	cd ../../lib/trans/common && ${MAKE}  -f Makefile CONF=Debug

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}

# Subprojects
.clean-subprojects:
	cd ../../lib/base/kms && ${MAKE}  -f Makefile CONF=Debug clean
	cd ../../lib/base/tools && ${MAKE}  -f Makefile CONF=Debug clean
	cd ../../lib/trans/common && ${MAKE}  -f Makefile CONF=Debug clean

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
