obj-m += device_driver.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	gcc -o user_program user_program.c
	gcc -o user_program_reset user_program_reset.c

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm user_program
