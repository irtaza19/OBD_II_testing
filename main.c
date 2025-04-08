/**
* @file        main.h
* @author      Aqib D. Ace 
* @date        17 March 2025 10:45 PM
* @version     0.0.0
*/

#include "mcc_generated_files/system.h"

char receivedCmd[50]; //buffer for commands

void delay_ms(uint16_t ms) { //temporary delay function
    for (uint16_t i = 0; i < ms; i++) {
        for (uint16_t j = 0; j < 180; j++) {
            asm("nop"); // No-operation instruction
        }
    }
}

int main(void)
{
    
    // initialize the device
    SYSTEM_Initialize();

    delay_ms(200);
    UART1_WriteString("ELM327 v1.4b\r"); ///ATI string received on connection
    processCommand("ATPPR");
    UART1_Write('>');
        
    
    automatedTest();
    
        
    while (1)
    {
        // wait for the command and process it
        UART1_ReadString(receivedCmd, 50);
        processCommand(receivedCmd);
    }
    return 1;
}
