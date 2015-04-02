# Makefile for micronfcboard
GCC_BIN = 
TARGET = TARGET_APPNEARME_MICRONFCBOARD
TOOLCHAIN = TOOLCHAIN_GCC_ARM
INCLUDE_PATHS = -I./intfs/ -I./types/

LIBRARY = libnxprdlib.a
SOURCES = ./comps/phacDiscLoop/src/phacDiscLoop.c \
./comps/phacDiscLoop/src/Sw/phacDiscLoop_Sw.c \
./comps/phacDiscLoop/src/Sw/phacDiscLoop_Sw_Int.c \
./comps/phalFelica/src/phalFelica.c \
./comps/phalFelica/src/Sw/phalFelica_Sw.c \
./comps/phalMfc/src/phalMfc.c \
./comps/phalMfc/src/phalMfc_Int.c \
./comps/phalMfc/src/Sw/phalMfc_Sw.c \
./comps/phalMfdf/src/phalMfdf.c \
./comps/phalMfdf/src/phalMfdf_Int.c \
./comps/phalMfdf/src/Sw/phalMfdf_Sw.c \
./comps/phalMfdf/src/Sw/phalMfdf_Sw_Int.c \
./comps/phalMful/src/phalMful.c \
./comps/phalMful/src/phalMful_Int.c \
./comps/phalMful/src/Sw/phalMful_Sw.c \
./comps/phalT1T/src/phalT1T.c \
./comps/phalT1T/src/Sw/phalT1T_Sw.c \
./comps/phalTop/src/phalTop.c \
./comps/phalTop/src/Sw/phalTop_Sw.c \
./comps/phalTop/src/Sw/phalTop_Sw_Int_T1T.c \
./comps/phalTop/src/Sw/phalTop_Sw_Int_T2T.c \
./comps/phalTop/src/Sw/phalTop_Sw_Int_T3T.c \
./comps/phalTop/src/Sw/phalTop_Sw_Int_T4T.c \
./comps/phbalReg/src/phbalReg.c \
./comps/phCidManager/src/phCidManager.c \
./comps/phCidManager/src/Sw/phCidManager_Sw.c \
./comps/phhalHw/src/Callback/phhalHw_Callback.c \
./comps/phhalHw/src/phhalHw.c \
./comps/phhalHw/src/Rc523/phhalHw_Rc523.c \
./comps/phhalHw/src/Rc523/phhalHw_Rc523_Cmd.c \
./comps/phhalHw/src/Rc523/phhalHw_Rc523_Int.c \
./comps/phhalHw/src/Rc523/phhalHw_Rc523_Wait.c \
./comps/phhalHw/src/Rc663/phhalHw_Rc663.c \
./comps/phhalHw/src/Rc663/phhalHw_Rc663_Cmd.c \
./comps/phhalHw/src/Rc663/phhalHw_Rc663_Int.c \
./comps/phhalHw/src/Rc663/phhalHw_Rc663_Wait.c \
./comps/phKeyStore/src/phKeyStore.c \
./comps/phKeyStore/src/Rc663/phKeyStore_Rc663.c \
./comps/phKeyStore/src/Sw/phKeyStore_Sw.c \
./comps/phlnLlcp/src/Fri/phlnLlcp_Fri_Llcp.c \
./comps/phlnLlcp/src/Fri/phlnLlcp_Fri_Mac.c \
./comps/phlnLlcp/src/Fri/phlnLlcp_Fri_MacNfcip.c \
./comps/phlnLlcp/src/Fri/phlnLlcp_Fri_OvrHal.c \
./comps/phlnLlcp/src/Fri/phlnLlcp_Fri_Transport.c \
./comps/phlnLlcp/src/Fri/phlnLlcp_Fri_Transport_Connection.c \
./comps/phlnLlcp/src/Fri/phlnLlcp_Fri_Transport_Connectionless.c \
./comps/phlnLlcp/src/Fri/phlnLlcp_Fri_Utils.c \
./comps/phlnLlcp/src/phlnLlcp.c \
./comps/phLog/src/phLog.c \
./comps/phnpSnep/src/Fri/phnpSnep_Fri.c \
./comps/phnpSnep/src/Fri/phnpSnep_Fri_ContextMgmt.c \
./comps/phnpSnep/src/Fri/phnpSnep_Fri_Utils.c \
./comps/phnpSnep/src/phnpSnep.c \
./comps/phOsal/src/phOsal.c \
./comps/phpalFelica/src/phpalFelica.c \
./comps/phpalFelica/src/Sw/phpalFelica_Sw.c \
./comps/phpalI14443p3a/src/phpalI14443p3a.c \
./comps/phpalI14443p3a/src/Sw/phpalI14443p3a_Sw.c \
./comps/phpalI14443p3b/src/phpalI14443p3a_Sw<43p3b.c \
./comps/phpalI14443p3b/src/Sw/phpalI14443p3b_Sw.c \
./comps/phpalI14443p4/src/phpalI14443p4.c \
./comps/phpalI14443p4/src/Sw/phpalI14443p4_Sw.c \
./comps/phpalI14443p4a/src/phpalI14443p4a.c \
./comps/phpalI14443p4a/src/Sw/phpalI14443p4a_Sw.c \
./comps/phpalI18092mPI/src/phpalI18092mPI.c \
./comps/phpalI18092mPI/src/Sw/phpalI18092mPI_Sw.c \
./comps/phpalI18092mPI/src/Sw/phpalI18092mPI_Sw_Int.c \
./comps/phpalI18092mT/src/phpalI18092mT.c \
./comps/phpalI18092mT/src/Sw/phpalI18092mT_Sw.c \
./comps/phpalI18092mT/src/Sw/phpalI18092mT_Sw_Int.c \
./comps/phpalMifare/src/phpalMifare.c \
./comps/phpalMifare/src/Sw/phpalMifare_Sw.c \
./comps/phTools/src/phTools.c \
./comps/phOsal/src/phOsal_GLib.c \
./comps/phbalReg/src/phBalReg_RpiSpi.c 

OBJECTS=$(SOURCES:.c=.o)

############################################################################### 
AS      = $(GCC_BIN)arm-none-eabi-as
CC      = $(GCC_BIN)arm-none-eabi-gcc
CPP     = $(GCC_BIN)arm-none-eabi-g++
LD      = $(GCC_BIN)arm-none-eabi-gcc
OBJCOPY = $(GCC_BIN)arm-none-eabi-objcopy
OBJDUMP = $(GCC_BIN)arm-none-eabi-objdump
SIZE 	= $(GCC_BIN)arm-none-eabi-size
AR	= $(GCC_BIN)arm-none-eabi-ar

CPU = -mcpu=cortex-m0 -mthumb
CC_FLAGS = $(CPU) -c -g -fno-common -fmessage-length=0 -Wall -fno-exceptions -ffunction-sections -fdata-sections -fomit-frame-pointer
CC_FLAGS += -MMD -MP
CC_SYMBOLS = -DTARGET_APPNEARME_MICRONFCBOARD -DTARGET_M0 -DTARGET_CORTEX_M -DTARGET_NXP -DTARGET_LPC11UXX -D$(TOOLCHAIN) -DTOOLCHAIN_GCC -D__CORTEX_M0 -DARM_MATH_CM0 -DMBED_BUILD_TIMESTAMP=1427405354.12 -D__MBED__=1 
ARFLAGS = -r

ifeq ($(DEBUG), 1)
  CC_FLAGS += -DDEBUG -O0
else
  CC_FLAGS += -DNDEBUG -Os
endif

all: $(LIBRARY)
	
clean:
	rm -f $(PROJECT).bin $(PROJECT).elf $(PROJECT).hex $(PROJECT).map $(PROJECT).lst $(OBJECTS) $(DEPS)

.s.o:
	$(AS) $(CPU) -o $@ $<

.c.o:
	$(CC)  $(CC_FLAGS) $(CC_SYMBOLS) -std=gnu99   $(INCLUDE_PATHS) -o $@ $<

.cpp.o:
	$(CPP) $(CC_FLAGS) $(CC_SYMBOLS) -std=gnu++98 -fno-rtti $(INCLUDE_PATHS) -o $@ $<

$(LIBRARY): $(OBJECTS)
	$(AR) $(ARFLAGS) $@ $^

DEPS = $(OBJECTS:.o=.d)
-include $(DEPS)
