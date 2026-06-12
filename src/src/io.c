#include "io.h"

int init_serial_port(const char *port_name)
{
    int fd = open(port_name, O_RDWR | O_NOCTTY | O_NDELAY);

    if (fd < 0)
    {
        fprintf(stderr, "Error %d opening %s: %s\n", errno, port_name, strerror(errno));
        return -1;
    }

    struct termios tty;
    if (tcgetattr(fd, &tty) != 0)
    {
        fprintf(stderr, "Error %d from tcgetattr: %s\n", errno, strerror(errno));
        close(fd);
        return -1;
    }

    cfsetospeed(&tty, B115200);
    cfsetispeed(&tty, B115200);

    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;

    tty.c_cflag &= ~CRTSCTS;
    tty.c_cflag |= CREAD | CLOCAL;

    tty.c_lflag &= ~ICANON;
    tty.c_lflag &= ~ECHO;
    tty.c_lflag &= ~ECHOE;
    tty.c_lflag &= ~ECHONL;
    tty.c_lflag &= ~ISIG;

    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);

    tty.c_oflag &= ~OPOST;
    tty.c_oflag &= ~ONLCR;

    if (tcsetattr(fd, TCSANOW, &tty) != 0)
    {
        fprintf(stderr, "Error %d from tcsetattr: %s\n", errno, strerror(errno));
        close(fd);
        return -1;
    }

    fcntl(fd, F_SETFL, FNDELAY);

    return fd;
}

int write_data(int fd, const char *command)
{
    char buffer[256];

    snprintf(buffer, sizeof(buffer), "%s\r", command);

    int bytes_written = write(fd, buffer, strlen(buffer));

    if (bytes_written < 0)
    {
        fprintf(stderr, "Error writing to port\n");
        return -1;
    }

    return bytes_written;
}

int read_data(int fd, char *buffer, size_t buffer_size)
{
    memset(buffer, 0, buffer_size);

    int bytes_read = read(fd, buffer, buffer_size - 1);

    if (bytes_read > 0)
    {
        return bytes_read;
    }
    else if (bytes_read < 0 && errno != EAGAIN)
    {
        fprintf(stderr, "Error reading from port\n");
        return -1;
    }

    return 0;
}