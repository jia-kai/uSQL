# $File: Makefile
# $Date: Mon Oct 20 09:30:15 2014 +0800
# $Author: jiakai <jia.kai66@gmail.com>

BUILD_DIR = build
TARGET = usql_test
ARGS = 

CXX = g++ -std=c++1y

SRC_EXT = cpp
CPPFLAGS = 
override OPTFLAG ?= -O2

override CXXFLAGS += \
	-ggdb \
	-Wall -Wextra -Wnon-virtual-dtor -Wno-unused-parameter -Winvalid-pch \
	-Wno-unused-local-typedefs \
	$(CPPFLAGS) $(OPTFLAG)
override LDFLAGS += -lgtest_main -lgtest
override V ?= @

CXXSOURCES = $(shell find -L src -name "*.$(SRC_EXT)")
OBJS = $(addprefix $(BUILD_DIR)/,$(CXXSOURCES:.$(SRC_EXT)=.o))
DEPFILES = $(OBJS:.o=.d)


all: $(TARGET)

-include $(DEPFILES)

$(BUILD_DIR)/%.o: %.$(SRC_EXT)
	@echo "[cxx] $< ..."
	@mkdir -pv $(dir $@)
	@$(CXX) $(CPPFLAGS) -MM -MT "$@" "$<"  > "$(@:.o=.d)"
	$(V)$(CXX) -c $< -o $@ $(CXXFLAGS)


$(TARGET): $(OBJS)
	@echo "Linking ..."
	$(V)$(CXX) $(OBJS) -o $@ $(LDFLAGS)

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

run: $(TARGET)
	./$(TARGET) $(ARGS)

gdb: 
	OPTFLAG=-O0 make -j4
	gdb --args $(TARGET) $(ARGS)

git:
	git add -A
	git commit -a

gprof:
	OPTFLAG='-O3 -pg' LDFLAGS=-pg make -j4

.PHONY: all clean run gdb git gprof

# vim: ft=make

