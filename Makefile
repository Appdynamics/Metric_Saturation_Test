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
SATURATIONTEST_TEST_EXE = "saturationtest"

# CCP	    = g++
#CPPFLAGS   = -g -std=c++11
CC 	    = cc
CFLAGS      = -g 

# includes
SDK_INC     = -I$(SDK_INC_DIR)

LDFLAGS         = -lz -lappdynamics -ldl -lpthread -lrt
SDK_LOAD_PATH	= LD_LIBRARY_PATH=$(SDK_LIB_DIR)

# Citadel basic params: at a minimum you must provide the host, port, account, and key fields.
# Everything else will default to the values set in the code.
# 
#CONTROLLER_HOST=4.2.2.2
#CONTROLLER_PORT=8080
#CONTROLLER_ACCESS=abcde12345
#CONTROLLER_ACCOUNT=customer1

SATURATIONTEST_EXE_OPTS =  $(CONTROLLER_HOST)
SATURATIONTEST_EXE_OPTS += $(CONTROLLER_PORT)
SATURATIONTEST_EXE_OPTS += $(CONTROLLER_ACCESS)
SATURATIONTEST_EXE_OPTS += $(CONTROLLER_ACCOUNT)

all:	$(SATURATIONTEST_TEST_EXE)

.PHONY: clean run params help

help:
	@echo "\nThis program will generate massive amounts of custom metrics."
	@echo "It does so via the AppDynamics C/C++ SDK, and some not so clever code."
	@echo "To execute it, run \"make\" with no params to get the default behavior."
	@echo "The default behavior is to create 1000 custom metrics, once a minute, for 100 minutes."
	@echo "Or, run \"make once\" to have it generate only 1 custom metric only once."
	@echo "  that can be useful to just see what the actual paramters used will be."
	@echo "Or, run \"make lots\" to have it generate 10000 custom metrics for 100 minutes."
	@echo
	@echo "You must set the following environment variables beforehand!"
	@echo "------------------------------------------------------------"
	@echo "1) CONTROLLER_HOST    - the URL or ipaddr of the Controller."
	@echo "2) CONTROLLER_PORT    - the port address of the Controller."
	@echo "3) CONTROLLER_ACCESS  - the access key for the Controller."
	@echo "4) CONTROLLER_ACCOUNT - the account name (typically \"customer1\")."
	@echo 
	@echo "The parameters must be passsed as parameters 1 through 4 on the program command line."
	@echo "When you run the program, it will create the \"Metric_Saturation_Test_App\" Application"
	@echo "with a Tier called \"Metric_Saturation_Tier\", and within it two Nodes, the first is"
	@echo "called \"Default_Node\", and it will register some number of BTs to it when the test is"
	@echo "running, indicating that load is being generated.   The second is called \"Sauturation_Node\""
	@echo "and it will show the Custom Metrics."
	@echo
	@echo "The program, as it runs each cycle, will also generate some number of BTs for both the"
	@echo "Default and Saturation Nodes, once per minute/cycle.   It generates up to 4 different BT"
	@echo "names for the Default Node, and up to 32 different BT names for the Saturation Node".
	@echo
	@echo "To view metrics, after the test has fully run, use the Metric Browser, then select"
	@echo "the \"Application Infrastructure Performance\" entry, then expand the \"Metric_Saturation_Tier\","
	@echo "then expand the \"Custom Metrics\" entry, which will list all of the metric names.  Each"
	@echo "metric will have a name of the form \"Metric______N\" where N represents a value equal to the"
	@echo "metric, and the value associated with it."
	@echo
	@echo "The test takes two other parameters, the number of cycles to execute, and the number of metrics"
	@echo "to create, as parameters 5 and 6.   The default values are 100 and 1000, respectively.   Each"
	@echo "cycle will produce some numner of metrics, and then sleep for 1 minute.  The idea is that the"
	@echo "underpinning libagent logic, which writes to the Controller on 1 minute boundaries, will continually"
	@echo "be sending metrics and therefore fully taxing the Controller."
	@echo 
	@echo "Note: There isn't any known way to delete a custom metric once created, via the Controller U/I."
	@echo "If you discover one, please notify the author, and update this Makefile help message."
	@echo 

clean:
	@echo "cleaning everything..."
	rm -rf $(SATURATIONTEST_TEST_EXE)
	rm -f /tmp/appd/*

params:
	@echo 
	@echo "Explicitly set options" \" " $(SATURATIONTEST_EXE_OPTS) " \"
	@echo 

$(SATURATIONTEST_TEST_EXE): saturationtest.c Makefile
	rm -f $@
	$(CC) $(CFLAGS) $(SDK_INC) $< -L${SDK_LIB_DIR} $(LDFLAGS) -o $@ 

once:	$(SATURATIONTEST_TEST_EXE)
	$(SDK_LOAD_PATH) ./$(SATURATIONTEST_TEST_EXE) ${SATURATIONTEST_EXE_OPTS} 1 1 1

some:	$(SATURATIONTEST_TEST_EXE)
	$(SDK_LOAD_PATH) ./$(SATURATIONTEST_TEST_EXE) ${SATURATIONTEST_EXE_OPTS} 10 20 3

lots:	$(SATURATIONTEST_TEST_EXE)
	$(SDK_LOAD_PATH) ./$(SATURATIONTEST_TEST_EXE) $(SATURATIONTEST_EXE_OPTS) 1000 1000 100
