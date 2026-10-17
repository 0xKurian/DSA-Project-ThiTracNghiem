CXX ?= g++
CPPFLAGS ?= -Ithird_party/raylib-6.2_linux_amd64/include -Icore -Ibusiness -Iui
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -g
LDFLAGS ?= -Lthird_party
LDLIBS ?= third_party/raylib-6.2_linux_amd64/lib/libraylib.a -lGL -lm -lpthread -ldl -lrt -lX11
BUILD_DIR ?= build

CORE_SRC := $(wildcard core/*.cpp)
CORE_OBJ := $(patsubst core/%.cpp,$(BUILD_DIR)/core/%.o,$(CORE_SRC))

BUSINESS_SRC := $(wildcard business/*.cpp)
BUSINESS_OBJ := $(patsubst business/%.cpp,$(BUILD_DIR)/business/%.o,$(BUSINESS_SRC))

UI_SRC := $(wildcard ui/*.cpp)
UI_OBJ := $(patsubst ui/%.cpp,$(BUILD_DIR)/ui/%.o,$(UI_SRC))

ALL_OBJS := $(CORE_OBJ) $(BUSINESS_OBJ) $(UI_OBJ)

TEST_NAMES := $(basename $(notdir $(wildcard tests/*.cpp)))
TEST_BINS := $(addprefix $(BUILD_DIR)/tests/,$(TEST_NAMES))
DEPS := $(ALL_OBJS:.o=.d) $(TEST_BINS:=.d) $(BUILD_DIR)/core_demo.d $(BUILD_DIR)/app.d

.PHONY: all app demo test run run-gui run-cli clean
all: $(BUILD_DIR)/app $(BUILD_DIR)/core_demo $(TEST_BINS)
app: $(BUILD_DIR)/app
demo: $(BUILD_DIR)/core_demo

$(BUILD_DIR)/core/%.o: core/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/business/%.o: business/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/ui/%.o: ui/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/app: main.cpp $(ALL_OBJS)
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -MF $@.d $< $(ALL_OBJS) $(LDFLAGS) $(LDLIBS) -o $@

$(BUILD_DIR)/core_demo: examples/core_demo.cpp $(CORE_OBJ)
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -MF $@.d $< $(CORE_OBJ) -o $@

$(BUILD_DIR)/tests/%: tests/%.cpp $(CORE_OBJ)
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -MF $@.d $< $(CORE_OBJ) -o $@

run: $(BUILD_DIR)/app
	./$(BUILD_DIR)/app

run-gui: $(BUILD_DIR)/app
	./$(BUILD_DIR)/app

run-cli: $(BUILD_DIR)/app
	./$(BUILD_DIR)/app --console

test: $(TEST_BINS)
	@set -e; for test_bin in $(TEST_BINS); do echo "Running $$test_bin"; "$$test_bin"; done
	$(BUILD_DIR)/tests/test_core_contracts null

clean:
	rm -rf -- $(BUILD_DIR)

-include $(DEPS)
