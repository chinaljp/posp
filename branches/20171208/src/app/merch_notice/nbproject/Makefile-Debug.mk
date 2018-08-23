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
	${OBJECTDIR}/message.o \
	${OBJECTDIR}/notice.o \
	${OBJECTDIR}/param.o


# C Compiler Flags
CFLAGS=-DOCI_CHARSET_ANSI

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-L/usr/local/ocilib/lib -L/usr/local/lib -L/home/posp/lib -L/u01/app/oracle/product/11.2.0/dbhome_1/lib -L../../lib/base/kms -L${HOME}/api2016/lib64 -locilib -lhiredis -lzlog -lunionAPI2 ../../lib/base/kms/dist/Debug/GNU-Linux/libkms.a ../../lib/base/tools/dist/Debug/GNU-Linux/libtools.a

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${HOME}/bin/merch_notice

${HOME}/bin/merch_notice: ../../lib/base/kms/dist/Debug/GNU-Linux/libkms.a

${HOME}/bin/merch_notice: ../../lib/base/tools/dist/Debug/GNU-Linux/libtools.a

${HOME}/bin/merch_notice: ${OBJECTFILES}
	${MKDIR} -p ${HOME}/bin
	${LINK.c} -o ${HOME}/bin/merch_notice ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/message.o: message.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -w -I/usr/local/ocilib/include -I/usr/local/include/hiredis -I/usr/local/include -I../../lib/base/tools -I/usr/include -I../../lib/base/kms -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/message.o message.c

${OBJECTDIR}/notice.o: notice.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -w -I/usr/local/ocilib/include -I/usr/local/include/hiredis -I/usr/local/include -I../../lib/base/tools -I/usr/include -I../../lib/base/kms -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/notice.o notice.c

${OBJECTDIR}/param.o: param.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -w -I/usr/local/ocilib/include -I/usr/local/include/hiredis -I/usr/local/include -I../../lib/base/tools -I/usr/include -I../../lib/base/kms -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/param.o param.c

# Subprojects
.build-subprojects:
	cd ../../lib/base/kms && ${MAKE}  -f Makefile CONF=Debug
	cd ../../lib/base/tools && ${MAKE}  -f Makefile CONF=Debug

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}

# Subprojects
.clean-subprojects:
	cd ../../lib/base/kms && ${MAKE}  -f Makefile CONF=Debug clean
	cd ../../lib/base/tools && ${MAKE}  -f Makefile CONF=Debug clean

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
