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
	${OBJECTDIR}/channel_conv.o \
	${OBJECTDIR}/chkmerch.o \
	${OBJECTDIR}/chkuser.o \
	${OBJECTDIR}/d0_trans.o \
	${OBJECTDIR}/db_chkpufa.o \
	${OBJECTDIR}/db_d0.o \
	${OBJECTDIR}/fee.o \
	${OBJECTDIR}/genurl.o \
	${OBJECTDIR}/judgecard.o \
	${OBJECTDIR}/merch_limit.o \
	${OBJECTDIR}/old_trans_detail.o \
	${OBJECTDIR}/risk_rule.o \
	${OBJECTDIR}/service.o \
	${OBJECTDIR}/settledate.o \
	${OBJECTDIR}/shieldrisk.o \
	${OBJECTDIR}/wallet.o


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
LDLIBSOPTIONS=-L/usr/local/lib -L/usr/local/ocilib/lib -L${HOME}/lib -L../../base/kms -L../../base/tools -L${HOME}/api2016/lib64 -locilib -lhiredis -lzlog -lunionAPI2 ../../base/kms/dist/Debug/GNU-Linux/libkms.a ../common/dist/Debug/GNU-Linux/libcommon.a ../../base/tools/dist/Debug/GNU-Linux/libtools.a

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${HOME}/lib/libaccount.${CND_DLIB_EXT}

${HOME}/lib/libaccount.${CND_DLIB_EXT}: ../../base/kms/dist/Debug/GNU-Linux/libkms.a

${HOME}/lib/libaccount.${CND_DLIB_EXT}: ../common/dist/Debug/GNU-Linux/libcommon.a

${HOME}/lib/libaccount.${CND_DLIB_EXT}: ../../base/tools/dist/Debug/GNU-Linux/libtools.a

${HOME}/lib/libaccount.${CND_DLIB_EXT}: ${OBJECTFILES}
	${MKDIR} -p ${HOME}/lib
	${LINK.c} -o ${HOME}/lib/libaccount.${CND_DLIB_EXT} ${OBJECTFILES} ${LDLIBSOPTIONS} -shared -fPIC

${OBJECTDIR}/channel_conv.o: channel_conv.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Werror -I/usr/local/include/hiredis -I/usr/local/include -I/usr/local/ocilib/include -I../common -I../../base/tools -I../../base/kms -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/channel_conv.o channel_conv.c

${OBJECTDIR}/chkmerch.o: chkmerch.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Werror -I/usr/local/include/hiredis -I/usr/local/include -I/usr/local/ocilib/include -I../common -I../../base/tools -I../../base/kms -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/chkmerch.o chkmerch.c

${OBJECTDIR}/chkuser.o: chkuser.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Werror -I/usr/local/include/hiredis -I/usr/local/include -I/usr/local/ocilib/include -I../common -I../../base/tools -I../../base/kms -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/chkuser.o chkuser.c

${OBJECTDIR}/d0_trans.o: d0_trans.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Werror -I/usr/local/include/hiredis -I/usr/local/include -I/usr/local/ocilib/include -I../common -I../../base/tools -I../../base/kms -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/d0_trans.o d0_trans.c

${OBJECTDIR}/db_chkpufa.o: db_chkpufa.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Werror -I/usr/local/include/hiredis -I/usr/local/include -I/usr/local/ocilib/include -I../common -I../../base/tools -I../../base/kms -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/db_chkpufa.o db_chkpufa.c

${OBJECTDIR}/db_d0.o: db_d0.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Werror -I/usr/local/include/hiredis -I/usr/local/include -I/usr/local/ocilib/include -I../common -I../../base/tools -I../../base/kms -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/db_d0.o db_d0.c

${OBJECTDIR}/fee.o: fee.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Werror -I/usr/local/include/hiredis -I/usr/local/include -I/usr/local/ocilib/include -I../common -I../../base/tools -I../../base/kms -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/fee.o fee.c

${OBJECTDIR}/genurl.o: genurl.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Werror -I/usr/local/include/hiredis -I/usr/local/include -I/usr/local/ocilib/include -I../common -I../../base/tools -I../../base/kms -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/genurl.o genurl.c

${OBJECTDIR}/judgecard.o: judgecard.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Werror -I/usr/local/include/hiredis -I/usr/local/include -I/usr/local/ocilib/include -I../common -I../../base/tools -I../../base/kms -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/judgecard.o judgecard.c

${OBJECTDIR}/merch_limit.o: merch_limit.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Werror -I/usr/local/include/hiredis -I/usr/local/include -I/usr/local/ocilib/include -I../common -I../../base/tools -I../../base/kms -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/merch_limit.o merch_limit.c

${OBJECTDIR}/old_trans_detail.o: old_trans_detail.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Werror -I/usr/local/include/hiredis -I/usr/local/include -I/usr/local/ocilib/include -I../common -I../../base/tools -I../../base/kms -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/old_trans_detail.o old_trans_detail.c

${OBJECTDIR}/risk_rule.o: risk_rule.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Werror -I/usr/local/include/hiredis -I/usr/local/include -I/usr/local/ocilib/include -I../common -I../../base/tools -I../../base/kms -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/risk_rule.o risk_rule.c

${OBJECTDIR}/service.o: service.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Werror -I/usr/local/include/hiredis -I/usr/local/include -I/usr/local/ocilib/include -I../common -I../../base/tools -I../../base/kms -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/service.o service.c

${OBJECTDIR}/settledate.o: settledate.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Werror -I/usr/local/include/hiredis -I/usr/local/include -I/usr/local/ocilib/include -I../common -I../../base/tools -I../../base/kms -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/settledate.o settledate.c

${OBJECTDIR}/shieldrisk.o: shieldrisk.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Werror -I/usr/local/include/hiredis -I/usr/local/include -I/usr/local/ocilib/include -I../common -I../../base/tools -I../../base/kms -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/shieldrisk.o shieldrisk.c

${OBJECTDIR}/wallet.o: wallet.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -Werror -I/usr/local/include/hiredis -I/usr/local/include -I/usr/local/ocilib/include -I../common -I../../base/tools -I../../base/kms -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/wallet.o wallet.c

# Subprojects
.build-subprojects:
	cd ../../base/kms && ${MAKE}  -f Makefile CONF=Debug
	cd ../common && ${MAKE}  -f Makefile CONF=Debug
	cd ../../base/tools && ${MAKE}  -f Makefile CONF=Debug

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}

# Subprojects
.clean-subprojects:
	cd ../../base/kms && ${MAKE}  -f Makefile CONF=Debug clean
	cd ../common && ${MAKE}  -f Makefile CONF=Debug clean
	cd ../../base/tools && ${MAKE}  -f Makefile CONF=Debug clean

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
