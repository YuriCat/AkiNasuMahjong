# 
# 1. General Compiler Settings
#
CXX       = g++
CXXFLAGS  = -std=c++14 -Wall -Wextra -Wcast-qual -Wno-unused-function -Wno-sign-compare -Wno-unused-value -Wno-unused-label -Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-parameter -fno-rtti \
            -pedantic -Wno-long-long -msse4.2 -mbmi -mbmi2 -mavx2 -D__STDC_CONSTANT_MACROS -fopenmp
INCLUDES  =
LIBRARIES = -lpthread

#
# 2. Target Specific Settings
#
ifeq ($(TARGET),match)
	CXXFLAGS += -Ofast -DNDEBUG -DMINIMUM -DMATCH
        output_dir := out/match/
endif
ifeq ($(TARGET),release)
	CXXFLAGS += -Ofast -DNDEBUG -DMINIMUM
        output_dir := out/release/
endif
ifeq ($(TARGET),debug)
	CXXFLAGS += -O0 -g -ggdb -DDEBUG -DBROADCAST -D_GLIBCXX_DEBUG
        output_dir := out/debug/
endif
ifeq ($(TARGET),default)
	CXXFLAGS += -O2 -g -ggdb
        output_dir := out/default/
endif

#
# 2. Default Settings (applied if there is no target-specific settings)
#
sources      ?= $(shell ls -R src/*.cc)
sources_dir  ?= src/
objects      ?=
directories  ?= $(output_dir)

#
# 4. Public Targets
#
default release debug development profile test coverage:
	$(MAKE) TARGET=$@ preparation client fg_client mahjong_test logic_test table_converter

match:
	$(MAKE) TARGET=$@ preparation client fg_client

run-coverage: coverage
	out/coverage --gtest_output=xml

clean:
	rm -rf out/*

scaffold:
	mkdir -p out test out/data doc lib obj resource

#
# 5. Private Targets
#
preparation $(directories):
	mkdir -p $(directories)

mahjong_test :
	$(CXX) $(CXXFLAGS) -o $(output_dir)mahjong_test $(sources_dir)test/mahjong_test.cc $(LIBRARIES)

logic_test :
	$(CXX) $(CXXFLAGS) -o $(output_dir)logic_test $(sources_dir)test/logic_test.cc $(LIBRARIES)

table_converter :
	$(CXX) $(CXXFLAGS) -o $(output_dir)table_converter $(sources_dir)test/table_converter.cc $(LIBRARIES)

client :
	$(CXX) $(CXXFLAGS) -o $(output_dir)client $(sources_dir)floodgate.cpp $(sources_dir)easywsclient.cpp $(LIBRARIES)

fg_client :
	$(CXX) $(CXXFLAGS) -o $(output_dir)fg_client $(sources_dir)floodgate.cpp $(sources_dir)easywsclient.cpp $(LIBRARIES) -DFLOODGATE

-include $(dependencies)