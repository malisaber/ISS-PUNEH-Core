.DEFAULT_GOAL := all

PROJECT := puneh-iss
SRC_DIR := src
INC_DIR := include
BUILD_DIR := build
BIN_DIR := bin
VCPKG_ROOT ?= .deps/vcpkg
ifeq ($(OS),Windows_NT)
VCPKG_TRIPLET ?= x64-windows
else
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
VCPKG_TRIPLET ?= x64-osx
else
VCPKG_TRIPLET ?= x64-linux
endif
endif
SYSTEMC_ROOT ?= $(VCPKG_ROOT)/installed/$(VCPKG_TRIPLET)

SOURCES := \
	$(SRC_DIR)/main.cpp \
	$(SRC_DIR)/Utilities.cpp \
	$(SRC_DIR)/channel.cpp \
	$(SRC_DIR)/memory.cpp \
	$(SRC_DIR)/memory_controller.cpp \
	$(SRC_DIR)/PUNEH.cpp \
	$(SRC_DIR)/RF.cpp \
	$(SRC_DIR)/SemiHosting.cpp \
	$(SRC_DIR)/system.cpp \
	$(SRC_DIR)/USART.cpp

OBJECTS := $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
DEPS_STAMP := $(BUILD_DIR)/.deps-installed

ifeq ($(origin SYSTEMC_ROOT),default)
USE_VCPKG := 1
else
USE_VCPKG := 0
endif

ifeq ($(OS),Windows_NT)
SHELL := powershell.exe
.SHELLFLAGS := -NoProfile -ExecutionPolicy Bypass -Command

TARGET := $(BIN_DIR)/$(PROJECT).exe

BOOTSTRAP_SCRIPT := bootstrap-vcpkg.bat
VCPKG_BIN := vcpkg.exe
MAKE_DIR = New-Item -ItemType Directory -Force -Path "$(1)" | Out-Null
MAKE_FILE = New-Item -ItemType File -Force -Path "$(1)" | Out-Null
REMOVE_DIR = Remove-Item -Recurse -Force "$(1)" -ErrorAction SilentlyContinue
VCPKG_CLONE = if (-not (Test-Path "$(VCPKG_ROOT)")) { git clone --depth 1 https://github.com/microsoft/vcpkg "$(VCPKG_ROOT)" }
VCPKG_BOOTSTRAP = & "$(VCPKG_ROOT)/$(BOOTSTRAP_SCRIPT)"
VCPKG_INSTALL = & "$(VCPKG_ROOT)/$(VCPKG_BIN)" install systemc:$(VCPKG_TRIPLET)
RUN_TARGET = & "$(TARGET)"
else
SHELL := /bin/sh

TARGET := $(BIN_DIR)/$(PROJECT)

BOOTSTRAP_SCRIPT := bootstrap-vcpkg.sh
VCPKG_BIN := vcpkg
MAKE_DIR = mkdir -p "$(1)"
MAKE_FILE = touch "$(1)"
REMOVE_DIR = rm -rf "$(1)"
VCPKG_CLONE = if [ ! -d "$(VCPKG_ROOT)" ]; then git clone --depth 1 https://github.com/microsoft/vcpkg "$(VCPKG_ROOT)"; fi
VCPKG_BOOTSTRAP = cd "$(VCPKG_ROOT)" && ./$(BOOTSTRAP_SCRIPT)
VCPKG_INSTALL = "$(VCPKG_ROOT)/$(VCPKG_BIN)" install systemc:$(VCPKG_TRIPLET)
RUN_TARGET = "$(TARGET)"
endif

CXX ?= g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -DSC_INCLUDE_DYNAMIC_PROCESSES -I"$(INC_DIR)" -I"$(SYSTEMC_ROOT)/include"
LDFLAGS := -L"$(SYSTEMC_ROOT)/lib"
LDLIBS := -lsystemc

all: $(TARGET)

run: $(TARGET)
	$(RUN_TARGET)

ifeq ($(USE_VCPKG),1)
install-deps: $(DEPS_STAMP)
else
install-deps:
	@echo "SYSTEMC_ROOT is set to $(SYSTEMC_ROOT); skipping vcpkg bootstrap."
endif

$(DEPS_STAMP): | $(BUILD_DIR)
	$(VCPKG_CLONE)
	$(VCPKG_BOOTSTRAP)
	$(VCPKG_INSTALL)
	$(call MAKE_FILE,$@)

$(BUILD_DIR):
	$(call MAKE_DIR,$@)

$(BIN_DIR):
	$(call MAKE_DIR,$@)

ifeq ($(USE_VCPKG),1)
BUILD_DEP := $(DEPS_STAMP)
else
BUILD_DEP :=
endif

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp $(BUILD_DEP) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c "$<" -o "$@"

$(TARGET): $(OBJECTS) | $(BIN_DIR)
	$(CXX) $(OBJECTS) $(LDFLAGS) $(LDLIBS) -o "$@"

clean:
	$(call REMOVE_DIR,$(BUILD_DIR))
	$(call REMOVE_DIR,$(BIN_DIR))

.PHONY: all run install-deps clean
