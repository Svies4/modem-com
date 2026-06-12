#include <stdlib.h>
#include <string.h>
#include "arg.h"
#include "utils.h" 

const char *argp_program_version = "modem-com 0.1";
const char *argp_program_bug_address = "<jbarauskas0@gmail.com>";
char doc[] = "modem-com - A CLI tool to read data from modems via AT commands.";
char args_doc[] = ""; 

enum {
    OPT_IMEI = 1000,
    OPT_INFO,
    OPT_OPERATOR,
    OPT_NETSTAT,
    OPT_BAND,
    OPT_SIM,
    OPT_CELL,
    OPT_NEIGHBOR,
    OPT_SIGNAL,
    OPT_IP,
    OPT_TEMP,
    OPT_APN,
    OPT_PHONE,
    OPT_SMS,
    OPT_ALL,
    OPT_JSON,
    OPT_DEBUG
};

struct argp_option options[] = {
    {"device",   'd', "DEVICE", 0, "Specify the modem device (default: /dev/ttyUSB2)"},
    {"imei",     OPT_IMEI,     0, 0, "Show modem IMEI"},
    {"info",     OPT_INFO,     0, 0, "Show modem model and manufacturer"},
    {"operator", OPT_OPERATOR, 0, 0, "Show current connected operator"},
    {"netstat",  OPT_NETSTAT,  0, 0, "Show network connection status"},
    {"band",     OPT_BAND,     0, 0, "Show mobile network frequency band"},
    {"sim",      OPT_SIM,      0, 0, "Show SIM card status (inserted/active)"},
    {"cell",     OPT_CELL,     0, 0, "Show serving cell information"},
    {"neighbor", OPT_NEIGHBOR, 0, 0, "Show neighbor cells"},
    {"signal",   OPT_SIGNAL,   0, 0, "Show signal strength"},
    {"ip",       OPT_IP,       0, 0, "Show IP address (if data session is active)"},
    {"temp",     OPT_TEMP,     0, 0, "Show modem temperature"},
    {"apn",      OPT_APN,      0, 0, "Show currently used APN"},
    {"phone",    OPT_PHONE,    0, 0, "Show phone number (if available)"},
    {"sms",      OPT_SMS,      0, 0, "Read SMS messages"},
    {"all",      OPT_ALL,      0, 0, "Show ALL available modem information"},
    {"json",     OPT_JSON,     0, 0, "Output results in JSON format"},
    {"debug",    OPT_DEBUG,    0, 0, "Enable debug mode (print raw AT commands/responses)"},
    {0} 
};

error_t parse_opt(int key, char *arg, struct argp_state *state)
{
    struct arguments *arguments = state->input;

    switch (key)
    {
    case 'd':
        trim(arg);
        arguments->device = arg;
        break;
    case OPT_IMEI:     arguments->show_imei = 1; break;
    case OPT_INFO:     arguments->show_info = 1; break;
    case OPT_OPERATOR: arguments->show_operator = 1; break;
    case OPT_NETSTAT:  arguments->show_net_status = 1; break;
    case OPT_BAND:     arguments->show_band = 1; break;
    case OPT_SIM:      arguments->show_sim_status = 1; break;
    case OPT_CELL:     arguments->show_cell = 1; break;
    case OPT_NEIGHBOR: arguments->show_neighbor = 1; break;
    case OPT_SIGNAL:   arguments->show_signal = 1; break;
    case OPT_IP:       arguments->show_ip = 1; break;
    case OPT_TEMP:     arguments->show_temp = 1; break;
    case OPT_APN:      arguments->show_apn = 1; break;
    case OPT_PHONE:    arguments->show_phone = 1; break;
    case OPT_SMS:      arguments->show_sms = 1; break;
    
    case OPT_ALL:
        arguments->show_all = 1;
        arguments->show_imei = 1;
        arguments->show_info = 1;
        arguments->show_operator = 1;
        arguments->show_net_status = 1;
        arguments->show_band = 1;
        arguments->show_sim_status = 1;
        arguments->show_cell = 1;
        arguments->show_neighbor = 1;
        arguments->show_signal = 1;
        arguments->show_ip = 1;
        arguments->show_temp = 1;
        arguments->show_apn = 1;
        arguments->show_phone = 1;
        arguments->show_sms = 1;
        break;
        
    case OPT_JSON:     arguments->use_json = 1; break;
    case OPT_DEBUG:    arguments->debug_mode = 1; break;
    
    case ARGP_KEY_END:
        if (!arguments->show_all && !arguments->show_imei && !arguments->show_info && 
            !arguments->show_operator && !arguments->show_net_status && 
            !arguments->show_band && !arguments->show_sim_status && 
            !arguments->show_cell && !arguments->show_neighbor && 
            !arguments->show_signal && !arguments->show_ip && 
            !arguments->show_temp && !arguments->show_apn && 
            !arguments->show_phone && !arguments->show_sms) 
        {
            argp_usage(state);
        }
        break;
    default:
        return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

struct argp argp = {options, parse_opt, args_doc, doc};