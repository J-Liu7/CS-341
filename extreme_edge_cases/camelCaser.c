/**
 * extreme_edge_cases
 * CS 341 - Fall 2023
 */
#include "camelCaser.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

char **camel_caser(const char *input_str) {
    if (!input_str)
        return NULL;

    int numSentences = 0;
    int i = 0;

    while(input_str[i]) {
        if (ispunct(input_str[i]))
            numSentences++;
        i++;
    }

    char** result = malloc((numSentences + 1) * sizeof(char*));
    result[numSentences] = NULL;

    i = 0;
    int j = 0;
    int numCharacters = 0;

    while(input_str[i]) {
        if (ispunct(input_str[i])) {
            result[j] = malloc((numCharacters + 1) * sizeof(char));
            result[j][numCharacters] = '\0';
            numCharacters = 0;
            j++;
        } else {
            if (!isspace(input_str[i]))
                numCharacters++;
        }
        i++;
    }
    i = 0;
    j = 0;
    int charCount = 0;
    int capitalize = 0;
    int firstChar = 1;
    char c;

    while(input_str[i]) {
        if (j == numSentences)
            break;
        if (ispunct(input_str[i])) {
            charCount = 0;
            j++;
            firstChar = 1;
            capitalize = 0;
        } else if (isspace(input_str[i])) {
            capitalize = 1;
        } else {
            c = input_str[i];
            if (isalpha(c)) {
                if (capitalize && !firstChar) {
                    c = toupper(input_str[i]);
                    capitalize = 0;
                } else {
                    c = tolower(input_str[i]);
                    capitalize = 0;
                }
            }
            if (!isalpha(c) && firstChar)
                capitalize = 0;
            firstChar = 0;
            result[j][charCount] = c;
            charCount++;
        }
        i++;
    }
    return result;
}

void destroy(char **result) {
    int k = 0;
    while (result[k]) {
        free(result[k]);
        result[k] = NULL;
        k++;
    }
    free(result);
    result = NULL;
    return;
}
