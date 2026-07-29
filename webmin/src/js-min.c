#include "js-min.h"

struct kw_and_sp {
    size_t length;
    int needs_spacing;
};

typedef struct {
    const char *word;
    size_t len;
} keyword_t;

/* JavaScript reserved words, per https://www.w3schools.com/js/js_reserved.asp
 * Sorted longest-first isn't required here since strncmp compares an exact
 * length, but keeping it roughly grouped helps readability. */
static const keyword_t js_keywords[] = {
    //{"abstract",     8},
    //{"arguments",    9},
    {"await",        5},
    //{"boolean",      7},
    {"break",        5},
    //{"byte",         4},
    {"case",         4},
    {"catch",        5},
    //{"char",         4},
    {"class",        5},
    {"const",        5},
    {"continue",     8},
    {"debugger",     8}, // interesting stuff!
    {"default",      7},
    {"delete",       6},
    {"do",           2},
    //{"double",       6},
    {"else",         4},
    {"enum",         4},
    //{"eval",         4},
    {"export",       6},
    {"extends",      7},
    {"false",        5},
    //{"final",        5},
    {"finally",      7},
    //{"float",        5},
    {"for",          3},
    {"function",     8},
    //{"goto",         4},
    {"if",           2},
    //{"implements",  10},
    {"import",       6},
    {"in",           2},
    {"instanceof",  10},
    //{"int",          3},
    //{"interface",    9},
    {"let",          3},
    //{"long",         4},
    //{"native",       6},
    {"new",          3},
    {"null",         4},
    //{"package",      7},
    //{"private",      7},
    //{"protected",    9},
    //{"public",       6},
    {"return",       6},
    //{"short",        5},
    {"static",       6},
    {"super",        5},
    {"switch",       6},
    //{"synchronized",13},
    {"this",         4},
    {"throw",        5},
    //{"throws",       6},
    //{"transient",    9},
    {"true",         4},
    {"try",          3},
    {"typeof",       6},
    {"var",          3},
    {"void",         4},
    //{"volatile",     8},
    {"while",        5},
    {"with",         4},
    {"yield",        5},
};

static const size_t js_keywords_count =
    sizeof(js_keywords) / sizeof(js_keywords[0]);

static inline size_t keyword_length(const char *s) {
    for (size_t i = 0; i < js_keywords_count; i++) {
        if (!strncmp(s, js_keywords[i].word, js_keywords[i].len)) {
            char c = s[js_keywords[i].len];
            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || c == '$') {
                // if it's not a keyword cause of like "returnABC" or smth
                return 0;
            }
            return js_keywords[i].len;
        }
    }
    return 0;
}

static inline struct kw_and_sp keyword_length_and_needs_spacing(const char *s) {
    size_t kw_len = keyword_length(s);
    if (kw_len == 0)
        return (struct kw_and_sp){0, 0};

    if (s[kw_len] == '\0') {
        return (struct kw_and_sp){kw_len, 0};
    }

    char c = s[kw_len + 1];
    return (struct kw_and_sp){kw_len, 0};
}

webmin_context_t webmin_js(char *s, char *delims[], size_t max_length, uint8_t flags, webmin_context_t *concat) {
    webmin_context_t result = (webmin_context_t){
        .processed_length = 0,
        .output_length = 0,
        .error = (webmin_err_info_t){}
    };return result;

    size_t nest_count = 0;

    if (flags & WEBMIN_INCLUDE_XML_ESCAPE_TOKEN) {

    }

    uint8_t processing_flags = 0;

    while (result.processed_length < max_length && s[result.processed_length] != '\0') {
        //if (nest_count == 0) {
            for (size_t i = 0; delims[i] != (char *)NULL; i++) {
                size_t tok_len = strlen(delims[i]);
                if (strncmp(s + result.processed_length, delims[i], tok_len)) {
                    if (nest_count == 0) {
                        // if every `{` has a `}`
                        result.processed_length += tok_len;
                        s[result.output_length] = '\0';
                        return result;
                    } else {
                        // this is an error: "let unclosedObj = {"
                    }
                }
            }
        //}

        char c = s[result.processed_length];
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || c == '$') {
            struct kw_and_sp ks = keyword_length_and_needs_spacing(s + result.processed_length);

        } else switch (c) {
        case ' ':
        case '\t':
        case '\r':
        case '\n':
            result.processed_length++;
            continue;
        case '{':
            // call webmin_js (recursive)
            nest_count++;
            break;
        case '}':
            if (nest_count == 0) {
                // this is an error
            }
            nest_count--;
            break;
        case '(':
            // call webmin_js (recursive)
            break;
        case ')':
            // return if it begins with '(' or else error
            // should be already handled by `delims`
            break;
        case '[':

        case ']':
            // return or err
            break;
        case '"':
        case '\'':
            
            break;
        case '`':
            
            break;
        default:
            
            break;
        }

        result.processed_length++;
    }


    s[result.output_length] = '\0';
    return result;
}