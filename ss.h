/*
    Author: Aleksa Djurdjevic

    Github repo: https://www.github.com/djurdjevicaleksa/ss

    Last modified: 10.7.2024.

*/

/*
    ---------------------------------------------------------------------------------------
    TO USE THIS LIBRARY, DEFINE "_SS_IMPLEMENT" IN YOUR SOURCE CODE AND THEN INCLUDE THIS HEADER
    ---------------------------------------------------------------------------------------
*/

#ifndef _SS_H
#define _SS_H

#include <aio.h>
#include <string.h>
#include <stdbool.h>

typedef struct {
    const char* data;
    size_t count;
} StringStruct;

#define SS(string_literal) ss_form_string(string_literal, sizeof(string_literal) - 1)
#define SSC(cstring) ss_form_string_nt(cstring)

#define SSFormat "%.*s"
#define SSArg(ss) (int)ss.count, ss.data 

/*
    Calculates the length of a null terminated string.

    @param string Input string
    @return Length of the string.
*/
size_t len(const char* string);

/*
    Initializes an SS struct. Used internally as well as via SS() macro.

    @param string String text.
    @param len Length of the string.

    @return SS.
*/
StringStruct ss_form_string(const char* string, size_t len);

/*
    Initializes an SS struct using a cstring as input.

    @param cstring Input cstring.
    @return SS.
*/
StringStruct ss_form_string_nt(const char* cstring);

/*
    Trims whitespaces from the beginning of the SS without modifying it.

    @param ss Input SS.
    @return Trimmed SS. 
*/
StringStruct ss_trim_left(StringStruct ss);

/*
    Trims whitespaces from the end of the SS without modifying it.

    @param ss Input SS.
    @return Trimmed SS.
*/
StringStruct ss_trim_right(StringStruct ss);

/*
    Trims whitespaces from the beginning and end of the SS without modifying it.

    @param ss Input SS.
    @return Trimmed SS.
*/
StringStruct ss_trim(StringStruct ss);

/*
    Cuts characters from the beginning to where it finds the first occurrence of the delimiter. Modifies input SS and returns the cut part.

    @param ss Input SS.
    @param delimiter The separator.
    @return Cut off part of the SS.
*/
StringStruct ss_cut_by_delim(StringStruct* ss, const char delimiter); 

/*
    Copies characters from the beginning to where it finds the first occurrence of the delimiter. Doesn't modify input SS and returns the copy.

    @param ss Input SS.
    @param delimiter The separator.
    @return Copied part of the SS.
*/
StringStruct ss_copy_by_delim(StringStruct ss, const char delimiter);

/*
    Cuts a number of characters from the beginning of the input SS. Modifies the input SS and returns the cut part.

    @param ss Input SS.
    @param n Number of characters to be cut.
    @return Cut off part of the SS.
*/
StringStruct ss_cut_n(StringStruct* ss, size_t n);

/*
    Copies a number of characters from the beginning of the input SS. Returns the copy and doesn't modify the input SS.

    @param ss Input SS.
    @param n Number of characters to be copied.
    @return Copied part of the SS.
*/
StringStruct ss_copy_n(StringStruct ss, size_t n);

/*
    Appends addition to the base SS. Counterintuitively doesn't modify in place.

*/
StringStruct ss_append(StringStruct base, StringStruct addition);

/*
    Figures out if the character is a digit (0 - 9).

*/
bool c_isdigit(char character);

/*
    Figures out if the character is a lowercase letter (a - z).

*/
bool c_islower(char character);

/*
    Figures out if the character is an uppercase letter (A - Z).

*/
bool c_isupper(char character);

/*
    Figures out if the character is a letter (a - z, A - Z).

*/
bool c_isletter(char character);

/*
    Figures out if the character is a digit (0 - 9) or a letter (a - z, A - Z).

*/
bool c_isalnum(char character);

/*
    Figures out if an SS is a valid word or not.
*/
bool ss_isword(StringStruct ss);

/*
    Figures out if an SS is a valid number or not. Supports integers and floating point numbers.
*/
bool ss_isnumber(StringStruct ss);

/*
    Figures out if a character is a mathematical operator or not (+, -, *, /, ^).
*/
bool c_isoperator(char c);

/*
    Converts a character to lowercase.
*/
char c_tolower(char character);

/*
    Converts a character to uppercase.
*/
char c_toupper(char character);

/*
    Converts an SS to a double. Use with ss_isnumber() to ensure validity.
*/
double ss_tod(StringStruct ss);

/*
    Converts an SS to a float. Use with ss_isnumber() to ensure validity.
*/
float ss_tof(StringStruct ss);

/*
    Figures out if an SS is empty or not.
*/
bool ss_is_empty(StringStruct* ss);

/*
    Figures out if an SS starats with the specified character or not.
*/
bool ss_starts_with(StringStruct* ss, const char character);

/*
    Figures out if an SS ends with the specified character or not.
*/
bool ss_ends_with(StringStruct* ss, const char character);

/*
    Figures out if provided SS are equal or not. Case sensitive.
*/
bool ss_cmp(StringStruct* ss1, StringStruct* ss2);

/*
    Figures out if provided SS are equal or not. Not case sensitive.
*/
bool ss_cmp_ignorecase(StringStruct* ss1, StringStruct* ss2);

/*
    Figures out if the SS equals the cstring. Case sensitive.
*/
bool ss_cmp_cstr(StringStruct* ss, const char* cstr);

/*
    Finds the first occurrence of the specified character in the SS and returns its index.

    @return Returns -1 if the SS doesn't contain specified character, otherwise its index.
*/
int c_find(StringStruct* ss, char character);

/*
    Finds the character at the specified index.

    @return Returns null-terminating character if the index is negative or outside the SS, otherwise returns the character.
*/
char c_charat(StringStruct* ss, int index);


#endif //_SS_H


/*
    ---------------------------------------------------------------------------------------
    TO USE THIS LIBRARY, DEFINE "_SS_IMPLEMENT" IN YOUR SOURCE CODE AND THEN INCLUDE THIS HEADER
    ---------------------------------------------------------------------------------------
*/



#ifdef _SS_IMPLEMENT

#include <aio.h>
#include <string.h>
#include <stdlib.h>

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

StringStruct ss_append(StringStruct base, StringStruct addition) {

    char buffer[base.count + addition.count + 1];
    memset(buffer, '\0', sizeof(buffer));

    memcpy(buffer, base.data, base.count);
    memcpy(buffer + base.count, addition.data, addition.count);
    
    return ss_form_string_nt(buffer);
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

    if(ss.count == 0) return false;

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

bool c_isoperator(char c) {

    return (c == '+' || c == '-' || c == '*' || c == '/' || c == '^' || c == '=' || c == '(' || c == ')');
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

#endif