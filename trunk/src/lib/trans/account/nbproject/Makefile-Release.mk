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
CND_CONF=Release
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/addnotice.o \
	${OBJECTDIR}/amountproc.o \
	${OBJECTDIR}/channel_conv.o \
	${OBJECTDIR}/chkagent.o \
	${OBJECTDIR}/chkfreedown.o \
	${OBJECTDIR}/chkhisoriginfo.o \
	${OBJECTDIR}/chkmerch.o \
	${OBJECTDIR}/chkmerchcard.o \
	${OBJECTDIR}/chkmerchcardpinerr.o \
	${OBJECTDIR}/chkoldtransdetail.c.o \
	${OBJECTDIR}/chkoriginauth.o \
	${OBJECTDIR}/chkorigininfo.o \
	${OBJECTDIR}/chkterm.o \
	${OBJECTDIR}/db_amtproc.o \
	${OBJECTDIR}/db_card.o \
	${OBJECTDIR}/db_chkpufa.o \
	${OBJECTDIR}/db_d0.o \
	${OBJECTDIR}/db_geturl.o \
	${OBJECTDIR}/db_originauth.o \
	${OBJECTDIR}/db_originfo.o \
	${OBJECTDIR}/db_riskrule.o \
	${OBJECTDIR}/db_risksamecard.o \
	${OBJECTDIR}/fee.o \
	${OBJECTDIR}/genurl.o \
	${OBJECTDIR}/judgecard.o \
	${OBJECTDIR}/merch_limit.o \
	${OBJECTDIR}/order.o \
	${OBJECTDIR}/riskcardblacklist.o \
	${OBJECTDIR}/riskquerymerch.o \
	${OBJECTDIR}/riskrule.o \
	${OBJECTDIR}/risksamecard.o \
	${OBJECTDIR}/service.o \
	${OBJECTDIR}/settledate.o \
	${OBJECTDIR}/shieldrisk.o \
	${OBJECTDIR}/t0inspect.o \
	${OBJECTDIR}/updoldpostransdetail.o \
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
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libaccount.${CND_DLIB_EXT}

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libaccount.${CND_DLIB_EXT}: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.c} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libaccount.${CND_DLIB_EXT} ${OBJECTFILES} ${LDLIBSOPTIONS} -shared -fPIC

${OBJECTDIR}/addnotice.o: addnotice.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/addnotice.o addnotice.c

${OBJECTDIR}/amountproc.o: amountproc.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/amountproc.o amountproc.c

${OBJECTDIR}/channel_conv.o: channel_conv.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/channel_conv.o channel_conv.c

${OBJECTDIR}/chkagent.o: chkagent.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/chkagent.o chkagent.c

${OBJECTDIR}/chkfreedown.o: chkfreedown.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/chkfreedown.o chkfreedown.c

${OBJECTDIR}/chkhisoriginfo.o: chkhisoriginfo.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/chkhisoriginfo.o chkhisoriginfo.c

${OBJECTDIR}/chkmerch.o: chkmerch.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/chkmerch.o chkmerch.c

${OBJECTDIR}/chkmerchcard.o: chkmerchcard.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/chkmerchcard.o chkmerchcard.c

${OBJECTDIR}/chkmerchcardpinerr.o: chkmerchcardpinerr.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/chkmerchcardpinerr.o chkmerchcardpinerr.c

${OBJECTDIR}/chkoldtransdetail.c.o: chkoldtransdetail.c.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/chkoldtransdetail.c.o chkoldtransdetail.c.c

${OBJECTDIR}/chkoriginauth.o: chkoriginauth.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/chkoriginauth.o chkoriginauth.c

${OBJECTDIR}/chkorigininfo.o: chkorigininfo.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/chkorigininfo.o chkorigininfo.c

${OBJECTDIR}/chkterm.o: chkterm.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/chkterm.o chkterm.c

${OBJECTDIR}/db_amtproc.o: db_amtproc.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/db_amtproc.o db_amtproc.c

${OBJECTDIR}/db_card.o: db_card.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/db_card.o db_card.c

${OBJECTDIR}/db_chkpufa.o: db_chkpufa.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/db_chkpufa.o db_chkpufa.c

${OBJECTDIR}/db_d0.o: db_d0.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/db_d0.o db_d0.c

${OBJECTDIR}/db_geturl.o: db_geturl.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/db_geturl.o db_geturl.c

${OBJECTDIR}/db_originauth.o: db_originauth.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/db_originauth.o db_originauth.c

${OBJECTDIR}/db_originfo.o: db_originfo.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/db_originfo.o db_originfo.c

${OBJECTDIR}/db_riskrule.o: db_riskrule.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/db_riskrule.o db_riskrule.c

${OBJECTDIR}/db_risksamecard.o: db_risksamecard.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/db_risksamecard.o db_risksamecard.c

${OBJECTDIR}/fee.o: fee.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/fee.o fee.c

${OBJECTDIR}/genurl.o: genurl.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/genurl.o genurl.c

${OBJECTDIR}/judgecard.o: judgecard.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/judgecard.o judgecard.c

${OBJECTDIR}/merch_limit.o: merch_limit.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/merch_limit.o merch_limit.c

${OBJECTDIR}/order.o: order.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/order.o order.c

${OBJECTDIR}/riskcardblacklist.o: riskcardblacklist.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/riskcardblacklist.o riskcardblacklist.c

${OBJECTDIR}/riskquerymerch.o: riskquerymerch.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/riskquerymerch.o riskquerymerch.c

${OBJECTDIR}/riskrule.o: riskrule.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/riskrule.o riskrule.c

${OBJECTDIR}/risksamecard.o: risksamecard.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/risksamecard.o risksamecard.c

${OBJECTDIR}/service.o: service.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/service.o service.c

${OBJECTDIR}/settledate.o: settledate.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/settledate.o settledate.c

${OBJECTDIR}/shieldrisk.o: shieldrisk.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/shieldrisk.o shieldrisk.c

${OBJECTDIR}/t0inspect.o: t0inspect.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/t0inspect.o t0inspect.c

${OBJECTDIR}/updoldpostransdetail.o: updoldpostransdetail.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/updoldpostransdetail.o updoldpostransdetail.c

${OBJECTDIR}/wallet.o: wallet.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/wallet.o wallet.c

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
