//
// Created by Jake Camadine on 1/28/2024.
//
#include <stdlib.h>
#include "string.h"
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <ctype.h>

struct quote {
    char *body;
    char *author;
    struct quote *next;
};

struct quote *head = NULL;
struct  quote *tail = NULL;

const char *flag = "quit";
char *puzzle;
char *encryptedString;
char encryptionKey[26]; //array positions map A-Z to 0-25
char playerKey[26];
char userInput[5];

int listSize = 0;

/**
 * Flush stdin when there is input left over.
 */
void flushBuffer(){
    if(strlen(userInput) == 4 && userInput[3] != '\n')
        while(getchar() != '\n');
}

/**
 * Swaps two characters at the specified position in the
 * specified array.
 *
 * @param array - the string to modify
 * @param first - the position of the first character
 * @param second - the position of the second character
 */
void swapChars(char *array, int first, int second) {
    char temp = array[first];
    array[first] = array[second];
    array[second] = temp;
}

/**
 * Implements the Fisher-Yates Shuffle on a character string.
 *
 * @param orderedString - the string to shuffle
 */
void shuffle(char *orderedString) {
    for(int i = (int)(strlen(orderedString)-1); i > 0; i--) {
        swapChars(orderedString, rand() % i, i);
    }
}

/**
 * @return a pointer to a newly allocated quote.
 */
struct quote *quoteAlloc() {
    struct quote *newQuote = (struct quote*) malloc(sizeof(struct quote));
    newQuote->body = NULL;
    newQuote->author = NULL;
    newQuote->next = NULL;
    return newQuote;
}

/**
 * Link a new quote to the last if head is not NULL. Create
 * head otherwise.
 *
 * @param newQuote - the quote to be added
 */
void append(struct quote *newQuote) {
    if(head == NULL) {
        head = newQuote;
        tail = newQuote;
        listSize++;
        return;
    }
    tail->next = newQuote;
    tail = tail->next;
    listSize++;
}

/**
 * Walks the list to the nth element and returns it.
 *
 * @param n - the index of the entry.
 * @return the entry at the specified index.
 */
struct quote *get(int n) {
    struct quote *current = head;
    for(int i = 0; i < n; i++) {
        current = current->next;
    }
    return current;
}

/**
 * Builds the body string of a quote.
 *
 * @param current - the current quote being built by loadQuotes
 * @param buffer - the buffer for the text file
 */
void buildBody(struct quote *current, char *buffer) {
    size_t lineLength = 0;
    size_t currentBodyLength = 0;
    char *tempString;

    if(current->body == NULL) {
        current->body = malloc((lineLength = strlen(buffer)) * sizeof(char) + 1);
        strncpy(current->body, buffer, lineLength + 1);
    }
    else {
        currentBodyLength = strlen(current->body);
        lineLength = strlen(buffer);
        tempString = malloc((sizeof(char) * (lineLength + currentBodyLength)) + 1);
        strncpy(tempString, current->body, currentBodyLength + 1);
        strncat(tempString, buffer, lineLength);
        free(current->body);
        current->body = tempString;
    }
}

/**
 * Builds a linked list of quotes from a text file.
 */
void loadQuotes() {
    FILE *quotes;
    size_t lineLength = 0;
    struct quote *current = quoteAlloc();
    char buffer[93];

    quotes = fopen("quotes.txt", "r");
    if(quotes == NULL) {
        printf("error opening file.");
        exit(1);
    }
    while(fgets(buffer, sizeof(buffer), quotes) != NULL) {
        if(strlen(buffer) < 3 && current->body != NULL && current->author != NULL) {
            append(current);
            current = quoteAlloc();
        }
        else if(buffer[0] == '-' && buffer[1] == '-') {
            current->author = malloc((lineLength = strlen(buffer)) * sizeof(char) + 1);
            strncpy(current->author, buffer, lineLength + 1);
        }
        else {
            buildBody(current, buffer);
        }
    }
    if(current->body != NULL && current->author != NULL) {
        append(current);
    }
    fclose(quotes);
}

/**
 * Walks the linked list of quotes freeing
 * allocated memory.
 */
void freeQuotes() {
    struct quote *current = head;
    struct quote *previous = NULL;

    while(current != NULL) {
        previous = current;
        current = current->next;
        free(previous->body);
        free(previous->author);
        free(previous);
    }
}

/**
 * Creates the list of quotes if it does not exist already.
 *
 * @return a random puzzle from the list of quotes.
 */
char *getPuzzle() {
    if(listSize == 0) {
        loadQuotes();
    }
    return get(rand() % listSize)->body;
}

/**
 * Walks through the puzzle and replaces each character with a corresponding
 * mapping in the encryption key unless non-alphabetic.
 */
 void encryptPuzzle() {
    for(int i = 0; i < strlen(puzzle); i++) {
        if (isalpha(puzzle[i])) {
            encryptedString[i] = encryptionKey[toupper(puzzle[i]) - 65];
        } else {
            encryptedString[i] = puzzle[i];
        }
    }
    encryptedString[strlen(puzzle)] = '\0';
}

/**
 * Initializes the necessary variables before the game loop begins.
 */
void initialization() {
    puzzle = getPuzzle();
    for(int i = 0; i < 26; i++) {
        encryptionKey[i] = (char) (65 + i);
    }
    shuffle(encryptionKey);
    for(int i = 0; i < 26; i++) {
        playerKey[i] = '\0';
    }
    encryptedString = (char *) malloc(sizeof(char) * strlen(puzzle) + 1);
    encryptPuzzle();
}

/**
 * Tests for a change in the game's state based on user input.
 *
 * @param input - user input
 * @return true if the input is 'quit' or NULL
 */
bool updateState(char *input) {
    if(strcmp(input, flag) == 0 || strcmp(input, "\n") == 0) {
        return true;
    }
    return false;
}

/**
 * Accepts a game move from the user.
 *
 * @return the user input buffer.
 */
char *acceptInput() {
    printf("\nEnter a letter and its replacement, or quit to end program: ");
    fgets(userInput, sizeof(userInput), stdin);
    flushBuffer();
    if(updateState(userInput) == false) {
        if(strlen(userInput) == 3 && isalpha(userInput[0]) && isalpha(userInput[1])) {
            playerKey[toupper(userInput[0])-65] = toupper(userInput[1]);
        }
        else {
            printf("\nInvalid input. Please try again.\n");
        }
    }
    return userInput;
}

/**
 * Prints the game world to the screen, and tracks the decryption progress.
 */
bool displayWorld() {
    bool decryptionStatus = true;
    printf("Encrypted: %s\n", encryptedString);
    printf("Decrypted: ");
    for(int i = 0; i < strlen(encryptedString); i++) {
        if(isalpha(encryptedString[i])) {
            if(playerKey[toupper(encryptedString[i])-65] == '\0') {
                decryptionStatus = false;
                printf("_");
            }
            else {
                if(encryptedString[i] != encryptionKey[playerKey[encryptedString[i] - 65] - 65]) {
                    decryptionStatus = false;
                }
                printf("%c", playerKey[toupper(encryptedString[i])-65]);
            }
        }
        else {
            printf("%c", encryptedString[i]);
        }
    }
    printf("\n");
    return decryptionStatus;
}

/**
 * Controls the flow of the game until the user quits.
 */
void gameLoop() {
    bool stateChange = false;
    while(stateChange == false) {
        displayWorld();
        stateChange = updateState(acceptInput());
    }
}

/**
 * Prepares the game for termination.
 */
void tearDown() {
    free(encryptedString);
    printf("all done.\n");
}

int main(int argc, char** argv) {
    srand(time(NULL));
    char buffer[3];
    do {
        initialization();
        gameLoop();
        tearDown();
        printf("\nNew Game?(y/n): ");
        fgets(buffer, sizeof(buffer), stdin);
    }while(buffer[0] == 'y');
    freeQuotes();
    return 0;
}