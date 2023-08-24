#Compile the launch.c by linking it with the lib_simpleloader.so
all:
	
	gcc -I../loader  launch.c -m32 -fPIC -o launch -L../bin/lib_simpleloader.so ../loader/loader.c

#Provide the command for cleanup
clean:
	-@rm -f  launch