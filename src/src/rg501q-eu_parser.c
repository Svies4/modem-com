#include "rg501q-eu_parser.h"

// Parser for AT+QTEMP info (+QTEMP:"key","value")
int parse_qtemp_rg501qeu(const char *line, ModemData *data)
{
    char key[64] = {0};
    char val[32] = {0};
    
    if(sscanf(line, "+QTEMP:\"%63[^\"]\",\"%31[^\"]\"", key, val) == 2) {
        
        if (strcmp(val, "-273") != 0) {
            size_t needed_len = snprintf(NULL, 0, "%s: %s°C", key, val) + 1;
            list_add(&data->temperatures, needed_len);
            
            snprintf(data->temperatures.data[data->temperatures.count], needed_len, 
                     "%s: %s°C", key, val);
                     
            data->temperatures.count++;
        }
        return 0;
    }
    return -1;
}