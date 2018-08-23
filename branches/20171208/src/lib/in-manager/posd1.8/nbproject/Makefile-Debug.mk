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
	${OBJECTDIR}/dbop.o \
	${OBJECTDIR}/iso.o \
	${OBJECTDIR}/packmsg.o \
	${OBJECTDIR}/service.o


# C Compiler Flags
CFLAGS=-Wall

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-L${HOME}/api2016/lib64 -L/usr/local/ocilib/lib -L/usr/local/lib -L../../base/kms -lhiredis -locilib -lzlog -lunionAPI2 ../../base/tools/dist/Debug/GNU-Linux/libtools.a ../../base/hsm/dist/Debug/GNU-Linux/libhsm.a ../../trans/common/dist/Debug/GNU-Linux/libcommon.a ../../base/kms/dist/Debug/GNU-Linux/libkms.a

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${HOME}/lib/libposd_1.8.${CND_DLIB_EXT}

${HOME}/lib/libposd_1.8.${CND_DLIB_EXT}: ../../base/tools/dist/Debug/GNU-Linux/libtools.a

${HOME}/lib/libposd_1.8.${CND_DLIB_EXT}: ../../base/hsm/dist/Debug/GNU-Linux/libhsm.a

${HOME}/lib/libposd_1.8.${CND_DLIB_EXT}: ../../trans/common/dist/Debug/GNU-Linux/libcommon.a

${HOME}/lib/libposd_1.8.${CND_DLIB_EXT}: ../../base/kms/dist/Debug/GNU-Linux/libkms.a

${HOME}/lib/libposd_1.8.${CND_DLIB_EXT}: ${OBJECTFILES}
	${MKDIR} -p ${HOME}/lib
	${LINK.c} -o ${HOME}/lib/libposd_1.8.${CND_DLIB_EXT} ${OBJECTFILES} ${LDLIBSOPTIONS} -shared -fPIC

${OBJECTDIR}/dbop.o: dbop.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Werror -I/usr/local/include/hiredis -I/usr/local/include -I/usr/local/ocilib/include -I../../base/hsm -I../../base/tools -I../../trans/common -I../../../platform/manager -I../../base/kms -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/dbop.o dbop.c

${OBJECTDIR}/iso.o: iso.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Werror -I/usr/local/include/hiredis -I/usr/local/include -I/usr/local/ocilib/include -I../../base/hsm -I../../base/tools -I../../trans/common -I../../../platform/manager -I../../base/kms -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/iso.o iso.c

${OBJECTDIR}/packmsg.o: packmsg.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Werror -I/usr/local/include/hiredis -I/usr/local/include -I/usr/local/ocilib/include -I../../base/hsm -I../../base/tools -I../../trans/common -I../../../platform/manager -I../../base/kms -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/packmsg.o packmsg.c

${OBJECTDIR}/service.o: service.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Werror -I/usr/local/include/hiredis -I/usr/local/include -I/usr/local/ocilib/include -I../../base/hsm -I../../base/tools -I../../trans/common -I../../../platform/manager -I../../base/kms -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/service.o service.c

# Subprojects
.build-subprojects:
	cd ../../base/tools && ${MAKE}  -f Makefile CONF=Debug
	cd ../../base/hsm && ${MAKE}  -f Makefile CONF=Debug
	cd ../../trans/common && ${MAKE}  -f Makefile CONF=Debug
	cd ../../base/kms && ${MAKE}  -f Makefile CONF=Debug

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}

# Subprojects
.clean-subprojects:
	cd ../../base/tools && ${MAKE}  -f Makefile CONF=Debug clean
	cd ../../base/hsm && ${MAKE}  -f Makefile CONF=Debug clean
	cd ../../trans/common && ${MAKE}  -f Makefile CONF=Debug clean
	cd ../../base/kms && ${MAKE}  -f Makefile CONF=Debug clean

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
