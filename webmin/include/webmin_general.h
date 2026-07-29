#ifndef WEBMIN_DEFS_H
#define WEBMIN_DEFS_H

#include <stddef.h>
#include <string.h>

#define WEBMIN_INCLUDE_XML_ESCAPE_TOKEN     (1 << 0)
#define WEBMIN_INCLUDE_PHP                  (1 << 1)

typedef enum {
    webmin_err_html_tag_invalid_start,
    webmin_err_html_tag_invalid_type_begin,
    webmin_err_html_tag_invalid_type,
    webmin_err_html_tag_invalid_self_close,
    webmin_err_html_tag_invalid_name_character,
    webmin_err_html_tag_equal_sign_expected_before_value,
} webmin_err_info_t;

typedef struct {
    size_t processed_length;
    size_t output_length;
    webmin_err_info_t error;
    size_t max_length;
} webmin_context_t;

#endif