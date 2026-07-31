#include "webmin_general.h"

uint8_t webmin_string_equals_ignore_case_with_length(const char *x, const char *y, size_t n1, size_t n2) {
    size_t i;
    if (n1 != n2)
        return 0;
    for (i = 0; i < n1; i++) {
        if (tolower(x[i]) != tolower(y[i])) {
            printf("(\"%.*s\" == \"%.*s\") == %d\n", (int)n1, x, (int)n2, y, 0);
            return 0;
        }
    }
    printf("(\"%.*s\" == \"%.*s\") == %d\n",
        (int)n1, x,
        (int)n2, y,
        1// || n1 == n2
    );
    //return x[i] == '\0' || y[i] == '\0';// || n1 == n2;
    return 1;
}