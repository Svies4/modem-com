#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cJSON.h>
#include <unistd.h>
#include <poll.h>
#include "arg.h"
#include "utils.h"
#include "io.h"
#include "modem_types.h"
#include "at.h"
#include "parser.h"

typedef struct {
    int *show_flag;
    const char *debug_msg;
    ParseState expect_state;
    const char *command;
    int timeout_ms;
} CommandExecRule;

int main(int argc, char **argv)
{
    struct arguments arguments = {0}; 
    ModemData data = {0};
    ModemHandler *handler = NULL;
    
    arguments.device = "/dev/ttyUSB2";

    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    if (arguments.debug_mode) printf("[DEBUG] Opening port: %s\n", arguments.device);

    int fd = init_serial_port(arguments.device);

    if (fd < 0)
    {
        printf("Failed to open serial port\n");
        return 1;
    }

    if (arguments.debug_mode) printf("[DEBUG] Discovering modem model...\n");

    data.current_expect = EXPECT_MODEL;
    send_at_command(fd, "AT+CGMM", &data, handler, 3, 2000);
    data.current_expect = EXPECT_NOTHING;
    handler = init_modem_parser(data.manufacturer, data.model);
    if (handler == NULL) {
        if (arguments.debug_mode) printf("[DEBUG] Unknown modem '%s'. Falling back to default parser.\n", data.model);
        handler = init_modem_parser(NULL, "EG06");
    } else if (arguments.debug_mode) {
        printf("[DEBUG] Detected Model: %s\n", strlen(data.model) ? data.model : "UNKNOWN");
    }

    CommandExecRule exec_rules[] = {
        { &arguments.show_imei,       "Fetching IMEI...",            EXPECT_IMEI,         handler->at_rules->imei,           3000 },
        { &arguments.show_info,       "Fetching INFO...",            EXPECT_MANUFACTURER, handler->at_rules->manufacturer,   3000 },
        { &arguments.show_operator,   "Fetching CURRENT OPERATOR...",EXPECT_NOTHING,      handler->at_rules->operator,       3000 },
        { &arguments.show_net_status, "Fetching NETWORK STATUS...",  EXPECT_NOTHING,      handler->at_rules->net_stat,       3000 },
        { &arguments.show_band,       "Fetching MOBILE BAND...",     EXPECT_NOTHING,      handler->at_rules->mobile_band,    3000 },
        { &arguments.show_sim_status, "Fetching SIM STATUS...",      EXPECT_NOTHING,      handler->at_rules->sim_stat,       3000 },
        { &arguments.show_cell,       "Fetching SERVING CELL...",    EXPECT_NOTHING,      handler->at_rules->serving_cell,   3000 },
        { &arguments.show_neighbor,   "Fetching NEIGHBOR CELLS...",  EXPECT_NOTHING,      handler->at_rules->neighbour_cell, 3000 },
        { &arguments.show_signal,     "Fetching SIGNAL STRENGTH...", EXPECT_NOTHING,      handler->at_rules->signal,         3000 },
        { &arguments.show_ip,         "Fetching IP ADDRESSES...",    EXPECT_NOTHING,      handler->at_rules->ip_addr,        3000 },
        { &arguments.show_sms,        "Configuring SMS MODE...",     EXPECT_NOTHING,      handler->at_rules->set_print_mode, 3000 },
        { &arguments.show_sms,        "Fetching SMS MESSAGES...",    EXPECT_NOTHING,      handler->at_rules->sms,            5000 },
        { &arguments.show_temp,       "Fetching TEMPERATURES...",    EXPECT_NOTHING,      handler->at_rules->temp,           3000 },
        { &arguments.show_phone,      "Fetching PHONE NUMBER...",    EXPECT_NOTHING,      handler->at_rules->number,         3000 },
        { &arguments.show_apn,        "Fetching APN...",             EXPECT_NOTHING,      handler->at_rules->apn,            3000 },    
        { NULL, NULL, EXPECT_NOTHING, NULL, 0 }
    };

    for (int i = 0; exec_rules[i].show_flag != NULL; i++) {
        if (*(exec_rules[i].show_flag)) {
            if (exec_rules[i].command == NULL) {
                if (arguments.debug_mode) printf("[DEBUG] Skipping: %s (Not supported by modem)\n", exec_rules[i].debug_msg);
                continue;
            }
            if (arguments.debug_mode) printf("[DEBUG] %s\n", exec_rules[i].debug_msg);
            
            data.current_expect = exec_rules[i].expect_state;
            if (send_at_command(fd, exec_rules[i].command, &data, handler, 3, exec_rules[i].timeout_ms)) {
                printf("ERROR\n");
            }
            data.current_expect = EXPECT_NOTHING;
        }
    }

    print_results(&data, &arguments);

    if (data.temperatures.data != NULL) {
        for (int i = 0; i < data.temperatures.count; i++) {
            free(data.temperatures.data[i]);
        }
        free(data.temperatures.data);
    }

    if (data.sms.data != NULL) {
        for (int i = 0; i < data.sms.count; i++) {
            free(data.sms.data[i]);
        }
        free(data.sms.data);
    }

    if (data.neighbor.data != NULL) {
        for (int i = 0; i < data.neighbor.count; i++) {
            free(data.neighbor.data[i]);
        }
        free(data.neighbor.data);
    }

    if (data.ip_addresses.data != NULL) {
        for (int i = 0; i < data.ip_addresses.count; i++) {
            free(data.ip_addresses.data[i]);
        }
        free(data.ip_addresses.data);
    }

    return 0;
}