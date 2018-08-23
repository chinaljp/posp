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
	${OBJECTDIR}/chktrans.o \
	${OBJECTDIR}/dbop.o \
	${OBJECTDIR}/param.o \
	${OBJECTDIR}/proc.o


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
LDLIBSOPTIONS=-L/usr/local/ocilib/lib -L/usr/local/lib -L/home/posp/lib -L/u01/app/oracle/product/11.2.0/dbhome_1/lib -locilib -lhiredis -lzlog ../../lib/base/tools/dist/Debug/GNU-Linux/libtools.a

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${HOME}/bin/checktrans

${HOME}/bin/checktrans: ../../lib/base/tools/dist/Debug/GNU-Linux/libtools.a

${HOME}/bin/checktrans: ${OBJECTFILES}
	${MKDIR} -p ${HOME}/bin
	${LINK.c} -o ${HOME}/bin/checktrans ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/chktrans.o: chktrans.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -w -I/usr/local/include/hiredis -I/usr/local/include -I../../lib/base/tools -I/usr/local/ocilib/include -I/usr/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/chktrans.o chktrans.c

${OBJECTDIR}/dbop.o: dbop.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -w -I/usr/local/include/hiredis -I/usr/local/include -I../../lib/base/tools -I/usr/local/ocilib/include -I/usr/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/dbop.o dbop.c

${OBJECTDIR}/param.o: param.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -w -I/usr/local/include/hiredis -I/usr/local/include -I../../lib/base/tools -I/usr/local/ocilib/include -I/usr/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/param.o param.c

${OBJECTDIR}/proc.o: proc.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -w -I/usr/local/include/hiredis -I/usr/local/include -I../../lib/base/tools -I/usr/local/ocilib/include -I/usr/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/proc.o proc.c

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
