# Make sure to run clean if you want to run seperate rule/program that contains main

glfw_path		= $(HOME)/installs/glfw
glfw_dl_path	= $(glfw_path)/lib64
glad_path		= $(HOME)/packages
project_dir 	= src examples .
src_files		= src/impl_libcook.o
rekt			= examples/rekt.o
main			:= main.o

cflags			= clang -D_GNU_SOURCE -std=c2x -Wall -O0 -g
includes 		= -I$(glfw_path)/include -I$(glad_path) -Iinclude
rpath			= -Wl,-rpath=$(glfw_dl_path)
linkers			= -L$(glfw_dl_path) $(rpath) -lglfw -lm
target			= out

find_obj_files 	= $(wildcard $(dir)/*.o)
obj_files		= $(foreach dir, $(project_dir), $(find_obj_files))

.PHONY		: clean

exp			: $(main)
			$(MAKE) run

rekt		: $(rekt)
			$(MAKE) run

run			:  $(target)
			./$(target)

$(target):  $(src_files) $(obj_files)
			$(cflags) -o $@ $^ $(linkers)

%.o			: %.c
			$(cflags) -c $< -o $@ $(includes)

clean:
	rm $(target) $(obj_files)
