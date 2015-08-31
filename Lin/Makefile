EXECUTABLE    := hyperspect_bfgsb_CL

CXXFILES      := main.cpp time_util.cpp hyperspect_bfgsb_cl.cpp hyperspect.cpp bfgsb_cl.cpp parallel_eval.cpp solver.cpp coarse_grain.cpp yexp_calc_cl.cpp
FFILES        := lbfgsb.f

# Basic directory setup
ROOTDIR     ?= .
SRCDIR      ?= src
OBJDIR      ?= obj
DEPDIR      ?= dep
BINDIR      ?= .

#ROOTBINDIR ?= bin
#ROOTBINDIR  ?= .
#BINDIR     ?= $(ROOTBINDIR)/linux

# Compilers
CC         := g++
CXX        := g++
FC         := gfortran

# Linker
LD         := g++

# Includes
INCLUDES  += -I/usr/local/cuda/include
#INCLUDES  += -I. -I$(ATISTREAMSDKROOT)include

# Libraries
LIB_PATHS  :=
LIBS       += $(LIB_PATHS) 
LIBS       += -lm -lpthread -lgfortran -lOpenCL

# Warning flags
CWARN_FLAGS :=
CXXWARN_FLAGS :=
FWARN_FLAGS :=
LDWARN_FLAGS :=

CFLAGS    :=  $(CWARN_FLAGS) $(INCLUDES)
CXXFLAGS  :=  $(CXXWARN_FLAGS) $(INCLUDES)
FFLAGS    :=  $(FWARN_FLAGS) $(INCLUDES)
LDFLAGS   :=  $(LDWARN_FLAGS)

# Architecture specific flags
TARGET_ARCH := -m64 -march=core2 -msse3 -mfpmath=sse 

# Compiler specific general flags
CFLAGS  		+=
CXXFLAGS  	+=
FFLAGS      +=

# Linker flags
LDFLAGS      += 

# Compiler specific optimization flags

# Common optimization flags
COMMONOPT_FLAGS := -O3

# Common flags for compilers and linker
COMMONFLAGS = $(TARGET_ARCH)

# Turn either debugging information or compiler optimization on
ifeq ($(debug),1)
   COMMONFLAGS += -g -D__DEBUG__
else
   COMMONFLAGS += $(COMMONOPT_FLAGS)
endif

# Turn profile information on
ifeq ($(profile),1)
   COMMONFLAGS += -pg
endif
     

TARGETDIR := $(BINDIR)
TARGET    := $(TARGETDIR)/$(EXECUTABLE)

################################################################################
# Check for input flags and set compiler flags appropriately
################################################################################

# Add common flags
CFLAGS    += $(COMMONFLAGS)
CXXFLAGS  += $(COMMONFLAGS)
FFLAGS    += $(COMMONFLAGS)
LDFLAGS   += $(COMMONFLAGS)

################################################################################
# Set up object files
################################################################################

#OBJDIR := $(ROOTOBJDIR)/$(BINSUBDIR)
OBJS :=  $(patsubst %.c,$(OBJDIR)/%.c.o,$(notdir $(CFILES)))
OBJS +=  $(patsubst %.cpp,$(OBJDIR)/%.cpp.o,$(notdir $(CXXFILES)))
OBJS +=  $(patsubst %.f,$(OBJDIR)/%.f.o,$(notdir $(FFILES)))

################################################################################
# Set up dependency files
################################################################################

DEPS :=  $(patsubst %.c,$(DEPDIR)/%.c.d,$(notdir $(CFILES)))
DEPS +=  $(patsubst %.cpp,$(DEPDIR)/%.cpp.d,$(notdir $(CXXFILES)))

################################################################################
# Rules
################################################################################

.PHONY : clean

$(TARGET) : $(OBJS) Makefile
	$(LD) $(LDFLAGS) $(OBJS) $(LIBS) -o $(TARGET) 

$(OBJS) : | $(OBJDIR) $(DEPDIR)

$(OBJDIR)/%.c.o : $(SRCDIR)%.c  Makefile
	$(CC) -MMD -MP -MF $(DEPDIR)/$(notdir $<.d) $(CFLAGS) -c $< -o $@ 

$(OBJDIR)/%.cpp.o : $(SRCDIR)/%.cpp  Makefile
	$(CXX) -MMD -MP -MF $(DEPDIR)/$(notdir $<.d) $(CXXFLAGS) -c $< -o $@ 

$(OBJDIR)/%.f.o : $(SRCDIR)/%.f Makefile
	$(FC) $(FFLAGS) -c $< -o $@ 

$(OBJDIR) : 
	@mkdir -p $(OBJDIR)

$(DEPDIR) :
	@mkdir -p $(DEPDIR)

clean:
	$(VERBOSE)rm -rf $(OBJDIR)
	$(VERBOSE)rm -rf $(DEPDIR)
	$(VERBOSE)rm -rf $(TARGET)

-include $(DEPS)


