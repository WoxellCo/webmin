#include "webmin_general.h"

uint8_t webmin_string_equals_ignore_case_with_length(const char *x, const char *y, size_t n) {
    size_t i;
    for (i = 0; i < n; i++) {
        if (tolower(x[i]) != tolower(y[i]))
            return 0;
    }
    //return x[i] == '\0' || y[i] == '\0';
    return 1;
}