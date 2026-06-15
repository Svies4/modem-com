#include "eg06_parser.h"

// Parser for Signal Strength (AT+CSQ)
  int parse_csq(const char *line, ModemData *data)
{
    float rssi;
    if (sscanf(line, "+CSQ: %f", &rssi) == 1){
        data->signal_strength = rssi;
        return 0;
    }
    return -1;
}
// Parser for Operator (AT+COPS?)
  int parse_cops(const char *line, ModemData *data)
{
    char temp_name[64] = {0};
    if (sscanf(line, "+COPS: %*d,%*d,\"%63[^\"]\"", temp_name) == 1){
        strncpy(data->operator_name, temp_name, sizeof(data->operator_name) - 1);
        return 0;
    }
    return -1;
}
// Parser for Network Status (AT+CEREG?)
  int parse_cereg(const char *line, ModemData *data)
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
  int parse_qnwinfo(const char *line, ModemData *data)
{
    char band[64] = {0};
    if (sscanf(line, "+QNWINFO: \"%*[^\"]\",\"%*[^\"]\",\"%63[^\"]\"", band) == 1){
        strncpy(data->band, band, sizeof(data->band) - 1);
        return 0;
    }
    return -1;
}
// Parser for AT+CPIN? sim status
  int parse_cpin(const char *line, ModemData *data)
{
    char sim_status[32] = {0};
    if (sscanf(line, "+CPIN: %31[^\"]", sim_status) == 1){
        strncpy(data->sim_status, sim_status, sizeof(data->sim_status) - 1);
        return 0;
    }
    return -1;
}
// Parser for SMS Listing (AT+CMGL="ALL")
  int parse_cmgl(const char *line, ModemData *data)
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
  int parse_qengs(const char *line, ModemData *data)
{
    char serving_cell[256] = {0};
    if (sscanf(line, "+QENG: \"servingcell\",%255[^\r\n]", serving_cell) == 1){
        strncpy(data->serving_cell, serving_cell, sizeof(data->serving_cell) - 1);
        return 0;
    }
    return -1;
}

// Parser for AT+QENG="neighbourcell" cell information
  int parse_qengn(const char *line, ModemData *data)
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
  int parse_cgpaddr(const char *line, ModemData *data)
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
  int parse_qtemp(const char *line, ModemData *data)
{
    int pmic, xo, pa;
    if(sscanf(line, "+QTEMP: %d,%d,%d", &pmic, &xo, &pa) == 3){
        size_t needed_len = snprintf(NULL, 0, "Core: %d°C | PA: %d°C | PMIC: %d°C", xo, pa, pmic) + 1;
        list_add(&data->temperatures, needed_len);
        snprintf(data->temperatures.data[data->temperatures.count], needed_len, 
                 "Core: %d°C | PA: %d°C | PMIC: %d°C", xo, pa, pmic);
        data->temperatures.count++;
        return 0;
    }
    return -1;
}
//Parser for AT+CNUM phone number
  int parse_cnum(const char *line, ModemData *data)
{
    char number[32] = {0};
    if (sscanf(line, "%*[^,],\"%31[^\"]\"", number) == 1){
        strncpy(data->phone_number, number, sizeof(data->phone_number) - 1);
        return 0;
    }
    return -1;
}
// Parser for AT+CGDCONT? apn and ip info
  int parse_cgdcont(const char *line, ModemData *data)
{
    int cid;
    char protocol[32] = {0};
    char apn[64] = {0};
    char ip[128] = {0};
    int matched = sscanf(line, "+CGDCONT: %d,\"%31[^\"]\",\"%63[^\"]\",\"%127[^\"]\"", &cid, protocol, apn, ip);
    
    if (matched >= 2){
        size_t needed_len;    
        if (matched >= 3 && strlen(apn) > 0) {
            if (matched == 4 && strlen(ip) > 0) {
                needed_len = snprintf(NULL, 0, "APN: %s | IP: %s", apn, ip) + 1;
                list_add(&data->apn, needed_len);
                snprintf(data->apn.data[data->apn.count], needed_len, "APN: %s | IP: %s", apn, ip);
            } 
            else {
                needed_len = snprintf(NULL, 0, "APN: %s | IP: [No IP]", apn) + 1;
                list_add(&data->apn, needed_len);
                snprintf(data->apn.data[data->apn.count], needed_len, "APN: %s | IP: [No IP]", apn);
            }
        } else {
            needed_len = snprintf(NULL, 0, "APN: [None] | IP: [None]") + 1;
            list_add(&data->apn, needed_len);
            snprintf(data->apn.data[data->apn.count], needed_len, "APN: [None] | IP: [None]");
        }
        data->apn.count++;
        return 0;
    }
    return -1;
}