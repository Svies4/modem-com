#ifndef ARG_H_
#define ARG_H_
#include <stdlib.h>
#include <argp.h>
#include <stdio.h>
#include "utils.h"

extern char doc[];
extern char args_doc[];

struct arguments
{
    char *device; // -d /dev/ttyUSBx

    int show_imei;       // --imei
    int show_info;       // --info (Model, Manufacturer)
    int show_operator;   // --operator (Current operator)
    int show_net_status; // --netstat (Network connection status)
    int show_band;       // --band (Mobile frequency band)
    int show_sim_status; // --sim (SIM card status)
    int show_cell;       // --cell (Serving cell info)
    int show_neighbor;   // --neighbor (Neighbor cells)
    int show_signal;     // --signal (Signal strength)
    int show_ip;         // --ip (IP address)
    int show_temp;       // --temp (Modem temperature)
    int show_apn;        // --apn (APN)
    int show_phone;      // --phone (Phone number)
    int show_sms;        // --sms (SMS)

    int show_all;   // --all
    int use_json;   // --json
    int debug_mode; // --debug
};

error_t parse_opt(int key, char *arg, struct argp_state *state);

extern struct argp argp;
;

#endif