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
	${OBJECTDIR}/outgate.o \
	${OBJECTDIR}/outgate_func.o \
	${OBJECTDIR}/outgate_proc.o \
	${OBJECTDIR}/param.o \
	${OBJECTDIR}/pkdump.o \
	${OBJECTDIR}/sndrcv.o


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
LDLIBSOPTIONS=-L/usr/local/lib -L/usr/local/ocilib/lib -L/u02/app/oracle/product/11.2.0/db_1/lib -lhiredis -lzlog ../../lib/base/tools/dist/Debug/GNU-Linux/libtools.a -locilib

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${HOME}/bin/outgate

${HOME}/bin/outgate: ../../lib/base/tools/dist/Debug/GNU-Linux/libtools.a

${HOME}/bin/outgate: ${OBJECTFILES}
	${MKDIR} -p ${HOME}/bin
	${LINK.c} -o ${HOME}/bin/outgate ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/outgate.o: outgate.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Werror -I/usr/local/include/hiredis -I/usr/local/include -I/usr/local/ocilib/include -I../../lib/base/tools -I/usr/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/outgate.o outgate.c

${OBJECTDIR}/outgate_func.o: outgate_func.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Werror -I/usr/local/include/hiredis -I/usr/local/include -I/usr/local/ocilib/include -I../../lib/base/tools -I/usr/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/outgate_func.o outgate_func.c

${OBJECTDIR}/outgate_proc.o: outgate_proc.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Werror -I/usr/local/include/hiredis -I/usr/local/include -I/usr/local/ocilib/include -I../../lib/base/tools -I/usr/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/outgate_proc.o outgate_proc.c

${OBJECTDIR}/param.o: param.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Werror -I/usr/local/include/hiredis -I/usr/local/include -I/usr/local/ocilib/include -I../../lib/base/tools -I/usr/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/param.o param.c

${OBJECTDIR}/pkdump.o: pkdump.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Werror -I/usr/local/include/hiredis -I/usr/local/include -I/usr/local/ocilib/include -I../../lib/base/tools -I/usr/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/pkdump.o pkdump.c

${OBJECTDIR}/sndrcv.o: sndrcv.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Werror -I/usr/local/include/hiredis -I/usr/local/include -I/usr/local/ocilib/include -I../../lib/base/tools -I/usr/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/sndrcv.o sndrcv.c

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
