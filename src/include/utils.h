#ifndef UTILS_H_
#define UTILS_H_

#include <stdlib.h>
#include <stdio.h>
#include <cJSON.h>
#include <string.h>
#include "modem_types.h"
#include "arg.h"

typedef enum {
    TYPE_STRING,
    TYPE_INT,
    TYPE_SMS,
    TYPE_CELLS,
    TYPE_FLOAT,
    TYPE_LIST
} DataType;

typedef struct {
    const int *show_flag;
    const char *json_key;
    const char *text_label;
    DataType type;
    const void *data_ptr;
} PrintRule;

void print_results(const ModemData *data, struct arguments *req);
void trim (char *s);
void list_add(List *list, size_t needed_len);

#endif