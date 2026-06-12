#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <poll.h>
#include <sys/time.h>
#include "at.h"
#include "io.h"
#include "parser.h"

static long long get_time_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (((long long)tv.tv_sec) * 1000) + (tv.tv_usec / 1000);
}

static void extract_and_route_lines(char *line_buffer, int *buf_len, ModemData *data) {
    char *line_end;
    
    while ((line_end = strstr(line_buffer, "\r\n")) != NULL) {
        *line_end = '\0';
        
        route_response(line_buffer, data);

        int rem = *buf_len - ((line_end + 2) - line_buffer);
        memmove(line_buffer, line_end + 2, rem);
        
        *buf_len = rem;
        line_buffer[*buf_len] = '\0';
    }
}

static int execute_single_attempt(int fd, const char *cmd, ModemData *data, int timeout_ms) {
    char read_buf[256];
    char line_buffer[2048] = {0};
    int buf_len = 0;

    data->command_finished = 0;
    data->has_error = 0;
    memset(data->error_details, 0, sizeof(data->error_details));

    tcflush(fd, TCIOFLUSH); 
    if (write_data(fd, cmd) < 0) {
        return -1;
    }

    struct pollfd pfd = { .fd = fd, .events = POLLIN };
    long long start_time = get_time_ms();
    int time_left = timeout_ms;

    while (time_left > 0 && !data->command_finished) {
        
        if (poll(&pfd, 1, time_left) > 0 && (pfd.revents & POLLIN)) {
            
            int bytes = read_data(fd, read_buf, sizeof(read_buf));
            
            if (bytes > 0) {
                strncat(line_buffer, read_buf, sizeof(line_buffer) - buf_len - 1);
                buf_len += bytes;
                extract_and_route_lines(line_buffer, &buf_len, data);
                time_left = 500; 
            }
        }
        time_left = timeout_ms - (int)(get_time_ms() - start_time);
    }
    return (data->command_finished && !data->has_error) ? 0 : -1;
}

int send_at_command(int fd, const char *cmd, ModemData *data, int max_retries, int timeout_ms) {
    for (int attempt = 1; attempt <= max_retries; attempt++) {
        if (execute_single_attempt(fd, cmd, data, timeout_ms) == 0) {
            return 0;
        }
        sleep(1); 
    }

    if (data->has_error && strlen(data->error_details) > 0) {
        fprintf(stderr, "[MODEM ERROR] Command '%s' failed: %s\n", cmd, data->error_details);
    } else {
        fprintf(stderr, "[MODEM ERROR] Command '%s' timed out after %d attempts.\n", cmd, max_retries);
    }

    return -1;
}