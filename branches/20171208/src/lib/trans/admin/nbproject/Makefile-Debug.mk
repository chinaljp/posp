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
	${OBJECTDIR}/ChkUser.o \
	${OBJECTDIR}/chkagent.o \
	${OBJECTDIR}/chkmerch.o \
	${OBJECTDIR}/chkterm.o \
	${OBJECTDIR}/cups_resetkey.o \
	${OBJECTDIR}/db_order.o \
	${OBJECTDIR}/db_resetkey.o \
	${OBJECTDIR}/down_param.o \
	${OBJECTDIR}/service.o \
	${OBJECTDIR}/status_upload.o \
	${OBJECTDIR}/term_init.o \
	${OBJECTDIR}/term_sign.o \
	${OBJECTDIR}/term_signature.o


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
LDLIBSOPTIONS=-L/usr/local/lib -L/usr/local/ocilib/lib -L/home/posp/lib -L${HOME}/api2016/lib64 -L../../base/kms -locilib -lhiredis -lzlog -lunionAPI2 ../../base/hsm/dist/Debug/GNU-Linux/libhsm.a ../../base/tools/dist/Debug/GNU-Linux/libtools.a ../common/dist/Debug/GNU-Linux/libcommon.a ../../base/kms/dist/Debug/GNU-Linux/libkms.a

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${HOME}/lib/libadmin.${CND_DLIB_EXT}

${HOME}/lib/libadmin.${CND_DLIB_EXT}: ../../base/hsm/dist/Debug/GNU-Linux/libhsm.a

${HOME}/lib/libadmin.${CND_DLIB_EXT}: ../../base/tools/dist/Debug/GNU-Linux/libtools.a

${HOME}/lib/libadmin.${CND_DLIB_EXT}: ../common/dist/Debug/GNU-Linux/libcommon.a

${HOME}/lib/libadmin.${CND_DLIB_EXT}: ../../base/kms/dist/Debug/GNU-Linux/libkms.a

${HOME}/lib/libadmin.${CND_DLIB_EXT}: ${OBJECTFILES}
	${MKDIR} -p ${HOME}/lib
	${LINK.c} -o ${HOME}/lib/libadmin.${CND_DLIB_EXT} ${OBJECTFILES} ${LDLIBSOPTIONS} -shared -fPIC

${OBJECTDIR}/ChkUser.o: ChkUser.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Werror -I/usr/local/include/hiredis -I/usr/local/include -I/usr/local/ocilib/include -I../common -I../../base/tools -I../../base/hsm -I../../base/kms -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/ChkUser.o ChkUser.c

${OBJECTDIR}/chkagent.o: chkagent.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Werror -I/usr/local/include/hiredis -I/usr/local/include -I/usr/local/ocilib/include -I../common -I../../base/tools -I../../base/hsm -I../../base/kms -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/chkagent.o chkagent.c

${OBJECTDIR}/chkmerch.o: chkmerch.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Werror -I/usr/local/include/hiredis -I/usr/local/include -I/usr/local/ocilib/include -I../common -I../../base/tools -I../../base/hsm -I../../base/kms -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/chkmerch.o chkmerch.c

${OBJECTDIR}/chkterm.o: chkterm.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Werror -I/usr/local/include/hiredis -I/usr/local/include -I/usr/local/ocilib/include -I../common -I../../base/tools -I../../base/hsm -I../../base/kms -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/chkterm.o chkterm.c

${OBJECTDIR}/cups_resetkey.o: cups_resetkey.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Werror -I/usr/local/include/hiredis -I/usr/local/include -I/usr/local/ocilib/include -I../common -I../../base/tools -I../../base/hsm -I../../base/kms -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/cups_resetkey.o cups_resetkey.c

${OBJECTDIR}/db_order.o: db_order.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Werror -I/usr/local/include/hiredis -I/usr/local/include -I/usr/local/ocilib/include -I../common -I../../base/tools -I../../base/hsm -I../../base/kms -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/db_order.o db_order.c

${OBJECTDIR}/db_resetkey.o: db_resetkey.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Werror -I/usr/local/include/hiredis -I/usr/local/include -I/usr/local/ocilib/include -I../common -I../../base/tools -I../../base/hsm -I../../base/kms -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/db_resetkey.o db_resetkey.c

${OBJECTDIR}/down_param.o: down_param.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Werror -I/usr/local/include/hiredis -I/usr/local/include -I/usr/local/ocilib/include -I../common -I../../base/tools -I../../base/hsm -I../../base/kms -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/down_param.o down_param.c

${OBJECTDIR}/service.o: service.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Werror -I/usr/local/include/hiredis -I/usr/local/include -I/usr/local/ocilib/include -I../common -I../../base/tools -I../../base/hsm -I../../base/kms -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/service.o service.c

${OBJECTDIR}/status_upload.o: status_upload.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Werror -I/usr/local/include/hiredis -I/usr/local/include -I/usr/local/ocilib/include -I../common -I../../base/tools -I../../base/hsm -I../../base/kms -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/status_upload.o status_upload.c

${OBJECTDIR}/term_init.o: term_init.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Werror -I/usr/local/include/hiredis -I/usr/local/include -I/usr/local/ocilib/include -I../common -I../../base/tools -I../../base/hsm -I../../base/kms -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/term_init.o term_init.c

${OBJECTDIR}/term_sign.o: term_sign.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Werror -I/usr/local/include/hiredis -I/usr/local/include -I/usr/local/ocilib/include -I../common -I../../base/tools -I../../base/hsm -I../../base/kms -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/term_sign.o term_sign.c

${OBJECTDIR}/term_signature.o: term_signature.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Werror -I/usr/local/include/hiredis -I/usr/local/include -I/usr/local/ocilib/include -I../common -I../../base/tools -I../../base/hsm -I../../base/kms -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/term_signature.o term_signature.c

# Subprojects
.build-subprojects:
	cd ../../base/hsm && ${MAKE}  -f Makefile CONF=Debug
	cd ../../base/tools && ${MAKE}  -f Makefile CONF=Debug
	cd ../common && ${MAKE}  -f Makefile CONF=Debug
	cd ../../base/kms && ${MAKE}  -f Makefile CONF=Debug

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}

# Subprojects
.clean-subprojects:
	cd ../../base/hsm && ${MAKE}  -f Makefile CONF=Debug clean
	cd ../../base/tools && ${MAKE}  -f Makefile CONF=Debug clean
	cd ../common && ${MAKE}  -f Makefile CONF=Debug clean
	cd ../../base/kms && ${MAKE}  -f Makefile CONF=Debug clean

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
