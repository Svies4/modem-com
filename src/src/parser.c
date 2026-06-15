#include <stdio.h>
#include <string.h>
#include "parser.h"

static const ParserRule eg06_parser_rules[] = {
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

static const ParserRule rg501qeu_parser_rules[] = {
    {"+CSQ: ", parse_csq},
    {"+COPS: ", parse_cops},
    {"+CEREG: ", parse_cereg},
    {"+CMGL: ", parse_cmgl},
    {"+QNWINFO: ", parse_qnwinfo},
    {"+CPIN: ", parse_cpin},
    {"+QENG: \"servingcell\"", parse_qengs},
    {"+QENG: \"neighbourcell ", parse_qengn},
    {"+CGPADDR: ", parse_cgpaddr},
    {"+QTEMP:", parse_qtemp_rg501qeu},
    {"+CNUM: ", parse_cnum},
    {"+CGDCONT: ", parse_cgdcont},
    {NULL, NULL}};

//AT commands for getting specific info about modem
const AT_Rule default_at_rules = {
    .imei           = "AT+CGSN",
    .model          = "AT+CGMM",
    .manufacturer   = "AT+CGMI",
    .operator       = "AT+COPS?",
    .net_stat       = "AT+CEREG?",
    .mobile_band    = "AT+QNWINFO",
    .sim_stat       = "AT+CPIN?",
    .serving_cell   = "AT+QENG=\"servingcell\"",
    .neighbour_cell = "AT+QENG=\"neighbourcell\"",
    .signal         = "AT+CSQ",
    .ip_addr        = "AT+CGPADDR",
    .set_print_mode = "AT+CMGF=1",
    .sms            = "AT+CMGL=\"ALL\"",
    .temp           = "AT+QTEMP",
    .number         = "AT+CNUM",
    .apn            = "AT+CGDCONT?"
};

static const ModemHandler modem_registry[] = {
    {"EG06", eg06_parser_rules, &default_at_rules},
    {"RG501Q-EU", rg501qeu_parser_rules, &default_at_rules},
    {NULL, NULL, NULL}
};

ModemHandler* init_modem_parser(const char *manufacturer, const char *model)
{
    if (manufacturer == NULL && model == NULL) return NULL;
    
    for (int i = 0; modem_registry[i].match_string != NULL; i++) {
        if ((manufacturer && strcasestr(manufacturer, modem_registry[i].match_string) != NULL) ||
            (model && strcasestr(model, modem_registry[i].match_string) != NULL)) 
        {
            return (ModemHandler*)&(modem_registry[i]); 
        }
    }
    return NULL;
}

void route_response(const char *line, ModemData *data, ModemHandler *handler)
{
    if (strlen(line) == 0) return;

    if (data->current_expect == EXPECT_SMS_TEXT){
        size_t needed_len = snprintf(NULL, 0, "From %s: %s", data->current_sms_sender, line) + 1;
        list_add(&data->sms, needed_len);
        snprintf(data->sms.data[data->sms.count], needed_len, "From %s: %s", data->current_sms_sender, line);
        data->sms.count++;
        data->current_expect = EXPECT_NOTHING;
        return;
    }

    if (strcmp(line, "OK") == 0){
        data->command_finished = 1;
        return;
    }
    if (strcmp(line, "ERROR") == 0 || strncmp(line, "+CME ERROR", 10) == 0 || strncmp(line, "+CMS ERROR", 10) == 0){
        data->has_error = 1;
        data->command_finished = 1;
        strncpy(data->error_details, line, sizeof(data->error_details) - 1);
        return;
    }

    switch (data->current_expect)
    {
    case EXPECT_IMEI:
        if (line[0] >= '0' && line[0] <= '9')
        {
            strncpy(data->imei, line, sizeof(data->imei) - 1);
        }
        return;

    case EXPECT_MANUFACTURER:
        strncpy(data->manufacturer, line, sizeof(data->manufacturer) - 1);
        return;

    case EXPECT_MODEL:
        strncpy(data->model, line, sizeof(data->model) - 1);
        return;

    case EXPECT_NOTHING:
        if(handler == NULL) return;
        for (int i = 0; handler->rules[i].prefix != NULL; i++){
            if (strncmp(line, handler->rules[i].prefix, strlen(handler->rules[i].prefix)) == 0){
                handler->rules[i].handler(line, data);
                return;
            }
        }
        return;
    default:
        break;
    }   
}