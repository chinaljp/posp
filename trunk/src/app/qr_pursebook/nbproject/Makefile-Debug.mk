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
	${OBJECTDIR}/db_qrpurse.o \
	${OBJECTDIR}/fee.o \
	${OBJECTDIR}/param.o \
	${OBJECTDIR}/purbookpro.o \
	${OBJECTDIR}/qrpursebook.o


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
LDLIBSOPTIONS=-L/usr/local/lib -L/usr/local/ocilib/lib -locilib -lhiredis -lzlog ../../lib/trans/common/dist/Debug/GNU-Linux/libcommon.a ../../lib/base/tools/dist/Debug/GNU-Linux/libtools.a

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${HOME}/bin/qr_pursebook

${HOME}/bin/qr_pursebook: ../../lib/trans/common/dist/Debug/GNU-Linux/libcommon.a

${HOME}/bin/qr_pursebook: ../../lib/base/tools/dist/Debug/GNU-Linux/libtools.a

${HOME}/bin/qr_pursebook: ${OBJECTFILES}
	${MKDIR} -p ${HOME}/bin
	${LINK.c} -o ${HOME}/bin/qr_pursebook ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/db_qrpurse.o: db_qrpurse.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -I/usr/local/ocilib/include -I/usr/local/include/hiredis -I/usr/include -I/usr/local/include -I../../lib/base/tools -I../../lib/trans/common -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/db_qrpurse.o db_qrpurse.c

${OBJECTDIR}/fee.o: fee.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -I/usr/local/ocilib/include -I/usr/local/include/hiredis -I/usr/include -I/usr/local/include -I../../lib/base/tools -I../../lib/trans/common -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/fee.o fee.c

${OBJECTDIR}/param.o: param.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -I/usr/local/ocilib/include -I/usr/local/include/hiredis -I/usr/include -I/usr/local/include -I../../lib/base/tools -I../../lib/trans/common -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/param.o param.c

${OBJECTDIR}/purbookpro.o: purbookpro.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -I/usr/local/ocilib/include -I/usr/local/include/hiredis -I/usr/include -I/usr/local/include -I../../lib/base/tools -I../../lib/trans/common -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/purbookpro.o purbookpro.c

${OBJECTDIR}/qrpursebook.o: qrpursebook.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -I/usr/local/ocilib/include -I/usr/local/include/hiredis -I/usr/include -I/usr/local/include -I../../lib/base/tools -I../../lib/trans/common -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/qrpursebook.o qrpursebook.c

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
