/**
 * perilous_pointers
 * CS 341 - Fall 2023
 */
#include "part2-functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * (Edit this function to print out the "Illinois" lines in
 * part2-functions.c in order.)
 */
int main() {
    // your code here
     int value = 81;
    first_step(value);

    value = 132;
    int* value2 = &value;
    second_step(value2);

    value = 8942;
    int* value3[] = {value2};
    double_step(value3);

    char* char_value = malloc(sizeof(char));
    char_value[5] = '\n' + 5;\
    strange_step(char_value);
    
    void* temp = malloc(sizeof(char));
    char c = ' ';
    *(char*)temp = c;
    empty_step(temp);
    free(temp);

    *char_value = c;
    char_value[3] = 'u';
    temp = char_value;
    two_step(temp, char_value);

    char * char_value2 = char_value + 2;
    char * char_value3 = char_value2 + 2;
    three_step(char_value, char_value2, char_value3);

    char first_arr[2];
    char second_arr[3];
    char third_arr[4];
    first_arr[1] = 'a';
    second_arr[2] = first_arr[1] + 8;
    third_arr[3] = second_arr[2] + 8;
    step_step_step(first_arr, second_arr, third_arr);

    int b = 5;
    char a_char = 0;
    char* a = &a_char;
    *a = b;
    it_may_be_odd(a, b);

    char str[12] = "CS241,CS241";
    tok_step((char*) str);

    void* orange = malloc(sizeof(int*));
    void* blue = orange;
    ((char *)blue)[0] = 1;
    *(int*)orange = 513;
    the_end(orange, blue);

    free(orange);
    free(char_value); 
    return 0;
}
