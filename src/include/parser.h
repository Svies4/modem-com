#ifndef PARSER_H
#define PARSER_H

#include "modem_types.h"
#include "eg06_parser.h"
#include "rg501q-eu_parser.h"
#include <stdlib.h>

typedef int (*ParseHandler)(const char *line, ModemData *data);

typedef struct{
    const char *imei;
    const char *model;
    const char *manufacturer;
    const char *operator;
    const char *net_stat;
    const char *mobile_band;
    const char *sim_stat;
    const char *serving_cell;
    const char *neighbour_cell;
    const char *signal;
    const char *ip_addr;
    const char *set_print_mode;
    const char *sms;
    const char *temp;
    const char *number;
    const char *apn;
} AT_Rule;

typedef struct{
    const char *prefix;
    ParseHandler handler;
} ParserRule;

typedef const struct {
    const char *match_string;
    const ParserRule *rules;
    AT_Rule *at_rules;
} ModemHandler;

ModemHandler* init_modem_parser(const char *manufacturer, const char *model);

void route_response(const char *line, ModemData *data, ModemHandler *handler);

#endif