#include "webmin_general.h"
#include <stdio.h>
#include <stdlib.h>
#include <webmin.h>

int main() {
    FILE *f = fopen("index.html", "rb");

    size_t sz = fseek(f, 0L, SEEK_END);
    sz = ftell(f);
    rewind(f);

    char *content = (char *)malloc((sz + 1) * sizeof(char));

    size_t length = fread(content, 1, sz, f);

    fclose(f);

    webmin_context_t ctx = (webmin_context_t){
        .max_length = length,
        .output_length = 0,
        .processed_length = 0,
        .error = 0
    };

    webmin_html(&ctx, content, (const char *[]){NULL}, 0);
    printf("OUTPUT_LENGTH: %zu\n", ctx.output_length);
    printf("OUTPUT_ERROR: %u\n", ctx.error);
    content[ctx.output_length] = '\0';

    f = fopen("index-min.html", "w");
    fprintf(f, "%s", content);
    fclose(f);
    free(content);

    printf("%d\n",
        webmin_string_equals_ignore_case_with_length("CCC", "ccc", 3)
    );
}