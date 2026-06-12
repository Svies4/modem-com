#include <stdio.h>
#include <string.h>
#include "parser.h"

typedef int (*ParseHandler)(const char *line, ModemData *data);

// Parser for Signal Strength (AT+CSQ)
static int parse_csq(const char *line, ModemData *data)
{
    float rssi;
    if (sscanf(line, "+CSQ: %f", &rssi) == 1){
        data->signal_strength = rssi;
        return 0;
    }
    return -1;
}
// Parser for Operator (AT+COPS?)
static int parse_cops(const char *line, ModemData *data)
{
    char temp_name[64] = {0};
    if (sscanf(line, "+COPS: %*d,%*d,\"%63[^\"]\"", temp_name) == 1){
        strncpy(data->operator_name, temp_name, sizeof(data->operator_name) - 1);
        return 0;
    }
    return -1;
}
// Parser for Network Status (AT+CEREG?)
static int parse_cereg(const char *line, ModemData *data)
{
    int n, stat;
    if (sscanf(line, "+CEREG: %d,%d", &n, &stat) == 2){
        if (stat == 1 || stat == 5) strcpy(data->net_status, "Connected");
        else strcpy(data->net_status, "Disconnected");
        return 0;
    }
    return -1;
}
// Parser for QNWINFO mobile network band
static int parse_qnwinfo(const char *line, ModemData *data)
{
    char band[64] = {0};
    if (sscanf(line, "+QNWINFO: \"%*[^\"]\",\"%*[^\"]\",\"%63[^\"]\"", band) == 1){
        strncpy(data->band, band, sizeof(data->band) - 1);
        return 0;
    }
    return -1;
}
// Parser for AT+CPIN? sim status
static int parse_cpin(const char *line, ModemData *data)
{
    char sim_status[32] = {0};
    if (sscanf(line, "+CPIN: %31[^\"]", sim_status) == 1){
        strncpy(data->sim_status, sim_status, sizeof(data->sim_status) - 1);
        return 0;
    }
    return -1;
}
// Parser for SMS Listing (AT+CMGL="ALL")
static int parse_cmgl(const char *line, ModemData *data)
{
    char sender[32] = {0};
    if (sscanf(line, "+CMGL: %*d,\"%*[^\"]\",\"%31[^\"]\"", sender) >= 1){
        strncpy(data->current_sms_sender, sender, sizeof(data->current_sms_sender) - 1);
        data->current_expect = EXPECT_SMS_TEXT;
        return 0;
    }
    return -1;
}
// Parser for AT+QENG="servingcell" cell information
static int parse_qengs(const char *line, ModemData *data)
{
    char serving_cell[256] = {0};
    if (sscanf(line, "+QENG: \"servingcell\",%255[^\r\n]", serving_cell) == 1){
        strncpy(data->serving_cell, serving_cell, sizeof(data->serving_cell) - 1);
        return 0;
    }
    return -1;
}

void list_add(List *list, size_t needed_len)
{
    if (list->capacity == 0) {
        list->capacity = 2;
        list->data = malloc(list->capacity * sizeof(char *));
    } else if (list->count >= list->capacity) {
        list->capacity += 2;
        list->data = realloc(list->data, list->capacity * sizeof(char *));
    } 
    list->data[list->count] = malloc(needed_len);
}
// Parser for AT+QENG="neighbourcell" cell information
static int parse_qengn(const char *line, ModemData *data)
{
    char cell_type[16] = {0};
    char cell_data[256] = {0};
    if (sscanf(line, "+QENG: \"neighbourcell %15[^\"]\",%255[^\r\n]", cell_type, cell_data) == 2){
        size_t needed_len = snprintf(NULL, 0, "[%s] %s", cell_type, cell_data) + 1;
        list_add(&data->neighbor, needed_len);
        snprintf(data->neighbor.data[data->neighbor.count], needed_len, "[%s] %s", cell_type, cell_data);
        data->neighbor.count++;
        return 0;
    } 
    return -1;
}
// Parser for AT+CGPADDR ip info
static int parse_cgpaddr(const char *line, ModemData *data)
{
    int cid;
    char ip_data[128] = {0};
    int matched = sscanf(line, "+CGPADDR: %d,\"%127[^\"]\"", &cid, ip_data);
    if (matched >= 1){
        size_t needed_len;

        if (matched == 2 && strlen(ip_data) > 0) {
            needed_len = snprintf(NULL, 0, "Context %d: %s", cid, ip_data) + 1;
            list_add(&data->ip_addresses, needed_len);
            snprintf(data->ip_addresses.data[data->ip_addresses.count], needed_len, "Context %d: %s", cid, ip_data);
        } else {
            char str[] = "Context %d: NO IP";
            needed_len = snprintf(NULL, 0, str, cid) + 1;
            list_add(&data->ip_addresses, needed_len);
            snprintf(data->ip_addresses.data[data->ip_addresses.count], needed_len, str, cid);
        } 
        data->ip_addresses.count++;
        return 0;
    }   
    return -1;
}
//Parser for AT+QTEMP modem temperature info
static int parse_qtemp(const char *line, ModemData *data)
{
    char temp[16] = {0};
    if(sscanf(line, "+QTEMP: %15[^\"]", temp) == 1){
        strncpy(data->temperature, temp, sizeof(data->temperature)-1);
        return 0;
    }
    return -1;
}
//Parser for AT+CNUM phone number
static int parse_cnum(const char *line, ModemData *data)
{
    char number[32] = {0};
    if (sscanf(line, "%*[^,],\"%31[^\"]\"", number) == 1){
        strncpy(data->phone_number, number, sizeof(data->phone_number) - 1);
        return 0;
    }
    return -1;
}
// Parser for AT+CGDCONT? apn info
static int parse_cgdcont(const char *line, ModemData *data)
{
    int cid;
    char protocol[32] = {0};
    char apn[64] = {0};

    int matched = sscanf(line, "+CGDCONT: %d,\"%31[^\"]\",\"%63[^\"]\"", &cid, protocol, apn);
    if (matched >= 2){
        size_t needed_len;
        
        if (matched == 3 && strlen(apn) > 0) {
            needed_len = snprintf(NULL, 0, "Context %d: %s (%s)", cid, apn, protocol) + 1;
            list_add(&data->apn, needed_len);
            snprintf(data->apn.data[data->apn.count], needed_len, "Context %d: %s (%s)", cid, apn, protocol);
        } else {
            needed_len = snprintf(NULL, 0, "Context %d: [No APN Configured] (%s)", cid, protocol) + 1;
            list_add(&data->apn, needed_len);
            snprintf(data->apn.data[data->apn.count], needed_len, "Context %d: [No APN Configured] (%s)", cid, protocol);
        }
        data->apn.count++;
        return 0;
    }
    return -1;
}

static const struct
{
    const char *prefix;
    ParseHandler handler;
} parser_rules[] = {
    {"+CSQ: ", parse_csq},
    {"+COPS: ", parse_cops},
    {"+CEREG: ", parse_cereg},
    {"+CMGL: ", parse_cmgl},
    {"+QNWINFO: ", parse_qnwinfo},
    {"+CPIN: ", parse_cpin},
    {"+QENG: \"servingcell\"", parse_qengs},
    {"+QENG: \"neighbourcell ", parse_qengn},
    {"+CGPADDR: ", parse_cgpaddr},
    {"+QTEMP: ", parse_qtemp},
    {"+CNUM: ", parse_cnum},
    {"+CGDCONT: ", parse_cgdcont},
    {NULL, NULL}};

void route_response(const char *line, ModemData *data)
{
    if (strlen(line) == 0)
        return;

    if (data->current_expect == EXPECT_SMS_TEXT)
    {
        size_t needed_len = snprintf(NULL, 0, "From %s: %s", data->current_sms_sender, line) + 1;
        list_add(&data->sms, needed_len);
        snprintf(data->sms.data[data->sms.count], needed_len, "From %s: %s", data->current_sms_sender, line);
        data->sms.count++;
        data->current_expect = EXPECT_NOTHING;
        return;
    }

    if (strcmp(line, "OK") == 0)
    {
        data->command_finished = 1;
        return;
    }
    if (strcmp(line, "ERROR") == 0 || strncmp(line, "+CME ERROR", 10) == 0 || strncmp(line, "+CMS ERROR", 10) == 0)
    {
        data->has_error = 1;
        data->command_finished = 1;
        strncpy(data->error_details, line, sizeof(data->error_details) - 1);
        return;
    }

    for (int i = 0; parser_rules[i].prefix != NULL; i++)
    {
        if (strncmp(line, parser_rules[i].prefix, strlen(parser_rules[i].prefix)) == 0)
        {
            parser_rules[i].handler(line, data);
            return;
        }
    }

    switch (data->current_expect)
    {
    case EXPECT_IMEI:
        if (line[0] >= '0' && line[0] <= '9')
        {
            strncpy(data->imei, line, sizeof(data->imei) - 1);
        }
        break;

    case EXPECT_MANUFACTURER:
        strncpy(data->manufacturer, line, sizeof(data->manufacturer) - 1);
        break;

    case EXPECT_MODEL:
        strncpy(data->model, line, sizeof(data->model) - 1);
        break;

    case EXPECT_NOTHING:
    default:
        break;
    }
}