#ifndef EG06_PARSER_H
#define EG06_PARSER_H

#include "modem_types.h"
#include <stdlib.h>

extern int parse_csq(const char *line, ModemData *data);
extern int parse_cops(const char *line, ModemData *data);
extern int parse_cereg(const char *line, ModemData *data);
extern int parse_qnwinfo(const char *line, ModemData *data);
extern int parse_cpin(const char *line, ModemData *data);
extern int parse_cmgl(const char *line, ModemData *data);
extern int parse_qengs(const char *line, ModemData *data);
extern int parse_qengn(const char *line, ModemData *data);
extern int parse_cgpaddr(const char *line, ModemData *data);
extern int parse_qtemp(const char *line, ModemData *data);
extern int parse_cnum(const char *line, ModemData *data);
extern int parse_cgdcont(const char *line, ModemData *data);

#endif