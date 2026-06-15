#include "utils.h"

void print_results(const ModemData *data, struct arguments *req)
{
    PrintRule rules[] = {
        { &req->show_imei,     "imei",            "IMEI",         TYPE_STRING, data->imei },
        { &req->show_info,     "manufacturer",    "Manufacturer", TYPE_STRING, data->manufacturer },
        { &req->show_info,     "model",           "Model",        TYPE_STRING, data->model },
        { &req->show_operator, "operator",        "Operator",     TYPE_STRING, data->operator_name },
        { &req->show_signal,   "signal_strength", "Signal (CSQ)", TYPE_FLOAT,    &data->signal_strength },
        { &req->show_cell,   "seving_cell", "Serving Cell info", TYPE_STRING,    &data->serving_cell },
        { &req->show_net_status,   "network_status", "Network status", TYPE_STRING,    &data->net_status },
        { &req->show_band,   "mobile_band", "Mobile band", TYPE_STRING,    &data->band },
        { &req->show_sim_status,   "sim_status", "SIM status", TYPE_STRING,    &data->sim_status },
        { &req->show_phone, "number", "Phone Number", TYPE_STRING, &data->phone_number },
        { &req->show_temp, "temperatures", "Temperatures", TYPE_LIST, &data->temperatures },
        { &req->show_sms,      "sms",             "SMS Messages", TYPE_LIST,    &data->sms },
        { &req->show_neighbor, "neighbor_cells", "Neighbor Cells", TYPE_LIST, &data->neighbor },
        { &req->show_ip, "ip_addresses", "IP Addresses", TYPE_LIST, &data->ip_addresses },
        { &req->show_apn, "apn", "APN", TYPE_LIST, &data->apn },
        { NULL, NULL, NULL, 0, NULL }
    };

    cJSON *root = NULL;
    if (req->use_json) {
        root = cJSON_CreateObject();
    } else {
        printf("\n--- MODEM DATA ---\n");
    }

    for (int i = 0; rules[i].show_flag != NULL; i++) {
        if (*(rules[i].show_flag)) {
            
            //handle string
            if (rules[i].type == TYPE_STRING) {
                const char *str_val = (const char *)rules[i].data_ptr;
                if (req->use_json) cJSON_AddStringToObject(root, rules[i].json_key, str_val);
                else printf("%-14s: %s\n", rules[i].text_label, str_val);
            } 
            
            //handle int
            else if (rules[i].type == TYPE_INT) {
                int int_val = *(const int *)rules[i].data_ptr;
                if (req->use_json) cJSON_AddNumberToObject(root, rules[i].json_key, int_val);
                else printf("%-14s: %d\n", rules[i].text_label, int_val);
            }

            //handle float
            else if (rules[i].type == TYPE_FLOAT) {
                float float_val = *(const float *)rules[i].data_ptr;
                if (req->use_json) cJSON_AddNumberToObject(root, rules[i].json_key, float_val);
                else printf("%-14s: %f\n", rules[i].text_label, float_val);
            }
            
            //handle list
            else if (rules[i].type == TYPE_LIST){
                List *list = rules[i].data_ptr;
                int count = list->count; int capacity = list->capacity; char **data = list->data;
                if (rules[i].type == TYPE_LIST && count > 0) {
                
                    if (req->use_json) {
                        cJSON *neighbor_array = cJSON_AddArrayToObject(root, rules[i].json_key);
                        for (int n = 0; n < count; n++) {
                            cJSON_AddItemToArray(neighbor_array, cJSON_CreateString(data[n]));
                        }
                    } else {
                        printf("\n--- %s ---\n", rules[i].text_label);
                        for (int n = 0; n < count; n++) {
                            printf("[%d] %s\n", n + 1, data[n]);
                        }
                    }
                }
            }  
        }
    }

    if (req->use_json) {
        char *json_string = cJSON_Print(root);
        printf("%s\n", json_string);
        free(json_string);
        cJSON_Delete(root);
    }
}

void trim(char *s){
    int i = strlen(s) - 1;
    while(i > 0){
        if (s[i] == ' ' || s[i] == '\n' || s[i] == '\t'){
            i--;
        }else{
            break;
        }
    }
    s[i + 1] = '\0';
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