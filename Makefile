# Make sure to run clean if you want to run seperate rule/program that contains main

project_dir 	= src src/platform examples .
src_files		= src/impl_libcook.o src/platform/osx.o
rekt			= examples/rekt.o
line			= examples/line.o
point			= examples/point.o
vert			= examples/vertices.o
main			:= main.o

cflags			= clang -D_GNU_SOURCE -std=c2x -Wall -O0 -g
includes 		= -Iinclude -Isrc/platform
linkers			= -lm
target			= out

find_obj_files 	= $(wildcard $(dir)/*.o)
obj_files		= $(foreach dir, $(project_dir), $(find_obj_files))
platform_os		= $(shell uname)

ifeq ($(platform_os), Darwin)
	linkers	+= -framework OpenGL -framework Foundation -framework AppKit
endif

.PHONY		: clean

exp			: $(main)
			$(MAKE) run

rekt		: $(rekt)
			$(MAKE) run

line		: $(line)
			$(MAKE) run

point		: $(point)
			$(MAKE) run

vert		: $(vert)
			$(MAKE) run

run			:  $(target)
			./$(target)

$(target):  $(src_files) $(obj_files)
			$(cflags) -o $@ $^ $(linkers)

%.o			: %.c
			$(cflags) -c $< -o $@ $(includes)

clean:
	rm $(target) $(obj_files)

