#ifndef MODEM_TYPES_H
#define MODEM_TYPES_H

#include <stdint.h>

typedef enum
{
    EXPECT_NOTHING = 0,
    EXPECT_IMEI,
    EXPECT_MANUFACTURER,
    EXPECT_MODEL,
    EXPECT_SMS_TEXT,
    EXPECT_CELLS
} ParseState;

typedef struct {
    char **data;
    int count;
    int capacity;
} List;

typedef struct
{
    char imei[32];
    char manufacturer[64];
    char model[64];
    char operator_name[64];
    char net_status[32];
    char band[64];
    char sim_status[32];
    char serving_cell[128];
    List neighbor;
    float signal_strength;
    char ip_address[64];
    List temperatures;
    List apn;
    char phone_number[32];
    List sms;
    char current_sms_sender[32];

    List ip_addresses;

    ParseState current_expect;
    int command_finished;
    int has_error;
    char error_details[128];
} ModemData;

#endif