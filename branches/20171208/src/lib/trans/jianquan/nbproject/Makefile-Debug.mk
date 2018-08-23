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
	${OBJECTDIR}/auth.o \
	${OBJECTDIR}/chkagent.o \
	${OBJECTDIR}/chkmerch.o \
	${OBJECTDIR}/chkterm.o \
	${OBJECTDIR}/db_auth.o \
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
LDLIBSOPTIONS=-L../../base/kms -L/usr/local/lib -L/usr/local/ocilib/lib -L/home/post/lib/ -L${HOME}/api2016/lib64 -locilib -lunionAPI2 -lhiredis ../../base/tools/dist/Debug/GNU-Linux/libtools.a ../common/dist/Debug/GNU-Linux/libcommon.a ../../base/kms/dist/Debug/GNU-Linux/libkms.a

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${HOME}/lib/libjianquan.${CND_DLIB_EXT}

${HOME}/lib/libjianquan.${CND_DLIB_EXT}: ../../base/tools/dist/Debug/GNU-Linux/libtools.a

${HOME}/lib/libjianquan.${CND_DLIB_EXT}: ../common/dist/Debug/GNU-Linux/libcommon.a

${HOME}/lib/libjianquan.${CND_DLIB_EXT}: ../../base/kms/dist/Debug/GNU-Linux/libkms.a

${HOME}/lib/libjianquan.${CND_DLIB_EXT}: ${OBJECTFILES}
	${MKDIR} -p ${HOME}/lib
	${LINK.c} -o ${HOME}/lib/libjianquan.${CND_DLIB_EXT} ${OBJECTFILES} ${LDLIBSOPTIONS} -shared -fPIC

${OBJECTDIR}/auth.o: auth.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Werror -I/usr/local/include/hiredis -I/usr/local/include -I../../base/kms -I/usr/local/ocilib/include -I../common -I/usr/local/ocilib/include\; -I../../base/tools -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/auth.o auth.c

${OBJECTDIR}/chkagent.o: chkagent.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Werror -I/usr/local/include/hiredis -I/usr/local/include -I../../base/kms -I/usr/local/ocilib/include -I../common -I/usr/local/ocilib/include\; -I../../base/tools -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/chkagent.o chkagent.c

${OBJECTDIR}/chkmerch.o: chkmerch.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Werror -I/usr/local/include/hiredis -I/usr/local/include -I../../base/kms -I/usr/local/ocilib/include -I../common -I/usr/local/ocilib/include\; -I../../base/tools -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/chkmerch.o chkmerch.c

${OBJECTDIR}/chkterm.o: chkterm.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Werror -I/usr/local/include/hiredis -I/usr/local/include -I../../base/kms -I/usr/local/ocilib/include -I../common -I/usr/local/ocilib/include\; -I../../base/tools -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/chkterm.o chkterm.c

${OBJECTDIR}/db_auth.o: db_auth.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Werror -I/usr/local/include/hiredis -I/usr/local/include -I../../base/kms -I/usr/local/ocilib/include -I../common -I/usr/local/ocilib/include\; -I../../base/tools -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/db_auth.o db_auth.c

${OBJECTDIR}/service.o: service.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Werror -I/usr/local/include/hiredis -I/usr/local/include -I../../base/kms -I/usr/local/ocilib/include -I../common -I/usr/local/ocilib/include\; -I../../base/tools -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/service.o service.c

# Subprojects
.build-subprojects:
	cd ../../base/tools && ${MAKE}  -f Makefile CONF=Debug
	cd ../common && ${MAKE}  -f Makefile CONF=Debug
	cd ../../base/kms && ${MAKE}  -f Makefile CONF=Debug

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}

# Subprojects
.clean-subprojects:
	cd ../../base/tools && ${MAKE}  -f Makefile CONF=Debug clean
	cd ../common && ${MAKE}  -f Makefile CONF=Debug clean
	cd ../../base/kms && ${MAKE}  -f Makefile CONF=Debug clean

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
