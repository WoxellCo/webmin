#ifndef WEBMIN_DEFS_H
#define WEBMIN_DEFS_H

#include <stdio.h> ///// debug
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#define WEBMIN_INCLUDE_XML_ESCAPE_TOKEN     (1 << 0)
#define WEBMIN_INCLUDE_PHP                  (1 << 1)

typedef enum {
    webmin_err_html_tag_invalid_start,
    webmin_err_html_tag_invalid_type_begin,
    webmin_err_html_tag_invalid_type,
    webmin_err_html_tag_invalid_self_close,
    webmin_err_html_tag_invalid_name_character,
    webmin_err_html_tag_equal_sign_expected_before_value,
    webmin_err_html_tag_closes_nothing,
    webmin_err_html_tag_unmatched_close,
} webmin_err_info_t;

typedef struct {
    size_t processed_length;
    size_t output_length;
    webmin_err_info_t error;
    size_t max_length;
} webmin_context_t;

uint8_t webmin_string_equals_ignore_case_with_length(const char *x, const char *y, size_t n1, size_t n2);

#endif