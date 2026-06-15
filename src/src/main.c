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

static int running = 1;

void signalHandler(int sig)
{
    printf("signal received: %d\n", sig);
    running = 0;
}

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

    if (arguments.show_imei) {
        if (arguments.debug_mode) printf("[DEBUG] Fetching IMEI...\n");
        data.current_expect = EXPECT_IMEI;
        if(send_at_command(fd, handler->at_rules->imei, &data, handler, 3, 3000)){
            printf("ERROR\n");
        }
        data.current_expect = EXPECT_NOTHING;
    }

    if (arguments.show_info) {
        if (arguments.debug_mode) printf("[DEBUG] Fetching INFO...\n");
        data.current_expect = EXPECT_MANUFACTURER;
        if(send_at_command(fd, handler->at_rules->manufacturer, &data, handler, 3, 3000)){
            printf("ERROR\n");
        }
        data.current_expect = EXPECT_NOTHING;
    }

    if (arguments.show_operator) {
        if (arguments.debug_mode) printf("[DEBUG] Fetching CURRENT OPERATOR...\n");
        data.current_expect = EXPECT_NOTHING;
        if(send_at_command(fd, handler->at_rules->operator, &data, handler, 3, 3000)){
            printf("ERROR\n");
        }
    }

    if (arguments.show_net_status) {
        if (arguments.debug_mode) printf("[DEBUG] Fetching NETWORK STATUS...\n");
        data.current_expect = EXPECT_NOTHING;
        if(send_at_command(fd, handler->at_rules->net_stat, &data, handler, 3, 3000)){
            printf("ERROR\n");
        }
    }

    if (arguments.show_band) {
        if (arguments.debug_mode) printf("[DEBUG] Fetching MOBILE BAND...\n");
        data.current_expect = EXPECT_NOTHING;
        if(send_at_command(fd, handler->at_rules->mobile_band, &data, handler, 3, 3000)){
            printf("ERROR\n");
        }
    }

    if (arguments.show_sim_status) {
        if (arguments.debug_mode) printf("[DEBUG] Fetching SIM STATUS...\n");
        data.current_expect = EXPECT_NOTHING;
        if(send_at_command(fd, handler->at_rules->sim_stat, &data, handler, 3, 3000)){
            printf("ERROR\n");
        }
    }

    if (arguments.show_cell) {
        if (arguments.debug_mode) printf("[DEBUG] Fetching SERVING CELL...\n");
        data.current_expect = EXPECT_NOTHING;
        if(send_at_command(fd, handler->at_rules->serving_cell, &data, handler, 3, 3000)){
            printf("ERROR\n");
        }
    }

    if (arguments.show_neighbor) {
        if (arguments.debug_mode) printf("[DEBUG] Fetching SERVING CELL...\n");
        data.current_expect = EXPECT_CELLS;
        if(send_at_command(fd, handler->at_rules->neighbour_cell, &data, handler, 3, 3000)){
            printf("ERROR\n");
        }
        data.current_expect = EXPECT_NOTHING;
    }

    if (arguments.show_signal) {
        if (arguments.debug_mode) printf("[DEBUG] Fetching SIGNAL STRENGTH...\n");
        data.current_expect = EXPECT_NOTHING;
        if(send_at_command(fd, handler->at_rules->signal, &data, handler, 3, 3000)){
            printf("ERROR\n");
        }
    }

    if (arguments.show_ip) {
        if (arguments.debug_mode) printf("[DEBUG] Fetching IP ADDRESSES...\n");
        data.current_expect = EXPECT_NOTHING;
        if(send_at_command(fd, handler->at_rules->ip_addr, &data, handler, 3, 3000)){
            printf("ERROR\n");
        }
    }

    if (arguments.show_sms) {
        if (arguments.debug_mode) printf("[DEBUG] Fetching SMS MESSAGES...\n");
        send_at_command(fd, handler->at_rules->set_print_mode, &data, handler, 3, 3000);
        send_at_command(fd, handler->at_rules->sms, &data, handler, 3, 5000);
    }

    if (arguments.show_temp) {
        if (arguments.debug_mode) printf("[DEBUG] Fetching TEMPERATURES...\n");
        data.current_expect = EXPECT_NOTHING;
        if(send_at_command(fd, handler->at_rules->temp, &data, handler, 3, 3000)){
            printf("ERROR\n");
        }
    }

    if (arguments.show_phone) {
        if (arguments.debug_mode) printf("[DEBUG] Fetching PHONE NUMBER...\n");
        data.current_expect = EXPECT_NOTHING;
        if(send_at_command(fd, handler->at_rules->number, &data, handler, 3, 3000)){
            printf("ERROR\n");
        }
    }

    if (arguments.show_apn) {
        if (arguments.debug_mode) printf("[DEBUG] Fetching APN...\n");
        data.current_expect = EXPECT_NOTHING;
        if(send_at_command(fd, handler->at_rules->apn, &data, handler, 3, 3000)){
            printf("ERROR\n");
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