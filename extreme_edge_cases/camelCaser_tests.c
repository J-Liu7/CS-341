/**
 * extreme_edge_cases
 * CS 341 - Fall 2023
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "camelCaser.h"
#include "camelCaser_tests.h"

int test_camelCaser(char **(*camelCaser)(const char *),
                    void (*destroy)(char **)) {
    // TODO: Implement me!
    
    // Test 0: testing if the input is null
    char** temp0 = camelCaser(NULL);
    if (temp0 != NULL) {
        destroy(temp0);
        return 0;
    }


    //Test 1: testing a single punctuation
    char** temp1 = camelCaser(".");
    if (temp1[0][0] != '\0' || temp1[1] != NULL) {
        destroy(temp1);
        return 0;
    }
    destroy(temp1);


    // Test 2: testing when the string is empty
    char** temp2 = camelCaser("");
    if (temp2[0] != NULL) {
        destroy(temp2);
        return 0;
    }
    destroy(temp2);


    // Test 3: testing numbers special characters
    char** temp3 = camelCaser("7\t27wysi(5)");
    if (temp3[2] != NULL) {
        destroy(temp3);
        return 0;
    }
    char* expected3[3];
    expected3[0] = "727Wysi";
    expected3[1] = "5";
    expected3[2] = NULL;
    int i = 0;
    while(expected3[i]) {
        if (strcmp(expected3[i], temp3[i]) != 0) {
            destroy(temp3);
            return 0;
        }
        i++;
    }
    destroy(temp3);
    

    // Test 4: testing continuous punctuation, punctuation separated by a space and a non-sentence
    char** temp4 = camelCaser(".! .,osAIUHioasu");  
    if (temp4[4] != NULL) {
        destroy(temp4);
        return 0;
    }
    char* expected4[5];
    expected4[0] = "";
    expected4[1] = "";
    expected4[2] = "";
    expected4[3] = "";
    expected4[4] = NULL;
    i = 0;
    while(expected4[i]) {
        if(strcmp(expected4[i], temp4[i]) != 0) {
            destroy(temp4);
            return 0;
        }
        i++;
    }
    destroy(temp4);
    

    // Test 5: a short normal input with 1 sentence
    char** temp5 = camelCaser("bruh. uoshakdgfkuosdygf");
    if (temp5[1] != NULL) {
        destroy(temp5);
        return 0;
    }
    char* expected5[2];
    expected5[0] = "bruh";
    expected5[1] = NULL;
    i = 0;
    while (expected5[i]) {
        if (strcmp(expected5[i], temp5[i]) != 0) {
            destroy(temp5);
            return 0;
        }
        i++;
    }
    destroy(temp5);

    // Test 6: Test continuous punctuation and a single upper case
    char** temp6 = camelCaser(".[7when youseeit]A(.!");
    if (temp6[6] != NULL) {
        destroy(temp6);
        return 0;
    }
    char* expected6[7];
    expected6[0] = "";
    expected6[1] = "";
    expected6[2] = "7whenYouseeit";
    expected6[3] = "a";
    expected6[4] = "";
    expected6[5] = "";
    expected6[6] = NULL;
    i = 0;
    while(expected6[i]) {
        if (strcmp(expected6[i], temp6[i]) != 0) {
            destroy(temp6);
            return 0;
        }
        i++;
    }
    destroy(temp6);


    // Test 7: a long normal input
    char** temp7 = camelCaser("I cant take it. I see an image of a random score posted and then I see it.");
    if (temp7[2] != NULL) {
        destroy(temp7);
        return 0;
    }
    char* expected7[3];
    expected7[0] = "iCantTakeIt";
    expected7[1] = "iSeeAnImageOfARandomScorePostedAndThenISeeIt";
    expected7[2] = NULL;
    i = 0;
    while(expected7[i]) {
        if (strcmp(expected7[i], temp7[i]) != 0) {
            destroy(temp7);
            return 0;
        }
        i++;
    }
    destroy(temp7);

    // Test 8: random long string
    char** temp8 = camelCaser("uKj1w tr45a!2o 4E5J8mc4. 24IporO uxQRTI KrMGH RT(VASJDLOKLN48asdlVa");
    if (temp8[3] != NULL) {
        destroy(temp8);
        return 0;
    }
    char* expected8[4];
    expected8[0] = "ukj1wTr45a";
    expected8[1] = "2o4E5j8mc4";
    expected8[2] = "24iporoUxqrtiKrmghRt";
    expected8[3] = NULL;
    i = 0;
    while(expected8[i]) {
        if (strcmp(expected8[i], temp8[i]) != 0) {
            destroy(temp8);
            return 0;
        }
        i++;
    }
    destroy(temp8);

    // Test 9: ???
    char** temp9 = camelCaser("7 27Wysi.");
    if ((strcmp(temp9[0], "727Wysi") != 0) && temp9[1] != NULL) {
        destroy(temp9);
        return 0;
    } 
    destroy(temp9);
    return 1;
}
