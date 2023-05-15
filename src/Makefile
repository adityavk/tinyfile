CXX := g++
UNAME_S := $(shell uname -s)

# Dirs
SRCDIR := sources
INCDIR := include
BINDIR := ../bin
BUILDDIR := $(BINDIR)/build
SERVICE_DIR_NAME := service
LIB_DIR_NAME := lib
COMMON_DIR_NAME := common
CLIENT_DIR_NAME := client
LIB_HEADERS_OUT_DIR=$(BINDIR)/tiny-file

THIRD_PARTY_DIR := third-party
THIRD_PARTY_BUILD_DIR := $(BUILDDIR)/$(THIRD_PARTY_DIR)
SNAPPY_DIR_NAME := snappy-c
SNAPPY_SRC_DIR := $(THIRD_PARTY_DIR)/$(SNAPPY_DIR_NAME)
SNAPPY_BUILD_DIR := $(THIRD_PARTY_BUILD_DIR)/$(SNAPPY_DIR_NAME)

# Targets
SERVICE_NAME := TFService
LIB_NAME := libTinyFile.a
CLIENT_NAME := TFClient
TF_SERVICE := $(BINDIR)/$(SERVICE_NAME)
TF_LIB := $(BINDIR)/$(LIB_NAME)
TF_CLIENT := $(BINDIR)/$(CLIENT_NAME)

# Code Lists
SERVICE_SOURCES=$(wildcard $(SRCDIR)/$(SERVICE_DIR_NAME)/*.cpp)
LIB_SOURCES=$(wildcard $(SRCDIR)/$(LIB_DIR_NAME)/*.cpp)
COMMON_SOURCES=$(wildcard $(SRCDIR)/$(COMMON_DIR_NAME)/*.cpp)
CLIENT_SOURCES=$(wildcard $(SRCDIR)/$(CLIENT_DIR_NAME)/*.cpp)

SERVICE_OBJECTS=$(SERVICE_SOURCES:$(SRCDIR)/%.cpp=$(BUILDDIR)/%.o)
LIB_OBJECTS=$(LIB_SOURCES:$(SRCDIR)/%.cpp=$(BUILDDIR)/%.o)
COMMON_OBJECTS=$(COMMON_SOURCES:$(SRCDIR)/%.cpp=$(BUILDDIR)/%.o)
CLIENT_OBJECTS=$(CLIENT_SOURCES:$(SRCDIR)/%.cpp=$(BUILDDIR)/%.o)

SERVICE_HEADERS=$(wildcard $(INCDIR)/$(SERVICE_DIR_NAME)/*.h)
LIB_HEADERS=$(wildcard $(INCDIR)/$(LIB_DIR_NAME)/*.h)
LIB_PVT_HEADERS=$(wildcard $(INCDIR)/$(LIB_DIR_NAME)-private/*.h)
CLIENT_HEADERS=$(wildcard $(INCDIR)/$(CLIENT_DIR_NAME)/*.h)
COMMON_HEADERS=$(wildcard $(INCDIR)/$(COMMON_DIR_NAME)/*.h)

# Dir Lists
INCDIRS := $(shell find include/**/* -name '*.h' -exec dirname {} \; | sort | uniq)
INCLIST := $(patsubst include/%,-I include/%,$(INCDIRS))
THIRD_PARTY_INC := -I $(THIRD_PARTY_DIR)

# Shared Compiler Flags
INC := -I $(INCDIR) -I /usr/local/include
LIB := -L /usr/local/lib
DEBUG_LOGS := 0
CXXFLAGS := -c -Wall -Wextra -std=c++17 -O3 -DDEBUG=$(DEBUG_LOGS)
CLIENT_LD_FLAGS = -L$(BINDIR) -lTinyFile
CLIENT_INCLIST= -I /usr/local/include -I $(BINDIR) $(THIRD_PARTY_INC) -I $(INCDIR)/$(CLIENT_DIR_NAME) -I $(INCDIR)/$(COMMON_DIR_NAME)

# Platform Specific Compiler Flags
ifeq ($(UNAME_S),Linux)
    LIB += -lpthread
    CLIENT_LD_FLAGS += -lstdc++fs
endif

.PHONY: all clean $(TF_SERVICE) $(TF_LIB) $(TF_CLIENT)

all: $(TF_SERVICE) $(TF_LIB) $(TF_CLIENT)

clean:
	@echo "Cleaning $(BUILDDIR) and $(BINDIR) artifacts...";
	$(RM) -r $(BUILDDIR)
	$(RM) -r $(LIB_HEADERS_OUT_DIR)
	$(RM) $(TF_SERVICE)
	$(RM) $(TF_LIB)
	$(RM) $(TF_CLIENT)
	$(RM) -r $(BINDIR)/output

$(TF_SERVICE): $(SERVICE_OBJECTS) $(COMMON_OBJECTS) $(SNAPPY_BUILD_DIR)/snappy.o | $(BINDIR)
	@echo "  Linking $(TF_SERVICE)...";
	$(CXX) $^ -o $(TF_SERVICE) $(LIB)

$(TF_LIB): $(LIB_OBJECTS) $(COMMON_OBJECTS) | $(BINDIR) $(LIB_HEADERS_OUT_DIR)
	@echo "Linking $(TF_LIB)...";
	ar rcs $@ $^
	cp -aR $(INCDIR)/$(LIB_DIR_NAME)/* $(LIB_HEADERS_OUT_DIR)

$(TF_CLIENT): $(CLIENT_OBJECTS) $(SNAPPY_BUILD_DIR)/snappy.o | $(BINDIR)
	@echo "  Linking $(TF_CLIENT)...";
	$(CXX) $^ -o $(TF_CLIENT) $(LIB) $(CLIENT_LD_FLAGS)

$(BUILDDIR)/$(SERVICE_DIR_NAME)/%.o: $(SRCDIR)/$(SERVICE_DIR_NAME)/%.cpp $(SERVICE_HEADERS) $(COMMON_HEADERS) | $(BUILDDIR)/$(SERVICE_DIR_NAME)
	@echo "Compiling $<..."; $(CXX) $(CXXFLAGS) $(INC) $(THIRD_PARTY_INC) -o $@ $<

$(BUILDDIR)/$(LIB_DIR_NAME)/%.o: $(SRCDIR)/$(LIB_DIR_NAME)/%.cpp $(LIB_HEADERS) $(LIB_PVT_HEADERS) $(COMMON_HEADERS) | $(BUILDDIR)/$(LIB_DIR_NAME)
	@echo "Compiling $<..."; $(CXX) $(CXXFLAGS) $(INC) -o $@ $<

$(BUILDDIR)/$(COMMON_DIR_NAME)/%.o: $(SRCDIR)/$(COMMON_DIR_NAME)/%.cpp $(COMMON_HEADERS) | $(BUILDDIR)/$(COMMON_DIR_NAME)
	@echo "Compiling $<..."; $(CXX) $(CXXFLAGS) $(INC) -o $@ $<

$(BUILDDIR)/$(CLIENT_DIR_NAME)/%.o: $(SRCDIR)/$(CLIENT_DIR_NAME)/%.cpp $(CLIENT_HEADERS) $(LIB_HEADERS) | $(BUILDDIR)/$(CLIENT_DIR_NAME)
	@echo "Compiling $<..."; $(CXX) $(CXXFLAGS) $(CLIENT_INCLIST) -o $@ $<

$(SNAPPY_BUILD_DIR)/snappy.o: $(SNAPPY_SRC_DIR)/snappy.c | $(SNAPPY_BUILD_DIR)
	@echo "Compiling $<..."; $(CC) -Wall -g -O2 -DNDEBUG=1  -DSG=1 -fPIC -c -o $@ $<

$(BUILDDIR)/$(SERVICE_DIR_NAME):
	@mkdir -p $(BUILDDIR)/$(SERVICE_DIR_NAME)

$(BUILDDIR)/$(LIB_DIR_NAME):
	@mkdir -p $(BUILDDIR)/$(LIB_DIR_NAME)

$(BUILDDIR)/$(COMMON_DIR_NAME):
	@mkdir -p $(BUILDDIR)/$(COMMON_DIR_NAME)

$(BUILDDIR)/$(CLIENT_DIR_NAME):
	@mkdir -p $(BUILDDIR)/$(CLIENT_DIR_NAME)

$(THIRD_PARTY_BUILD_DIR):
	@mkdir -p $(THIRD_PARTY_BUILD_DIR)

$(SNAPPY_BUILD_DIR): $(THIRD_PARTY_BUILD_DIR)
	@mkdir -p $(SNAPPY_BUILD_DIR)

$(BINDIR):
	@mkdir -p $(BINDIR)

$(LIB_HEADERS_OUT_DIR):
	@mkdir -p $(LIB_HEADERS_OUT_DIR)
