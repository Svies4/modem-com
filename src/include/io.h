#ifndef IO_H_
#define IO_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>

int init_serial_port(const char *port_name);
int write_data(int fd, const char *command);
int read_data(int fd, char *buffer, size_t buffer_size);

#endif