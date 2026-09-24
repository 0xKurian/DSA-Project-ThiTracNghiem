CXX ?= g++
CPPFLAGS ?=
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -g
LDFLAGS ?=
LDLIBS ?=
BUILD_DIR ?= build

CORE_SRC := $(wildcard core/*.cpp)
CORE_OBJ := $(patsubst core/%.cpp,$(BUILD_DIR)/core/%.o,$(CORE_SRC))
TEST_NAMES := $(basename $(notdir $(wildcard tests/*.cpp)))
TEST_BINS := $(addprefix $(BUILD_DIR)/tests/,$(TEST_NAMES))
DEPS := $(CORE_OBJ:.o=.d) $(TEST_BINS:=.d) $(BUILD_DIR)/core_demo.d

.PHONY: all demo test clean
all: $(BUILD_DIR)/core_demo $(TEST_BINS)
demo: $(BUILD_DIR)/core_demo

$(BUILD_DIR)/core/%.o: core/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/core_demo: examples/core_demo.cpp $(CORE_OBJ)
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -MF $@.d $< $(CORE_OBJ) $(LDFLAGS) $(LDLIBS) -o $@

$(BUILD_DIR)/tests/%: tests/%.cpp $(CORE_OBJ)
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -MF $@.d $< $(CORE_OBJ) $(LDFLAGS) $(LDLIBS) -o $@

test: $(TEST_BINS)
	@set -e; for test_bin in $(TEST_BINS); do echo "Running $$test_bin"; "$$test_bin"; done
	$(BUILD_DIR)/tests/test_core_contracts null

clean:
	rm -rf -- $(BUILD_DIR)

-include $(DEPS)
