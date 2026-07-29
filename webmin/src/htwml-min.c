#include "html-min.h"
#include "webmin_general.h"
#include <stddef.h>

enum html_tag_status {
    hts_begin,          // <█ or </█
    hts_type,           // <type█
    hts_void,           // <type █ or <type key="value"█ <───┐
    hts_key,            // <type key█ <──────────┐           │
    hts_key_after,      // <type key █ ──────────┘           │
    hts_assign,         // <type key=█ or <type key = █      │
    hts_value           // <type key="█ ─────────────────────┘
};

enum html_value_type {
    hvt_static,
    hvt_js,
    hvt_css,
    hvt_class
};

void webmin_html_process_value(webmin_context_t *ctx, uint8_t value_type) {
    // not minifying js nor css until i implemented the minifier for them
    if (value_type == hvt_static || value_type == hvt_js || value_type == hvt_css) {
        
    }
}

webmin_html_tag webmin_html_tag_extractor(webmin_context_t *ctx, char *s, const char *delims[], uint8_t flags) {
    webmin_html_tag tag = (webmin_html_tag){
        .type = NULL,
        .type_length = 0,
        .flags = 0
    };

    if (s[ctx->processed_length] != '<') {
        ctx->error = webmin_err_html_tag_invalid_start;
        return tag;
    }
    s[ctx->output_length++] = '<';
    if (s[++ctx->processed_length] == '/') {
        tag.flags |= WEBMIN_HTML_TAG_CLOSE;
        ctx->processed_length++;
    }

    uint8_t state = hts_begin;
    size_t last_identifier_begin = 0, last_identifier_end = 0;

    while (s[ctx->processed_length] != '\0' && ctx->processed_length < ctx->max_length) {
        char c = s[ctx->processed_length];

        // hopefully every compiler is smart enough to apply this 'simple' optimization, switch-case is terrible
        if (state == hts_begin) {
            // https://www.w3schools.com/xml/xml_elements.asp
            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') {
                s[ctx->output_length] = c;
                last_identifier_begin = ctx->output_length;
                tag.type = s + ctx->output_length++;
                
                state = hts_type;
            } else {
                ctx->error = webmin_err_html_tag_invalid_type_begin;
                return tag;
            }
        } else if (state == hts_type) {
            // https://www.w3schools.com/xml/xml_elements.asp
            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.' || c == ':') {
                s[ctx->output_length++] = c;
            } else if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
                tag.type_length = ctx->output_length - last_identifier_begin;
                s[ctx->output_length++] = ' ';
                state = hts_void;
            } else if (c == '/') {
                if ((tag.flags & WEBMIN_HTML_TAG_CLOSE) || (tag.flags & WEBMIN_HTML_TAG_SELF_CLOSE)) {
                    ctx->error = webmin_err_html_tag_invalid_self_close;
                    return tag;
                }
                tag.flags |= WEBMIN_HTML_TAG_SELF_CLOSE;
                s[ctx->output_length++] = '/';
            } else if (c == '>') {
                s[ctx->output_length++] = '>';
                // increment ctx->processed_length? or do it after it reutrn? i'll try the latter
                return tag;
            } else {
                ctx->error = webmin_err_html_tag_invalid_type;
                return tag;
            }
        } else if (state == hts_void) {
            switch (c) {case' ':case'\t':case'\n':case'\r': {
                // ignore extra spacing
            } break; case'=':case'"':case'\'': {
                ctx->error = webmin_err_html_tag_invalid_name_character;
                return tag;
            } case'/': {
                if ((tag.flags & WEBMIN_HTML_TAG_CLOSE) || (tag.flags & WEBMIN_HTML_TAG_SELF_CLOSE)) {
                    ctx->error = webmin_err_html_tag_invalid_self_close;
                    return tag;
                }
                tag.flags |= WEBMIN_HTML_TAG_SELF_CLOSE;
                s[ctx->output_length++] = '/';
            } break; case'>': {
                s[ctx->output_length++] = '>';
                // same thing here
                return tag;
            } default: {
                s[ctx->output_length] = c;
                last_identifier_begin = ctx->output_length++;

                state = hts_key;
            } break;}
        } else if (state == hts_key) {
            switch (c) {case' ':case'\t':case'\n':case'\r': {
                last_identifier_end = ctx->output_length;
                s[ctx->output_length++] = ' ';
                state = hts_key_after;
            } break; case'"':case'\'': {
                ctx->error = webmin_err_html_tag_invalid_name_character;
                return tag;
            } case'/': {
                if ((tag.flags & WEBMIN_HTML_TAG_CLOSE) || (tag.flags & WEBMIN_HTML_TAG_SELF_CLOSE)) {
                    ctx->error = webmin_err_html_tag_invalid_self_close;
                    return tag;
                }
                tag.flags |= WEBMIN_HTML_TAG_SELF_CLOSE;
                s[ctx->output_length++] = '/';
            } break; case'>': {
                s[ctx->output_length++] = '>';
                // same thing here
                return tag;
            } case'=': {
                s[ctx->output_length++] = '=';
                state = hts_assign;
            } break; default: {
                s[ctx->output_length] = c;
                last_identifier_begin = ctx->output_length++;

                state = hts_key;
            } break;}
        } else if (state == hts_key_after) {
            switch (c) {case' ':case'\t':case'\n':case'\r': {
                // ignore extra spacing
            } break; case'"':case'\'': {
                ctx->error = webmin_err_html_tag_equal_sign_expected_before_value;
                return tag;
            } case'/': {
                if ((tag.flags & WEBMIN_HTML_TAG_CLOSE) || (tag.flags & WEBMIN_HTML_TAG_SELF_CLOSE)) {
                    ctx->error = webmin_err_html_tag_invalid_self_close;
                    return tag;
                }
                tag.flags |= WEBMIN_HTML_TAG_SELF_CLOSE;
                s[ctx->output_length++] = '/';
            } break; case'>': {
                s[ctx->output_length++] = '>';
                // same thing here
                return tag;
            } case'=': {
                s[ctx->output_length - 1] = '=';
                state = hts_assign;
            } break; default: {
                s[ctx->output_length] = c;
                last_identifier_begin = ctx->output_length++;

                state = hts_key;
            } break;}
        } else if (state == hts_assign) {
            switch (c) {case' ':case'\t':case'\n':case'\r': {
                // ignore extra spacing
            } break; case'=':case'/':case'>': {
                ctx->error = webmin_err_html_tag_invalid_name_character;
                return tag;
            } case'\'':case'"': {
                s[ctx->output_length++] = c;

                state = hts_value;
            } break; default: {
                s[ctx->output_length++] = c;

                state = hts_value;
            } break;}
        }

        ctx->processed_length++;
    }
}

void webmin_html_inner(webmin_context_t *ctx, char *s, const char *delims[], size_t max_length, uint8_t flags) {
    
}

void webmin_html(webmin_context_t *ctx, char *s, const char *delims[], size_t max_length, uint8_t flags) {

}