# r tab stops set to 4
#
# zlib					[sudo apt-get install zlib1g-dev]
#
# Makefile to create Metric Saturation Loads using the libagent based AppDynamics C/C++SDK

# NOTE: You must ceate a symbolic link in the .../extlibs directory called appdynamics-cpp-sdk that
# points to the latest version of the AppDynamics C++ SDK install.

SDK_ROOT	= ./extlibs/appdynamics-cpp-sdk
SDK_INC_DIR	= $(SDK_ROOT)/include
SDK_LIB_DIR	= $(SDK_ROOT)/lib

# The derived executable
SATURATIONTEST_TEST_EXE = saturationtest

# CCP	    = g++
#CPPFLAGS   = -g -std=c++11
CC 	    = cc
CFLAGS      = -g 

# includes
SDK_INC     = -I$(SDK_INC_DIR)

LDFLAGS         = -lz -lappdynamics -ldl 
SDK_LOAD_PATH	= LD_LIBRARY_PATH=$(SDK_LIB_DIR)

# Citadel basic params: at a minimum you must provide the host, port, account, and key fields.
# Everything else will default to the values set in the code.
# 
CONTROLLER_HOST=4.2.2.2
CONTROLLER_PORT=8080
CONTROLLER_ACCESS=abcde12345
CONTROLLER_ACCOUNT=customer1

SATURATIONTEST_EXE_OPTS =  $(CONTROLLER_HOST)
SATURATIONTEST_EXE_OPTS += $(CONTROLLER_PORT)
SATURATIONTEST_EXE_OPTS += $(CONTROLLER_ACCESS)
SATURATIONTEST_EXE_OPTS += $(CONTROLLER_ACCOUNT)

all:	$(SATURATIONTEST_TEST_EXE)

.PHONY: clean run params

clean:
	@echo "cleaning everything..."
	rm -rf $(SATURATIONTEST_TEST_EXE)

params:
	@echo 
	@echo "Explicitly set options" \" " $(SATURATIONTEST_EXE_OPTS) " \"
	@echo 

$(SATURATIONTEST_TEST_EXE): saturationtest.c Makefile
	rm -f $@
	$(CC) $(CFLAGS) $(SDK_INC) $< -L${SDK_LIB_DIR} $(LDFLAGS) -o $@

run:	$(SATURATIONTEST_TEST_EXE)
	$(SDK_LOAD_PATH) ./$(SATURATIONTEST_TEST_EXE) $(SATURATIONTEST_EXE_OPTS)
