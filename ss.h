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