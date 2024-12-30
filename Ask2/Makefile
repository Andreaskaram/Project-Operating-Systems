CC=gcc
CFLAGS= -lpthread

launch: launch.o passenger.o ipc_utils.o
    $(CC) -o launch ipc_utils.c passenger.c launch.c -lpthread
