#Create lib_simpleloader.so from loader.c
lib_simpleloader.so : loader.c
	gcc -m32 -fPIC -shared -o lib_simpleloader.so loader.c

#Provide the command for cleanup
clean:
	-@rm -f fib loader lib_simpleloader.so