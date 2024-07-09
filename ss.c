#include <aio.h>
#include <string.h>
#include <stdlib.h>

#include "ss.h"

size_t len(const char* string) {

    char* start = (char*)string;
    while(*start != '\0') start++;

    return start - string;
}

StringStruct ss_form_string(const char* string, size_t len) {

    StringStruct ss;
    ss.data = string;
    ss.count = len;

    return ss;
}

StringStruct ss_form_string_nt(const char* cstring) {

    return ss_form_string(cstring, len(cstring));
}

StringStruct ss_trim_left(StringStruct string) {

    size_t i = 0;
    while(i < string.count && *(string.data + i) == ' ') i++;

    return ss_form_string(string.data + i, string.count - i);
}

StringStruct ss_trim_right(StringStruct string) {

    size_t i = 0;
    while(i < string.count && *(string.data + string.count - i - 1) == ' ') i++;

    return ss_form_string(string.data, string.count - i);
}

StringStruct ss_trim(StringStruct string) {

    return ss_trim_left(ss_trim_right(string));
}

StringStruct ss_cut_by_delim(StringStruct* string, const char delimiter) {

    size_t i = 0;
    while(i < string->count && *(string->data + i) != delimiter) i++; 
    
    StringStruct ret = ss_form_string(string->data, i);

    if(i == string->count) {

        string->data += i - 1;
        string->count = 0;

        return ret;
    }

    string->data += i + 1;
    string->count -= i + 1;

    return ret;
} 

StringStruct ss_copy_by_delim(StringStruct string, const char delimiter) {

    size_t i = 0;
    while(i < string.count && *(string.data + i) != delimiter) i++;

    return ss_form_string(string.data, i);
}



/*
    Noteworthy:

    Negative n values cause the size_t to overflow due to it being an unsigned value. The entire SS will be cut.
    If n is larger than the SS size, the whole SS will be cut.
*/

StringStruct ss_cut_n(StringStruct* ss, size_t n) {

    size_t l = n > ss->count ? ss->count : n;

    StringStruct ret = ss_form_string(ss->data, l);

    ss->data += l;
    ss->count -= l;

    return ret;
}



/*
    Noteworthy:

    Negative n values cause the size_t to overflow due to it being an unsigned value. The entire SS will be copied.
    If n is larger than the SS size, the whole SS will be copied.
*/

StringStruct ss_copy_n(StringStruct ss, size_t n) {

    size_t l = n > ss.count ? ss.count : n;

    return ss_form_string(ss.data, l);
}

bool c_isdigit(char character) {

    return (character >= '0' && character <= '9');
}

bool c_islower(char character) {

    return character >= 'a' && character <= 'z';
}

bool c_isupper(char character) {

    return character >= 'A' && character <= 'Z';
}

bool c_isletter(char character) {

    return c_islower(character) || c_isupper(character);
}

bool c_isalnum(char character) {

    return c_isdigit(character) || c_isletter(character);
}

bool ss_isword(StringStruct ss) {

    for(int i = 0; i < ss.count; i++) {

        if(!c_isletter(c_charat(&ss, i))) return false;
    }
    return true;
}

bool ss_isnumber(StringStruct ss) {

    size_t periods = 0;

    for(int i = 0; i < ss.count; i++) {

        if(c_charat(&ss, i) == '.') {

            periods++;
            
            if(periods > 1) return false;
            continue;
        }

        if(!c_isdigit(c_charat(&ss, i))) return false;
    }
    return true;
}

char c_tolower(char character) {

    if(c_isupper(character)) return character + 32;
    return character;
}

char c_toupper(char character) {

    if(c_islower(character)) return character - 32;
    return character;
}

double ss_tod(StringStruct ss) {

    char buffer[ss.count + 1];
    strncpy(buffer, ss.data, ss.count);
    buffer[ss.count] = '\0';

    return strtod(buffer, NULL);

}

float ss_tof(StringStruct ss) {

    char buffer[ss.count + 1];
    strncpy(buffer, ss.data, ss.count);
    buffer[ss.count] = '\0';

    return strtof(buffer, NULL);

}

bool ss_is_empty(StringStruct* ss) {

    return ss->count == 0;
}

bool ss_starts_with(StringStruct* ss, const char character) {

    if(ss->count == 0) return false;
    return *(ss->data) == character;
}

bool ss_ends_with(StringStruct* ss, const char character) {

    if(ss->count == 0) return false;
    return *(ss->data + ss->count - 1) == character;
}

bool ss_cmp(StringStruct* ss1, StringStruct* ss2) {

    if(ss1->count != ss2->count) return false;

    for(size_t i = 0; i < ss1->count; i++) {

        if(*(ss1->data + i) != *(ss2->data + i)) return false;
    }
    return true;
}

bool ss_cmp_ignorecase(StringStruct* ss1, StringStruct* ss2) {

    if(ss1->count != ss2->count) return false;

    for(size_t i = 0; i < ss1->count; i++) {

        if(c_tolower(*(ss1->data + i)) != c_tolower(*(ss2->data + i))) return false;
    }
    return true;
}

bool ss_cmp_cstr(StringStruct* ss, const char* cstr) {

    if(ss->count != strlen(cstr)) return false;

    for(int i = 0; i < ss->count; i++) {

        if(c_charat(ss, i) != cstr[i]) return false;
    }

    return true;
}

int c_find(StringStruct* ss, char character) {

    int i = 0;
    while(i < ss->count && *(ss->data + i) != character) i++;
    return i == ss->count? -1 : i;
}

char c_charat(StringStruct* ss, int index) {

    if(index < 0 || index >= ss->count) return '\0';

    return *(ss->data + index);
}