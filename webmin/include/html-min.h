#ifndef WEBMIN_HTML_MIN_H
#define WEBMIN_HTML_MIN_H

#include <stddef.h>
#include <stdint.h>

#include "webmin_general.h"

#define WEBMIN_HTML_TAG_SELF_CLOSE  (1 << 1) // like <div/> (not valid html but still useful information)
#define WEBMIN_HTML_TAG_SINGLE      (1 << 2) // like <br> or <br/>, they don't have a body
#define WEBMIN_HTML_TAG_CLOSE       (1 << 3) // </div>
#define WEBMIN_HTML_PRESERVE_SPACE  (1 << 4)

typedef struct {
    char *type;
    size_t type_length;
    uint8_t flags;
} webmin_html_tag;

webmin_html_tag webmin_html_tag_extractor(webmin_context_t *ctx, char *s, const char *delims[], uint8_t flags);
void webmin_html_inner(webmin_context_t *ctx, char *s, const char *delims[], uint8_t flags, const char *end_tag, size_t end_tag_length);
void webmin_html(webmin_context_t *ctx, char *s, const char *delims[], uint8_t flags);

#endif