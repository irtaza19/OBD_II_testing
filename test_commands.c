#include "test_commands.h"
#include "mcc_generated_files/system.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

// Explicit external declarations
extern void processCommand(char *command);
extern void UART1_ReadString(char *buffer, int maxLen);
extern void UART1_WriteString(const char *str);
extern void delay_ms(int ms);
extern unsigned long getSystemTime_ms(); // Function to get system time for execution measurement

// Constants
#define TOTAL_COMMANDS 30  // Total number of testable commands
#define BUFFER_SIZE 200    // Buffer size for UART responses and NVM state
#define MAX_RETRIES 3      // Number of retries for failed commands

// Full list of commands from commands.c
const char *validCommands[TOTAL_COMMANDS] = {
    "ATPPR", "ATPPS", "ATPOC", "STBRT 5000", "STUFC 1", "STWBR",
    "STI", "STSBR 9600", "STBR 115200", "STUFX 0", "STDI", "STDICPO",
    "STSATI TESTDATA", "STSDI TESTSTRING", "STSN", "STS@1 DEVICEINFO",
    "STVCAL 12.45", "STVR 2", "STVRX", "STCALSTAT", "STRSTNVM",
    "STSAVCAL", "STUIL 1", "STGPC", "STGPIR", "STGPIRH", "STGPOR",
    "STGPOW", "ATD", "ATZ"
};

// Global buffers for storing NVM states and responses
char initialNVMState[BUFFER_SIZE];
char currentNVMState[BUFFER_SIZE];
char commandResponse[BUFFER_SIZE];

// Track test results
int commandsTested = 0;
int commandsPassed = 0;
int commandsFailed = 0;

// Capture actual NVM state via ATPPS
void getNVMState(char *nvmState) {
    UART1_WriteString("\r\nExecuting ATPPS to get NVM state...\r\n");
    processCommand("ATPPS");
    delay_ms(500);
    UART1_ReadString(nvmState, BUFFER_SIZE);
}

// Compare two NVM states and return difference status
bool compareNVMStates(char *initial, char *current) {
    return (strcmp(initial, current) == 0);
}

// Execute a command and capture the response with retry logic
bool executeCommand(char *command, char *responseBuffer) {
    int attempt = 0;
    bool success = false;

    while (attempt < MAX_RETRIES && !success) {
        UART1_WriteString("\r\nExecuting: ");
        UART1_WriteString(command);
        UART1_WriteString("\r\n");

        unsigned long startTime = getSystemTime_ms();
        processCommand(command);
        delay_ms(500);
        UART1_ReadString(responseBuffer, BUFFER_SIZE);
        unsigned long endTime = getSystemTime_ms();

        // Log execution time
        UART1_WriteString("Execution Time: ");
        char timeBuffer[20];
        sprintf(timeBuffer, "%lu ms\r\n", (endTime - startTime));
        UART1_WriteString(timeBuffer);

        // Check for failure indicator ('?')
        if (strchr(responseBuffer, '?') != NULL) {
            UART1_WriteString("?? Command failed! Retrying...\r\n");
            attempt++;
        } else {
            success = true;
        }
    }

    return success;
}

// Run functionality test for all commands
void functionalityTest() {
    UART1_WriteString("\r\n--- Starting Full Functionality Test ---\r\n");

    // Step 1: Reset and store initial NVM state
    executeCommand("ATPPR", commandResponse);
    delay_ms(500);
    executeCommand("ATPOC", commandResponse);
    delay_ms(500);
    getNVMState(initialNVMState);  // Capture initial NVM state

    // Step 2: Execute and verify each command
    for (int i = 0; i < TOTAL_COMMANDS; i++) {
        char command[30];
        strcpy(command, validCommands[i]);

        UART1_WriteString("\r\n---------------------------------\r\n");
        
        // Execute command with retries
        bool commandSuccess = executeCommand(command, commandResponse);
        
        // Print the command response
        UART1_WriteString("Response: ");
        UART1_WriteString(commandResponse);
        UART1_WriteString("\r\n");

        // Step 3: Capture and compare NVM state after command execution
        getNVMState(currentNVMState);
        
        UART1_WriteString("\r\nComparing NVM state...\r\n");
        if (compareNVMStates(initialNVMState, currentNVMState)) {
            UART1_WriteString("? NVM state unchanged.\r\n");
        } else {
            UART1_WriteString("?? NVM state CHANGED!\r\n");
            UART1_WriteString("New NVM State: ");
            UART1_WriteString(currentNVMState);
            UART1_WriteString("\r\n");
        }

        // Track results
        commandsTested++;
        if (commandSuccess) {
            commandsPassed++;
        } else {
            commandsFailed++;
        }

        delay_ms(500);
    }

    // Step 4: Print test summary
    UART1_WriteString("\r\n--- Functionality Test Summary ---\r\n");
    char summaryBuffer[100];
    sprintf(summaryBuffer, "Total Commands Tested: %d\r\nCommands Passed: %d\r\nCommands Failed: %d\r\n",
            commandsTested, commandsPassed, commandsFailed);
    UART1_WriteString(summaryBuffer);
    UART1_WriteString("\r\n--- Test Completed ---\r\n");
}
