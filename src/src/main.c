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
    
    arguments.device = "/dev/ttyUSB2";

    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    if (arguments.debug_mode) printf("[DEBUG] Opening port: %s\n", arguments.device);

    int fd = init_serial_port(arguments.device);

    if (fd < 0)
    {
        printf("Failed to open serial port\n");
        return 1;
    }

    if (arguments.show_imei) {
        if (arguments.debug_mode) printf("[DEBUG] Fetching IMEI...\n");
        data.current_expect = EXPECT_IMEI;
        if(send_at_command(fd, "AT+CGSN", &data, 3, 3000)){
            printf("ERROR\n");
        }
        data.current_expect = EXPECT_NOTHING;
    }

    if (arguments.show_info) {
        if (arguments.debug_mode) printf("[DEBUG] Fetching INFO...\n");
        data.current_expect = EXPECT_MANUFACTURER;
        if(send_at_command(fd, "AT+CGMI", &data, 3, 3000)){
            printf("ERROR\n");
        }
        data.current_expect = EXPECT_MODEL;
        if(send_at_command(fd, "AT+CGMM", &data, 3, 3000)){
            printf("ERROR\n");
        }
        data.current_expect = EXPECT_NOTHING;
    }

    if (arguments.show_operator) {
        if (arguments.debug_mode) printf("[DEBUG] Fetching CURRENT OPERATOR...\n");
        data.current_expect = EXPECT_NOTHING;
        if(send_at_command(fd, "AT+COPS?", &data, 3, 3000)){
            printf("ERROR\n");
        }
    }

    if (arguments.show_net_status) {
        if (arguments.debug_mode) printf("[DEBUG] Fetching NETWORK STATUS...\n");
        data.current_expect = EXPECT_NOTHING;
        if(send_at_command(fd, "AT+CEREG?", &data, 3, 3000)){
            printf("ERROR\n");
        }
    }

    if (arguments.show_band) {
        if (arguments.debug_mode) printf("[DEBUG] Fetching MOBILE BAND...\n");
        data.current_expect = EXPECT_NOTHING;
        if(send_at_command(fd, "AT+QNWINFO", &data, 3, 3000)){
            printf("ERROR\n");
        }
    }

    if (arguments.show_sim_status) {
        if (arguments.debug_mode) printf("[DEBUG] Fetching SIM STATUS...\n");
        data.current_expect = EXPECT_NOTHING;
        if(send_at_command(fd, "AT+CPIN?", &data, 3, 3000)){
            printf("ERROR\n");
        }
    }

    if (arguments.show_cell) {
        if (arguments.debug_mode) printf("[DEBUG] Fetching SERVING CELL...\n");
        data.current_expect = EXPECT_NOTHING;
        if(send_at_command(fd, "AT+QENG=\"servingcell\"", &data, 3, 3000)){
            printf("ERROR\n");
        }
    }

    if (arguments.show_neighbor) {
        if (arguments.debug_mode) printf("[DEBUG] Fetching SERVING CELL...\n");
        data.current_expect = EXPECT_CELLS;
        if(send_at_command(fd, "AT+QENG=\"neighbourcell\"", &data, 3, 3000)){
            printf("ERROR\n");
        }
        data.current_expect = EXPECT_NOTHING;
    }

    if (arguments.show_signal) {
        if (arguments.debug_mode) printf("[DEBUG] Fetching SIGNAL STRENGTH...\n");
        data.current_expect = EXPECT_NOTHING;
        if(send_at_command(fd, "AT+CSQ", &data, 3, 3000)){
            printf("ERROR\n");
        }
    }

    if (arguments.show_ip) {
        if (arguments.debug_mode) printf("[DEBUG] Fetching IP ADDRESSES...\n");
        data.current_expect = EXPECT_NOTHING;
        if(send_at_command(fd, "AT+CGPADDR", &data, 3, 3000)){
            printf("ERROR\n");
        }
    }

    if (arguments.show_sms) {
        // 1. Put the modem in Text Mode (Otherwise it replies in unreadable Hex)
        send_at_command(fd, "AT+CMGF=1", &data, 3, 3000);
        
        // 2. Fetch all messages (Use a 5000ms timeout because reading from SIM is slow)
        send_at_command(fd, "AT+CMGL=\"ALL\"", &data, 3, 5000);
    }

    if (arguments.show_temp) {
        if (arguments.debug_mode) printf("[DEBUG] Fetching TEMPERATURES...\n");
        data.current_expect = EXPECT_NOTHING;
        if(send_at_command(fd, "AT+QTEMP", &data, 3, 3000)){
            printf("ERROR\n");
        }
    }

    if (arguments.show_phone) {
        if (arguments.debug_mode) printf("[DEBUG] Fetching PHONE NUMBER...\n");
        data.current_expect = EXPECT_NOTHING;
        if(send_at_command(fd, "AT+CNUM", &data, 3, 3000)){
            printf("ERROR\n");
        }
    }

    if (arguments.show_phone) {
        if (arguments.debug_mode) printf("[DEBUG] Fetching APN...\n");
        data.current_expect = EXPECT_NOTHING;
        if(send_at_command(fd, "AT+CGDCONT?", &data, 3, 3000)){
            printf("ERROR\n");
        }
    }

    print_results(&data, &arguments);

    //if (arguments.show_all)        printf("- Fetch ALL Data\n");
    //if (arguments.show_imei)       printf("- IMEI\n");
    //if (arguments.show_info)       printf("- Modem Info\n");
    //if (arguments.show_operator)   printf("- Operator\n");
    /*if (arguments.show_net_status) printf("- Network Status\n");
    if (arguments.show_band)       printf("- Frequency Band\n");
    if (arguments.show_sim_status) printf("- SIM Status\n");
    if (arguments.show_cell)       printf("- Serving Cell\n");
    if (arguments.show_neighbor)   printf("- Neighbor Cells\n");
    if (arguments.show_signal)     printf("- Signal Strength\n");
    if (arguments.show_ip)         printf("- IP Address\n");
    if (arguments.show_temp)       printf("- Temperature\n");
    if (arguments.show_apn)        printf("- APN\n");
    if (arguments.show_phone)      printf("- Phone Number\n");
    if (arguments.show_sms)        printf("- Read SMS\n");


    if (arguments.show_imei) {
        if (arguments.debug_mode) printf("[DEBUG] Fetching IMEI...\n");
        data.current_expect = EXPECT_IMEI;
        send_at_command(fd, "AT+CGSN", &data, 3, 3000);
        data.current_expect = EXPECT_NOTHING;
    }*/

    /*if (data.sms_messages != NULL) {
        for (int i = 0; i < data.sms_count; i++) {
            free(data.sms_messages[i]);
        }
        free(data.sms_messages);
    }*/

    /*if (data.neighbor_cells != NULL) {
        for (int i = 0; i < data.neighbor_count; i++) {
            free(data.neighbor_cells[i]);
        }
        free(data.neighbor_cells);
    }*/

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