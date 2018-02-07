#ifndef SHELPERS_H_
#define SHELPERS_H_

#include <stdint.h>

/**
 * Clones the given string, malloc'ing a new one.
 * @param s the string to clone
 * @return the malloc'd copy
 */
char *strclone(const char *s);

/**
 * Clones the given string until the given count.
 * @param s the string to clone
 * @param n the number of characters to clone
 * @return the malloc'd copy
 */
char *strnclone(const char *s, uint32_t n);


/**
 * Returns non-zero if the character is considered an end-of-line.
 * @param c a character
 * @return non-zero if eol, zero if not
 */
uint8_t eol(int c);

/**
 * Returns non-zero if the character is considered an end-of-token.
 * @param c a character
 * @return non-zero if eot, zero if not
 */
uint8_t eot(int c);

/**
 * Returns non-zero if the character is considered an end-of-expression.
 * @param c a character
 * @return non-zero if eoe, zero if not
 */
uint8_t eoe(int c);


/**
 * Finds the matching end character in a string, e.g. matching parens.
 * @param s the string to search
 * @param o the starting, opening character (e.g. '(')
 * @param c the end, closing character (e.g. ')')
 * @return offset of the end character in the string
 */
uint32_t findend(const char *s, char o, char c);

/**
 * Increments offset until the character in the string is not blank or fails
 * the given comparison.
 * @param s the string to use
 * @param cmp a comparing function, stops search if returns true
 * @param offset the variable to increment while searching
 */
void skipblank(const char *s, uint8_t (*cmp)(int), uint32_t *offset);

#endif // SHELPERS_H_
