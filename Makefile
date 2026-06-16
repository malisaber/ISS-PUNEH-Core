.DEFAULT_GOAL := all

SHELL := powershell.exe
.SHELLFLAGS := -NoProfile -ExecutionPolicy Bypass -Command

PROJECT := puneh-iss
SRC_DIR := src
INC_DIR := include
BUILD_DIR := build
BIN_DIR := bin
TARGET := $(BIN_DIR)/$(PROJECT).exe

VCPKG_ROOT ?= .deps/vcpkg
VCPKG_TRIPLET ?= x64-windows
SYSTEMC_ROOT ?= $(VCPKG_ROOT)/installed/$(VCPKG_TRIPLET)

CXX ?= g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -DSC_INCLUDE_DYNAMIC_PROCESSES -I$(INC_DIR) -I$(SYSTEMC_ROOT)/include
LDFLAGS := -L$(SYSTEMC_ROOT)/lib
LDLIBS := -lsystemc

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

all: $(TARGET)

run: $(TARGET)
	& "$(TARGET)"

install-deps: $(DEPS_STAMP)

$(DEPS_STAMP):
	if (-not (Test-Path "$(VCPKG_ROOT)")) { git clone --depth 1 https://github.com/microsoft/vcpkg "$(VCPKG_ROOT)" }
	& "$(VCPKG_ROOT)/bootstrap-vcpkg.bat"
	& "$(VCPKG_ROOT)/vcpkg.exe" install systemc:$(VCPKG_TRIPLET)
	New-Item -ItemType File -Force -Path "$@" | Out-Null

$(BUILD_DIR):
	New-Item -ItemType Directory -Force -Path "$@" | Out-Null

$(BIN_DIR):
	New-Item -ItemType Directory -Force -Path "$@" | Out-Null

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp $(DEPS_STAMP) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c "$<" -o "$@"

$(TARGET): $(OBJECTS) | $(BIN_DIR)
	$(CXX) $(OBJECTS) $(LDFLAGS) $(LDLIBS) -o "$@"

clean:
	Remove-Item -Recurse -Force "$(BUILD_DIR)" -ErrorAction SilentlyContinue
	Remove-Item -Recurse -Force "$(BIN_DIR)" -ErrorAction SilentlyContinue

.PHONY: all run install-deps clean
