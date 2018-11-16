#ifndef SPELLCHECKER_H
#define SPELLCHECKER_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define DICTIONARY_SIZE 200000

//Header file to handle the dictionary and spell checking functionality of the server.

/* Builds and returns pointer to a dictionary array containing all the words
 found in the passed dictionary file. */
char **buildDictionary(char *file){

    //Open the dictionary file
    FILE *inPtr;
    if((inPtr = fopen(file, "r")) == NULL){
      puts("File could not be opened");
      return NULL;
    }
    else{

        //Create dictionary data structure
        char *dictionary[DICTIONARY_SIZE];
        char **dictPtr = dictionary;
        size_t i = 0;
        while(!feof(inPtr)){ //read words from the dictionary file into the array
            char buffer[100];
            fgets(buffer, 100, inPtr);
            dictionary[i] = malloc(sizeof(char)*strlen(buffer));
            strncpy(dictionary[i], buffer, strcspn(buffer, "\r\n"));
            i++;
        }
        printf("Dictionary size : %lu\n", i-1);
        fclose(inPtr);
        return dictPtr;
    }

}

//Prints out the generated dictionary array.
void printDictionary(char **dictPtr){
    for(size_t i = 0; *dictPtr != NULL ; i++){
        printf("%lu: %s\n", i, *dictPtr);
        dictPtr++;
    }
}

//Checks if the working dictionary contains the passed word.
char *checkWord(char *word, char **dictPtr){
    char *results = malloc(sizeof(char)*50);

    //Linear search of the dictionary file for a matching word: O(n)
    for(size_t i = 0; *dictPtr != NULL ; i++){
        if(strcmp(word, *dictPtr) == 0){ //Dictionary contains word / word is spelled correctly
            strcpy(results, "OK!: ");
            strcat(results, word);
            strcat(results, "\n");
            return results;
        }
        dictPtr++;
    }

    //Dictionary did not contain word / word is misspelled
    strcpy(results, "MISSPELLED: ");
    strcat(results, word);
    strcat(results, "\n");
    return results;
}
#endif