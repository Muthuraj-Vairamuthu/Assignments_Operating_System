#invoke make inside following directories and in this order: loader, launch, fib
all:
	$(MAKE) -C loader
	$(MAKE) -C launcher
	$(MAKE) -C test

	mv ./loader/lib_simpleloader.so bin
	mv ./launcher/launch bin/

#move the lib_simpleloader.so and launch binaries inside bin directory
#Provide the command for cleanup
clean:
	$(MAKE) -C loader clean
	$(MAKE) -C launcher clean
	$(MAKE) -C test clean