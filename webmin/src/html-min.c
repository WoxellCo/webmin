#include "html-min.h"
#include "webmin_general.h"
#include <stdint.h>

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
    hvt_list
};

uint8_t webmin_html_is_tag_empty(const char *type, size_t type_length) {
    // https://www.tutsinsider.com/html/html-empty-elements/
    return webmin_string_equals_ignore_case_with_length(type, "area", type_length, SIZE_MAX)
        || webmin_string_equals_ignore_case_with_length(type, "base", type_length, SIZE_MAX)
        || webmin_string_equals_ignore_case_with_length(type, "basefont", type_length, SIZE_MAX)
        || webmin_string_equals_ignore_case_with_length(type, "bgsound", type_length, SIZE_MAX)
        || webmin_string_equals_ignore_case_with_length(type, "br", type_length, SIZE_MAX)
        || webmin_string_equals_ignore_case_with_length(type, "col", type_length, SIZE_MAX)
        || webmin_string_equals_ignore_case_with_length(type, "command", type_length, SIZE_MAX)
        || webmin_string_equals_ignore_case_with_length(type, "embed", type_length, SIZE_MAX)
        || webmin_string_equals_ignore_case_with_length(type, "frame", type_length, SIZE_MAX)
        || webmin_string_equals_ignore_case_with_length(type, "hr", type_length, SIZE_MAX)
        || webmin_string_equals_ignore_case_with_length(type, "img", type_length, SIZE_MAX)
        || webmin_string_equals_ignore_case_with_length(type, "input", type_length, SIZE_MAX)
        || webmin_string_equals_ignore_case_with_length(type, "isindex", type_length, SIZE_MAX)
        || webmin_string_equals_ignore_case_with_length(type, "keygen", type_length, SIZE_MAX)
        || webmin_string_equals_ignore_case_with_length(type, "link", type_length, SIZE_MAX)
        || webmin_string_equals_ignore_case_with_length(type, "meta", type_length, SIZE_MAX)
        || webmin_string_equals_ignore_case_with_length(type, "nextid", type_length, SIZE_MAX)
        || webmin_string_equals_ignore_case_with_length(type, "param", type_length, SIZE_MAX)
        || webmin_string_equals_ignore_case_with_length(type, "plaintext", type_length, SIZE_MAX)
        || webmin_string_equals_ignore_case_with_length(type, "source", type_length, SIZE_MAX)
        || webmin_string_equals_ignore_case_with_length(type, "track", type_length, SIZE_MAX)
        || webmin_string_equals_ignore_case_with_length(type, "wbr", type_length, SIZE_MAX)
    ;
}

uint8_t webmin_html_get_value_type(const char *key, size_t key_length) {
    if (webmin_string_equals_ignore_case_with_length(key, "class", key_length, SIZE_MAX)) {
        return hvt_list;
    } else if (webmin_string_equals_ignore_case_with_length(key, "style", key_length, SIZE_MAX)) {
        return hvt_css;
    } else if (webmin_string_equals_ignore_case_with_length(key, "on", 2, SIZE_MAX)) {
        return hvt_js;
    } return hvt_static;
}

void webmin_html_process_value(webmin_context_t *ctx, char *s, uint8_t value_type) {
    char end_token = s[ctx->processed_length];
    if (end_token != '\'' && end_token != '"')
        end_token = '\0';
    else {
        s[ctx->output_length++] = end_token;
        ctx->processed_length++;
    }

    // not minifying js nor css until i implemented the minifier for them
    if (value_type == hvt_static || value_type == hvt_js || value_type == hvt_css) {
        while (s[ctx->processed_length] != end_token && s[ctx->processed_length] != '\0') {
            char c = s[ctx->processed_length];
            if (end_token == '\0') {
                if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
                    break;
                else if (c == '/' || c == '>')
                    break;
            }

            s[ctx->output_length++] = c;
            ctx->processed_length++;
        }
    } else if (value_type == hvt_list) {
        uint8_t sp = 0;
        while (s[ctx->processed_length] != end_token && s[ctx->processed_length] != '\0') {
            char c = s[ctx->processed_length];
            if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
                if (end_token == '\0')
                    return;

                if (sp) {
                    s[ctx->output_length++] = ' ';
                    sp = 0;
                }
                ctx->processed_length++;
                continue;
            } else if ((c == '/' || c == '>') && end_token == '\0')
                return;
            sp = 1;
            s[ctx->output_length++] = c;
            ctx->processed_length++;
        }
    }
    s[ctx->output_length++] = s[ctx->processed_length];
    //ctx->processed_length++;
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
        s[ctx->output_length++] = '/';
        ctx->processed_length++;
    }

    uint8_t state = hts_begin;
    size_t last_identifier_begin = 0, last_identifier_end = 0;

    uint8_t sp = 1;
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
                tag.type_length = ctx->output_length - last_identifier_begin;
                tag.flags |= WEBMIN_HTML_TAG_SELF_CLOSE;
                s[ctx->output_length++] = '/';
            } else if (c == '>') {
                tag.type_length = ctx->output_length - last_identifier_begin;
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
                if (sp) {
                    s[ctx->output_length++] = ' ';
                    sp = 0;
                }
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
            sp = 1;
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
                s[ctx->output_length] = '=';
                last_identifier_end = ctx->output_length++;
                state = hts_assign;
            } break; default: {
                s[ctx->output_length++] = c;
                //last_identifier_begin = ctx->output_length++;

                //state = hts_key;
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
                ctx->error = webmin_err_html_tag_invalid_name_character; // wrong error: value expected instead of close or another equal sign
                return tag;
            } case'\'':case'"':default: {
                //s[ctx->output_length++] = c;
                state = hts_value;
                webmin_html_process_value(ctx, s, webmin_html_get_value_type(s + last_identifier_begin, last_identifier_end - last_identifier_begin));
                state = hts_void;
            } break;}
        }

        ctx->processed_length++;
    }

    return tag;
}

void webmin_html_inner(webmin_context_t *ctx, char *s, const char *delims[], uint8_t flags, const char *end_tag, size_t end_tag_length) {
    printf("TAG OPEN: \"%.*s\"\n", (int)end_tag_length, end_tag);
    uint8_t sp = 0, contains_text = 0;
    while (s[ctx->processed_length] != '\0' && ctx->processed_length < ctx->max_length) {
        for (size_t i = 0; delims[i] != NULL; i++) {
            size_t tok_len = strlen(delims[i]);
            if (!strncmp(s + ctx->processed_length, delims[i], tok_len)) {
                //ctx->processed_length += tok_len;
                return;
            }
        }

        char c = s[ctx->processed_length];

        if (c == '<') {
            if (ctx->output_length > 0 && s[ctx->output_length - 1] == ' ' && !contains_text)
                ctx->output_length--;
            sp = contains_text;
            char c = s[ctx->processed_length + 1];
            if (c == '/') {
                c = s[ctx->processed_length + 2];
            }
            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') {
                webmin_html_tag tag = webmin_html_tag_extractor(ctx, s, delims, 0);
                //ctx->processed_length++;
                printf("<%s%.*s>\n", tag.flags & WEBMIN_HTML_TAG_CLOSE ? "/" : "", (int)tag.type_length, tag.type);
                if (tag.flags & WEBMIN_HTML_TAG_CLOSE) {
                    printf("TAG CLOSE: \"%.*s\"\n", (int)end_tag_length, end_tag);
                    if (end_tag == NULL) {
                        ctx->error = webmin_err_html_tag_closes_nothing;puts("ERR");
                    } else if (!webmin_string_equals_ignore_case_with_length(tag.type, end_tag, tag.type_length, end_tag_length)) {
                        ctx->error = webmin_err_html_tag_unmatched_close;puts("ERR7");
                    }
                    return;
                } else if (webmin_string_equals_ignore_case_with_length(tag.type, "pre", tag.type_length, 3)) {
                    webmin_html_inner(ctx, s, delims, WEBMIN_HTML_PRESERVE_SPACE, "pre", 3);
                } else if (!webmin_html_is_tag_empty(tag.type, tag.type_length)) { // check if it's not empty tag like <br/>
                    ctx->processed_length++;
                    webmin_html_inner(ctx, s, delims, flags, tag.type, tag.type_length);
                }
                //continue;
            }
        } else if ((c == ' ' || c == '\n' || c == '\t') && !(flags & WEBMIN_HTML_PRESERVE_SPACE)) {
            if (sp) {
                sp = 0;
                s[ctx->output_length++] = ' ';
            }
        } else if (c == '\r' && !(flags & WEBMIN_HTML_PRESERVE_SPACE)) {

        } else {
            contains_text = 1;
            sp = 1;
            s[ctx->output_length++] = c;
        }

        ctx->processed_length++;
    }
}

void webmin_html(webmin_context_t *ctx, char *s, const char *delims[], uint8_t flags) {
    //webmin_html_tag tag = webmin_html_tag_extractor(ctx, s, delims, 0);
    //printf("TAG: <%.*s>\n", (int)tag.type_length, tag.type);

    webmin_html_inner(ctx, s, delims, 0, NULL, 0);
}