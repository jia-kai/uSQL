# $File: Makefile
# $Date: Wed Nov 05 20:08:42 2014 +0800
# $Author: jiakai <jia.kai66@gmail.com>

BUILD_DIR = build
TARGET = usql
override ARGS ?= 

CXX = g++ -std=c++1y
BISON ?= bison
FLEX ?= flex

SRC_EXT = cpp

override OPTFLAG ?= -O2

override CXXFLAGS += \
	-ggdb \
	-Wall -Wextra -Wnon-virtual-dtor -Wno-unused-parameter -Winvalid-pch \
	-Wno-deprecated-register \
	$(CPPFLAGS) $(OPTFLAG)
override V ?= @

CXXSOURCES = $(shell find -L src -name "*.$(SRC_EXT)")
OBJS = $(addprefix $(BUILD_DIR)/,$(CXXSOURCES:.$(SRC_EXT)=.o))
OBJS += $(BUILD_DIR)/src/usql/parser/sql.tab.o
OBJS += $(BUILD_DIR)/src/usql/parser/sql.yy.o
DEPFILES = $(OBJS:.o=.d)


all: $(TARGET)

-include $(DEPFILES)


$(BUILD_DIR)/%.o: %.$(SRC_EXT)
	@echo "[cxx] $< ..."
	@mkdir -pv $(dir $@)
	@$(CXX) $(CPPFLAGS) -MM -MT "$@" "$<"  > "$(@:.o=.d)"
	$(V)$(CXX) -c $< -o $@ $(CXXFLAGS)

src/usql/parser/sql.tab.hpp: src/usql/parser/sql.yy
parser: src/usql/parser/sql.tab.$(SRC_EXT) src/usql/parser/sql.yy.$(SRC_EXT)

src/usql/parser/sql.tab.$(SRC_EXT): src/usql/parser/sql.yy
	@echo "[bison] sql.tab"
	$(V)$(BISON) -d $< -o $@

src/usql/parser/sql.yy.$(SRC_EXT): src/usql/parser/sql.l
	@echo "[flex] sql.l"
	$(V)$(FLEX) -d -o $@ $<


$(TARGET): $(OBJS)
	@echo "Linking ..."
	$(V)$(CXX) $^ -o $@ -lpthread $(LDFLAGS)

clean:
	rm -rf $(BUILD_DIR)/src $(TARGET) 
	rm -rf src/usql/parser/sql.tab.$(SRC_EXT)
	rm -rf src/usql/parser/sql.yy.$(SRC_EXT)

clean-full:
	rm -rf $(BUILD_DIR) $(TARGET)

run: $(TARGET)
	./$(TARGET) $(ARGS) 2>data/test_log

gdb: 
	OPTFLAG=-O0 make -j4
	gdb --args $(TARGET) $(ARGS)

git:
	git add -A
	git commit -a

gprof:
	OPTFLAG='-O3 -pg' LDFLAGS=-pg make -j4

.SUFFIXES:

.PHONY: all clean clean-full run gdb git gprof parser scanner

# vim: ft=make

