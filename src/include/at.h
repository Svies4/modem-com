#ifndef AT_H
#define AT_H

#include "modem_types.h"
#include "parser.h"

int send_at_command(int fd, const char *cmd, ModemData *data, ModemHandler *handler, int max_retries, int timeout_ms);

#endif