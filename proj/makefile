LD = g++

COM_FLAGS = -g -Iinclude

CFLAGS = $(COM_FLAGS)
CXXFLAGS = $(COM_FLAGS) -std=c++23
LDFLAGS = $(COM_FLAGS)

TASKS = $(filter-out include,$(patsubst %/,%,$(wildcard */)))
TASKS_OUT = $(patsubst %,%.out,$(TASKS))

get_o_files = $(addsuffix .o, $(basename $(wildcard $1/*.c $1/*.cpp include/*.c include/*.cpp)))
OBJS = $(strip $(foreach t,$(TASKS),$(call get_o_files,$(t))))

INCLUDES = $(wildcard include/*.h include/*.hpp)

.PHONY: all clean

default:
	@printf "Usage: make [task]\n"

$(TASKS): %: %.out

.SECONDEXPANSION:

$(TASKS_OUT): %.out: $$(call get_o_files,$$*)
	$(LD) $(LDFLAGS) -o $@ $^

$(OBJS): %: $(INCLUDES)

clean:
	rm -fv $(TASKS_OUT) $(OBJS)
