LD = g++

-include */*.d
.PRECIOUS: */*.o

CXXFLAGS = -g -Ivn -MD -std=c++23 -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Wold-style-cast -Wnrvo
LDFLAGS = -g

VNOBJS = $(addsuffix .o, $(basename $(wildcard vn/*.cpp)))
ONOBJS = $(wildcard on/*.py)

.PHONY: on vn clean

default:
	@printf "Usage: make [task]\n"

vn: vn.out
	./vn.out $(FLAGS)

.SECONDEXPANSION:

vn.out: $(VNOBJS)
	$(LD) $(LDFLAGS) -o $@ $^

on:
	python3 on/on.py $(FLAGS)

clean:
	rm -rfv $(TASKS_OUT) */*.o */*.d on/__pycache__
