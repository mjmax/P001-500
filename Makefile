# makefile
#
# On command line:
#
# make all = Make software with checksum, including objects, list files, dependencies and output
# make release = Make software output with checksum, copy of common files under source for placing on server
# make clean = Clean out built project files.
# make filename.s = Just compile filename.c into the assembler code only
#
# To rebuild project do "make clean" then "make all" ("make clean all" or "make clean release").
#
# Directory structure is as follows:
# X300-508
#  |
#  +- src          <-- Source files for main application
#  +- src_bt       <-- Source files for bluetooth application
#  +- M01          <-- Main application s-record file
#  +- M02          <-- Main application programming exe
#  +- M05          <-- Bluetooth application zip containing "AIRcable.bas"
#
# Notes:
#   (1) Source found in common directory at time of "make release". This will only appear for "make release"
#   (2) Objects created during make. This will be deleted for "make release" (will stay for all other makes)
#   (3) List files created during make. This will be deleted for "make release" (will stay for all other makes)
#

# Special configuration for each configuration
MSG_CONFIG = P001-500 Main Application
TARGET = P001-500
MCU = atxmega128d3
TARGET_BT = AIRcable.bas


# Toolchain directories
UTILITYDIR = W:/utility
TOOLCHAINBASE = $(UTILITYDIR)/avr8-gnu-toolchain
TOOLCHAINDIR = $(TOOLCHAINBASE)/bin
TOOLSDIR = $(UTILITYDIR)/GnuWin32/bin
RINTOOLSDIR = $(UTILITYDIR)


# Checksum generation
CHECKSUMSEED = 8000
CHECKSUMSTARTADDRESS = 00000000
CHECKSUMENDADDRESS = 00020000
CHECKSUMBLOCKSIZE = 1000


# Define programs and commands.
CC = $(TOOLCHAINDIR)/avr-gcc
REMOVE = "$(TOOLSDIR)/rm.exe" -f
COPY = "$(TOOLSDIR)/cp.exe"
REMOVEDIR = "$(TOOLSDIR)/rm.exe" -f -r
ECHO = "$(TOOLSDIR)/echo.exe"
MKDIR = "$(TOOLSDIR)/mkdir.exe" -p
GREP = "$(TOOLSDIR)/grep.exe"
SED = "$(TOOLSDIR)/sed.exe"
TR = "$(TOOLSDIR)/tr.exe"
ADDCHECKSUM = "$(RINTOOLSDIR)/blockchk.exe"
SJOIN = "$(RINTOOLSDIR)/sjoin.exe"
EXTENDBIN = "$(RINTOOLSDIR)/binextend.exe"
ZIP = "$(TOOLSDIR)/zip.exe"
DOXYGEN = "C:/Program Files/doxygen/bin/doxygen.exe"
PWD = "$(TOOLSDIR)/pwd.exe"
MAKEUSER = "$(RINTOOLSDIR)/makeusercl_192.exe"
OBJCOPY = $(TOOLCHAINDIR)/avr-objcopy
OBJDUMP = $(TOOLCHAINDIR)/avr-objdump
SIZE = $(TOOLCHAINDIR)/avr-size
NM = $(TOOLCHAINDIR)/avr-nm
AVRDUDE = $(TOOLCHAINDIR)/avrdude
CHECKSIZE = $(RINTOOLSDIR)/checkgccsize.exe


# Library and build directories
SRC_DIR = ./src
OBJ_DIR = ./objs
OUT_DIR = ./output
M01_DIR = ./M01
M02_DIR = ./M02
SRC_BT_DIR = ./src_bt
M05_DIR = ./M05


# Version
VERSION := $(shell $(SED) -n -r "s/.define\s+MODULE_VERSION\s+\"P001-500-([0-9]+\.[0-9]+[0-9]+)\s*\".*/\1/p" < ./src/defines.h)


# Target prefixed with directory
TARG_WITH_DIR = $(OUT_DIR)/$(TARGET)
RELEASE_M01_TARGET = $(M01_DIR)/$(TARGET)-$(VERSION)-M01.mot
RELEASE_M02_TARGET1 = $(M02_DIR)/$(TARGET)-$(VERSION)-M02.exe
RELEASE_M02_TARGET2 = $(M02_DIR)/$(TARGET)-$(VERSION)-M02.zip
RELEASE_M05_TARGET = $(M05_DIR)/$(TARGET)-$(VERSION)-M05.zip


# List C source files here. (C dependencies are automatically generated.)
SRC = $(SRC_DIR)/example.c
SRC += $(SRC_DIR)/fifo_flush.c
SRC += $(SRC_DIR)/fifo_free.c
SRC += $(SRC_DIR)/fifo_getchar.c
SRC += $(SRC_DIR)/fifo_getstring.c
SRC += $(SRC_DIR)/fifo_init.c
SRC += $(SRC_DIR)/fifo_isempty.c
SRC += $(SRC_DIR)/fifo_putchar.c
SRC += $(SRC_DIR)/fifo_putstring.c
SRC += $(SRC_DIR)/fifo_used.c
SRC += $(SRC_DIR)/hardware.c
SRC += $(SRC_DIR)/inputs.c
SRC += $(SRC_DIR)/outputs.c
SRC += $(SRC_DIR)/r300.c
SRC += $(SRC_DIR)/usart_int.c
SRC += $(SRC_DIR)/wand.c
SRC += $(SRC_DIR)/vectdata.c
SRC += $(SRC_DIR)/command.c
SRC += $(SRC_DIR)/registers.c
SRC += $(SRC_DIR)/sci.c
SRC += $(SRC_DIR)/_sprintf.c
SRC += $(SRC_DIR)/eeprom.c
SRC += $(SRC_DIR)/database.c
SRC += $(SRC_DIR)/clock.c
SRC += $(SRC_DIR)/systems.c

# List of all source directories
SRC_DIRS = $(SRC_DIR)

# List bluetooth source rfiles
SRC_BT = $(SRC_BT_DIR)/$(TARGET).bas


# Search these folders for files 
EMPTY:=
SPACE:= $(EMPTY) $(EMPTY)
VPATH = $(subst $(SPACE),;,$(SRC_DIRS)) 


# List Assembler source files here.
# Make them always end in a capital .S.  Files ending in a lowercase .s
# will not be considered source files but generated files (assembler
# output from the compiler), and will be deleted upon "make clean"!
# Even though the DOS/Win* filesystem matches both .s and .S the same,
# it will preserve the spelling of the filenames, and gcc itself does
# care about how the name is spelled on its command-line.
ASRC = $(SRC_DIR)/cstart.S


# List any extra C include file directories here
C_INC_DIRS = $(SRC_DIR)


# Compiler flag to set the C Standard level.
# c89   - "ANSI" C
# gnu89 - c89 plus GCC extensions
# c99   - ISO C99 standard (not yet fully implemented)
# gnu99 - c99 plus GCC extensions
CSTANDARD = -std=gnu99

# Compiler flags.
#  -O*:          optimization level (0-3, s)
#  -f...:        tuning, see GCC manual and avr-libc documentation
#  -Wall...:     warning level
#  -Wa,...:      tell GCC to pass this to the assembler.
#    -adhlns...: create assembler listing
CFLAGS = -Os
CFLAGS += --freestanding
CFLAGS += -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CFLAGS += -Wall -Wstrict-prototypes
CFLAGS += -Wa,-adhlmns=$(OBJ_DIR)/$(patsubst %.c,%.lst,$(<F))
CFLAGS += $(CSTANDARD)
CFLAGS += -Werror


# Assembler flags.
#  -Wa,...:   tell GCC to pass this to the assembler.
#  -ahlms:    create listing
#  -gstabs:   have the assembler create line number information; note that
#             for use in COFF files, additional information about filenames
#             and function names needs to be present in the assembler source
#             files -- see avr-libc docs [FIXME: not yet described there]
#ASFLAGS = -adhlmns=$(OBJ_DIR)/$(patsubst %.S,.lst,$(<F)),-gstabs
ASFLAGS = 


# Linker flags.
#  -Wl,...:     tell GCC to pass this to linker.
#    -Map:      create map file
#    --cref:    add cross reference to  map file
LDFLAGS = -mmcu=$(MCU) -nostartfiles -Wl,-Map=$(TARG_WITH_DIR).map,--cref
LDFLAGS += -lm


# Define Messages
MSG_ERRORS_NONE = Errors: none
MSG_BEGIN = -------- begin --------
MSG_BUILD_FOR = !!!!BUILDING FOR: 
MSG_END = --------  end  --------
MSG_FLASH = Creating load file for Flash:
MSG_EEPROM = Creating load file for EEPROM:
MSG_EXTENDED_LISTING = Creating Extended Listing:
MSG_SYMBOL_TABLE = Creating Symbol Table:
MSG_LINKING = Linking:
MSG_COMPILING = Compiling:
MSG_ASSEMBLING = Assembling:
MSG_CLEANING = Cleaning project:
MSG_CREATE_DIR = Creating directories:
MSG_COMPILER = Building with compiler:
MSG_CHECK_SIZE = Checking code and data size:


# Define all object files
OBJ = $(addprefix $(OBJ_DIR)/, $(notdir $(patsubst %.c,%.o,$(SRC)) $(patsubst %.S,%.o,$(ASRC)) ) )

# Define all listing files
LST = $(addprefix $(OBJ_DIR)/, $(notdir $(patsubst %.S,%.lst,$(ASRC)) $(patsubst %.c,%.lst,$(SRC)) ) )


# Combine all necessary flags and optional flags.
# Add target processor to flags.
ALL_CFLAGS = -mmcu=$(MCU) $(CFLAGS)
ALL_ASFLAGS = -mmcu=$(MCU) -x assembler-with-cpp $(ASFLAGS)


# Default target.
all: begin ccversion createdirs build checksize finished info end

# Release target (cleans up intermediate build files)
release: begin clean ccversion createdirs build checksize release_files clean_release finished info release_info end


# Work out the dependancies for the build rule
BUILD_DEP = $(TARG_WITH_DIR).mot
RELEASE_DEP = $(RELEASE_M01_TARGET) $(RELEASE_M02_TARGET1) $(RELEASE_M02_TARGET2) $(RELEASE_M05_TARGET)  


# Make the appropriate files for the M0x folders
release_files: $(RELEASE_DEP)

# To build with checksum
build: $(BUILD_DEP)

# To build the doxygen documentation
doxygen:
	$(DOXYGEN) Doxyfile


# Pretty messaging
begin:
	@$(ECHO)
	@$(ECHO) $(MSG_BEGIN) 
	@$(ECHO) $(MSG_BUILD_FOR) $(MSG_CONFIG)
	@$(ECHO)

finished:
	@$(ECHO) $(MSG_ERRORS_NONE)

end:
	@$(ECHO) $(MSG_END)
	@$(ECHO)
	
info:
	@$(ECHO)
	@$(ECHO) $(MSG_BUILD_FOR) $(MSG_CONFIG)
	@$(ECHO)

release_info:
	@$(ECHO) M01 Output: $(RELEASE_M01_TARGET) - Application MOT file
	@$(ECHO) M02 Output: $(RELEASE_M02_TARGET1) $(RELEASE_M02_TARGET2) - Application user programming exe
	@$(ECHO) M05 Output: $(RELEASE_M05_TARGET) - Bluetooth Application zip file

createdirs:
	@$(ECHO)
	@$(ECHO) $(MSG_CREATE_DIR)
	$(MKDIR) $(OBJ_DIR)
	$(MKDIR) $(OUT_DIR)
	$(MKDIR) $(M01_DIR)
	$(MKDIR) $(M02_DIR)
	$(MKDIR) $(M05_DIR)


# Display compiler version information.
ccversion:
	@$(ECHO) $(MSG_COMPILER)
	@$(CC) --version


# Rule to create M01 release target
$(RELEASE_M01_TARGET): $(TARG_WITH_DIR).mot
	$(COPY) $< $@

# Rule to create M02 release target (.exe)
$(RELEASE_M02_TARGET1): $(TARG_WITH_DIR).mot
# Edit X300-005.dev to set MotFile, Description and OutputFile
	$(SED) -r "s/(MotFile=)(.*)/\1$(subst /,\\,$(subst .,\.,$<))/" ./makeuser/X300-005.dev | \
	$(SED) -r "s/(Description=)(.*)/\1$(TARGET)-$(VERSION)-M02 AgX320 Rear PCB Application/" | \
	$(SED) -r "s/(OutputFile=)(.*)/\1$(subst /,\\,$(subst .,\.,$@))/" > X300-005.dev
	$(MAKEUSER) X300-005.dev


# Rule to create M02 release target (.zip)  
$(RELEASE_M02_TARGET2): $(RELEASE_M02_TARGET1)
	$(ZIP) -9 -j $@ $<


# Rule to create M05 release target (.zip)  
$(RELEASE_M05_TARGET): $(SRC_BT)
	$(COPY) $< $(TARGET_BT)
	$(ZIP) -9 -j $@ $(TARGET_BT)


# Rule to create output Motorola S-Record with checksum from elf file
$(TARG_WITH_DIR).mot: $(TARG_WITH_DIR).elf
	@$(ECHO)
	@$(ECHO) $(MSG_FLASH) $@
	$(OBJCOPY) -O srec -R .eeprom $< $(OUT_DIR)/temp.mot
	$(ADDCHECKSUM) $(CHECKSUMSEED) $(CHECKSUMSTARTADDRESS) $(CHECKSUMENDADDRESS) $(CHECKSUMBLOCKSIZE) $(OUT_DIR)/temp.mot $@
	@$(REMOVE) $(OUT_DIR)/temp.mot


# Check that none of the sections have overflowed
checksize: $(TARG_WITH_DIR).elf
	@$(ECHO)
	@$(ECHO) $(MSG_CHECK_SIZE)
	$(SIZE) -A $(TARG_WITH_DIR).elf > checksize.txt
	$(CHECKSIZE) --limit=.text,131072,0 --limit=.data,8192, --combine=.data,.bss checksize.txt
	@$(REMOVE) checksize.txt


# Link rule to create ELF output file from object files
.SECONDARY : $(TARG_WITH_DIR).elf
.PRECIOUS : $(OBJ)
$(TARG_WITH_DIR).elf: $(OBJ) $(LIB)
	@$(ECHO)
	@$(ECHO) $(MSG_LINKING) $@
	$(CC) $(ALL_CFLAGS) $(OBJ) --output $@ $(LDFLAGS)


# Cancel implicit rule, because we want to compile .o's into the object directory
%.o : %.c


# Compile: create object files from C source files.
$(OBJ_DIR)/%.o : %.c
	@$(ECHO)
	@$(ECHO) $(MSG_COMPILING) $<
	$(CC) -c $(ALL_CFLAGS) $(patsubst %,-I%,$(C_INC_DIRS)) \
	      $< -o $(OBJ_DIR)/$(@F)


# Compile: create assembler files from C source files.
%.s : %.c
	@$(ECHO)
	@$(ECHO) Compiling $< to assembler source
	$(CC) -S $(ALL_CFLAGS) $< -o ./src/$(@F)


# Assemble: create object files from assembler source files.
$(OBJ_DIR)/%.o : %.S
	@$(ECHO)
	@$(ECHO) $(MSG_ASSEMBLING) $<
	$(CC) -c $(ALL_ASFLAGS) $< -o $(OBJ_DIR)/$(@F)


# Target: clean project.
clean: begin clean_all finished end


# Rule to clean up everything
clean_all:
	@$(ECHO)
	@$(ECHO) $(MSG_CLEANING)
	$(REMOVE) $(TARG_WITH_DIR).hex
	$(REMOVE) $(TARG_WITH_DIR).eep
	$(REMOVE) $(TARG_WITH_DIR).cof
	$(REMOVE) $(TARG_WITH_DIR).elf
#	$(REMOVE) $(TARG_WITH_DIR).map
	$(REMOVE) $(TARG_WITH_DIR).a90
	$(REMOVE) $(TARG_WITH_DIR).sym
	$(REMOVE) $(TARG_WITH_DIR).lnk
	$(REMOVE) $(TARG_WITH_DIR).lss
	$(REMOVE) $(TARG_WITH_DIR).mot
	$(REMOVE) $(TARG_WITH_DIR).bin
	$(REMOVE) $(TARGET_BT)
	$(REMOVE) $(OBJ)
	$(REMOVE) $(LST)
	$(REMOVE) $(CLIB)
	$(REMOVEDIR) ./doxygen
	$(REMOVE) $(RELEASE_M01_TARGET)
	$(REMOVE) $(RELEASE_M02_TARGET1)
	$(REMOVE) $(RELEASE_M02_TARGET2)
	$(REMOVE) $(RELEASE_M05_TARGET)


# Rule to clean up intermediate parts of build and leave output binary, ready for release
clean_release:
	@$(ECHO)
	@$(ECHO) $(MSG_CLEANING)
	$(REMOVE) $(TARG_WITH_DIR).hex
	$(REMOVE) $(TARG_WITH_DIR).eep
	$(REMOVE) $(TARG_WITH_DIR).cof
	$(REMOVE) $(TARG_WITH_DIR).elf
#	$(REMOVE) $(TARG_WITH_DIR).map
	$(REMOVE) $(TARG_WITH_DIR).a90
	$(REMOVE) $(TARG_WITH_DIR).sym
	$(REMOVE) $(TARG_WITH_DIR).lnk
	$(REMOVE) $(TARG_WITH_DIR).lss
	$(REMOVE) $(TARG_WITH_DIR).mot
	$(REMOVE) $(TARG_WITH_DIR).bin
	$(REMOVE) $(TARGET_BT)
	$(REMOVE) $(OBJ)
	$(REMOVE) $(LST)
	$(REMOVE) $(CLIB)
	$(REMOVEDIR) doxygen


# Listing of phony targets.
.PHONY : all begin doxygen createdirs finish end ccversion \
         build clean clean_all clean_release release_files info release_info checksize
