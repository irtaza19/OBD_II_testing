#ifndef TEST_AUTO_H
#define TEST_AUTO_H

#include "mcc_generated_files/system.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

// Function Declarations
void generateTestCases(char testCases[][20], int maxTests);
void automatedTest();
void delay_ms(uint16_t ms);  // ? Declare delay_ms() so test_auto.c can use it
void UART1_WriteString(const char *str);  // ? Declare missing function
void processSTCommand(char *command);  // ? Declare missing function

#endif /* TEST_AUTO_H */
