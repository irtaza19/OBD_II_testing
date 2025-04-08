/**
* @file        commands.c
* @author      Aqib D. Ace 
* @date        March 2025
* @version     0.0.0
*/


#include "xc.h"
#include "mcc_generated_files/system.h"
#include "mcc_generated_files/uart1.h"
#include <stdbool.h>
#include "commands.h"

extern char ELM327_VERSION[32];
char tempCommand[32];
unsigned long UART1_Timeout;
uint8_t UART1FlowCtrl = 1;
unsigned long baudrate = 9600;
unsigned long oldbaudrate = 9600;
char TXbuffer[20];
uint32_t BRError = 0;
char receivedChar;
char hardIDStr[16] = "STN2120 r1.0\0";
char stiStr[16]    = "STN2120 v5.6.5\0";
char stixStr[32]   = "STN2120 v5.6.5 [2020.10.14]\0";
char serialStr[16] = "212010154303\0";
//bool argErrFlag = false;
bool statusLEDs = true;

void processCommand(char *command)
{
    int j = 0;
    
        // Remove spaces from command
        for (int i = 0; command[i] != '\0' && j < sizeof(tempCommand) - 1; i++) {
        if (command[i] != ' ') {
            tempCommand[j++] = command[i];
        }
        }
        tempCommand[j] = '\0'; // Null-terminate the modified command
        
        //capitalize the Alphabets
        for (int i = 0; tempCommand[i] != '\0'; i++)
        {
            if((tempCommand[i] >= 'a') && (tempCommand[i] <= 'z'))
            {
                tempCommand[i] = (tempCommand[i] - 'a') + 'A';
            }
        }
        
        if (tempCommand[0] == 'S' && tempCommand[1] == 'T')
        {
            processSTCommand(tempCommand);
        }
        else if (tempCommand[0] == 'A' && tempCommand[1] == 'T')
        {
            processATCommand(tempCommand);
        }
        else if (tempCommand[0] == '0')
        {
            processOBDRequest(tempCommand);
        }
        else
        {
            UART1_Write('?');   //wrong command error
        }
        UART1_Write('\r'); //return carriage return to host
        //check Line Feed controlled by ATL
        if (getLFStatus())
        {
            UART1_Write('\n');  //if line feed is on send line feed
        }
        UART1_Write('>'); //send prompt for next command
}

//function for checking numerical arguments
bool argErrCheck(char* command)  
{
    for (int i = 0; command[i] != '\0'; i++)
            {
                if((command[i] < '0') || (command[i] > '9'))
                {
                    return 1;
                    break;
                }
            }
    return 0;
}

//function for checking ASCII arguments
bool argAsciiErrCheck(char* command)
{
    for (int i = 0; command[i] != '\0'; i++)
            {
                if((command[i] <0x020) || (command[i] > 0x07E))
                {
                    return 1;
                    break;
                }
            }
    return 0;
}

//function for processing ST commands
void processSTCommand(char *command) {
    if  (strncmp(command, "STBRT", 5) == 0)
    {
        
        char *Uart1TimeoutStr = command + 5;  
        if (argErrCheck(Uart1TimeoutStr))
        {
            UART1_Write('?');
        }
        else
        {
        unsigned long Uart1Timeout = strtoul(Uart1TimeoutStr, NULL, 10);
        if (Uart1Timeout > 0 && Uart1Timeout < 65536) 
        {
            UART1_Timeout = Uart1Timeout;
            UART1_Write('O');
            UART1_Write('K');
        }
        else
        {
            UART1_Write('?');
        }
        
        }
    }
    else if (strncmp(command, "STBR", 4) == 0 ) 
    {
        char *baudrateStr = command + 4; // Skip "STBR "
        if (argErrCheck(baudrateStr))
        {
            UART1_Write('?');
        }
        else
        {
            baudrate = strtoul(baudrateStr, NULL, 10);
            if (baudrate > 38 && baudrate <= 10000000) {
                BRError = UART1_GetBRdiff(baudrate);
                if (BRError <= 3)
                {
                    UART1_WriteString("OK\r");
                    delay_ms(200);
                    UART1_Initialize(baudrate);
                    delay_ms(200);
                    UART1_WriteString("STN2120 v0.0.0");
                    receivedChar = UART1_Read();
                    if (receivedChar == '\r')
                    {
                        UART1_WriteString("\rOK");
                        oldbaudrate = baudrate;
                    }
                    else
                    {
                        UART1_Initialize(oldbaudrate);
                    }
                }
                else
                {
                   UART1_Write('?');
                }
              }
            else
            {
                UART1_Write('?');
            }
        }
    }
    else if (strncmp(command, "STSBR", 5) == 0) {
        char *Uart1TFBaudStr = command + 5; // Skip "STBR "
        if (argErrCheck(Uart1TFBaudStr))
        {
            UART1_Write('?');
        }
        else
        {
            baudrate = strtoul(Uart1TFBaudStr, NULL, 10);
            if (baudrate >= 38 && baudrate <= 10000000) 
            {
                BRError = UART1_GetBRdiff(baudrate);
                if (BRError <= 3)
                {
                    UART1_Write('O');
                    UART1_Write('K');
                    UART1_Write('\r');
                    if (getLFStatus())
                    {
                        UART1_Write('\n');
                    }
                    delay_ms(200);
                    UART1_Initialize(baudrate);
                    delay_ms(200);
                }
                else
                {
                    UART1_Write('?');
                }       
            }
            else
            {
                UART1_Write('?');
            }
        }
    }
    else if (strncmp(command, "STUFC", 5) == 0) {
        char *stufcStr = command + 5; // Skip "STBR "
        if (argErrCheck(stufcStr))
        {
            UART1_Write('?');
        }
        else
        {
          uint32_t stufc = strtoul(stufcStr, NULL, 10);
          if (stufc == 0 || stufc == 1)
          {
            if (stufc == 0) 
            {
                UART1FlowCtrl = 0;
                UART1_Write('O');
                UART1_Write('K');
            }
            else if (stufc == 1)
            {
                UART1FlowCtrl = 1;
                UART1_Write('O');
                UART1_Write('K');
            }
          }
          else
          {
              UART1_Write('?');
          }
        }
    }
    else if (strcmp(command, "STWBR") == 0)
    {   
        sprintf(TXbuffer, "%lu", baudrate);
        writeToNVM("STWBR",TXbuffer);
        UART1_Write('O');
        UART1_Write('K');      
    }
    else if (strcmp(command, "STDI") == 0)
    {
        //Print device hardware ID string (e.g., ?OBDLink r1.7?)
        UART1_WriteString(hardIDStr);
    }
    else if (strcmp(command, "STDICPO") == 0)
    {
        //Print POR (Power on Reset) count
    }
    else if (strcmp(command, "STI") == 0)
    {
        UART1_WriteString(stiStr);
    }
    else if (strncmp(command, "STSATI", 6) == 0) {
        char *atiStr = command + 6; 
        if (argAsciiErrCheck(atiStr))
        {
            UART1_Write('?');
        }
        else
        {
          //UART1_WriteString(atiStr);
            writeToNVM("STSATI",atiStr);
        }
    }
    else if (strncmp(command, "STSDI", 5) == 0) 
    {
        char *diStr = command + 5; // Skip "STBR "
        if (argAsciiErrCheck(diStr))
        {
            UART1_Write('?');
        }
        else
        {
            writeToNVM("STSDI",diStr);
        }
        
    }
    else if (strcmp(command, "STSN") == 0)  
    {
        //Print device serial number
        UART1_WriteString(serialStr);
        
    }
    
    else if (strncmp(command, "STS@1", 5) == 0) 
    {
        //use to store the device description string returned by AT@1
        char *atidStr = command + 5; 
        if (argAsciiErrCheck(atidStr))
        {
            UART1_Write('?');
        }
        else
        {
          writeToNVM("STS@1",atidStr);
        }     
    }
    else if (strncmp(command, "STVCAL",6) == 0) 
    {
        /*Calibrate voltage measurement. The voltage 
        returned by ATRV and STVR commands can be 
        calibrated using this command. Takes current voltage 
        with a maximum value of 65.534*/ 
        char *voltCalStr = command + 6; // Skip "STBR "
        if (argAsciiErrCheck(voltCalStr))
        {
            UART1_Write('?');
        }
        else
        {
          calibrateVoltage(voltCalStr);
        }   
    }
    else if (strncmp(command, "STVR", 4) == 0) 
    {
         /*Read voltage in volts. Returns calibrated voltage 
        measured by the ANALOG_IN pin.*/
        char* ADCvolts;  // 
        char* precession = command + 4; //0-3
        if (argErrCheck(precession))
        {
            UART1_Write('?');
        }
        else
        {
           readADCvolts(ADCvolts,precession);
           
        } 
        UART1_WriteString(ADCvolts);
       
    }
    else if (strcmp(command, "STVRX") == 0) 
    {
        /*Read voltage in ADC steps. Returns the voltage on
         ANALOG_IN pin in ADC counts. The range is 0x000 
         (AVSS) to 0xFFF (AVDD)*/  
         readADCSteps();
    }
    else if (strcmp(command, "STCALSTAT") == 0) 
    {
        //Read voltage calibration status  
        uint8_t stcalstatus = readSavedCalibrationStatus();
        if (stcalstatus == 1)
        {
            UART1_WriteString("ANALOG IN: SAVED");
        }
        else if (stcalstatus == 2)
        {
            UART1_WriteString("ANALOG IN: NOT READY");
        }
        else if (stcalstatus == 3)
        {
            UART1_WriteString("ANALOG IN: READY");
        }
        else
        {
            UART1_Write('?');
        }
    }
    else if (strcmp(command, "STRSTNVM") == 0) 
    {
        //Reset NVM to factory defaults 
        resetAllNVMToDefault();
    }
    else if (strcmp(command, "STSAVCAL") == 0) 
    {
        //Save all calibration values
        saveCallibratedValues();
    }
    else if (strncmp(command, "STUIL",5) == 0) 
    {
        //Disable/Enable LEDs
        //Default Enable
        char *stuilStr = command + 5; 
        if (argErrCheck(stuilStr))
        {
            UART1_Write('?');
        }
        else
        {
          uint32_t stuil = strtoul(stuilStr, NULL, 10);
          if (stuil == 0 || stuil == 1)
          {
            if (stuil == 0) 
            {
                statusLEDs = false;
                UART1_Write('O');
                UART1_Write('K');
            }
            else if (stuil == 1)
            {
                statusLEDs = true;
                UART1_Write('O');
                UART1_Write('K');
            }
          }
          else
          {
              UART1_Write('?');
          }
        }
    }
    else if (strncmp(command, "STGPC",5) == 0) 
    {
        //Configure I/O pins
    }
    else if (strncmp(command, "STGPIR",6) == 0) 
    {
        //Read inputs
    }
    else if (strncmp(command, "STGPIRH",7) == 0) 
    {
        //Read inputs, report value as hex
    }
    else if (strncmp(command, "STGPOR",6) == 0) 
    {
        //Read Output latches
    }
    else if (strncmp(command, "STGPOW",6) == 0) 
    {
        //Write Output latches
    }
    else 
    {
            UART1_Write('?');
    }
    
}

//function for processing AT commands
void processATCommand(char *command)
{
    if  (strcmp(command, "ATD") == 0)
    {
        /*Set all settings to defaults
           * Tester address
           * Last saved protocol
           * Protocol baud rate
           * Message headers
           * Message filter
           * Timeouts*/
        setAllSettoDefault();
        UART1_WriteString("\rOK");
    }
    else if  (strncmp(command, "ATE", 3) == 0)
    {
        //Turn echo on/off 1 means on 0 means off
        //default ON
        
        char *echoStr = command + 3; // Skip "STBR "
        if (argErrCheck(echoStr))
        {
            UART1_Write('?');
        }
        else
        {
          uint32_t echoStatus = strtoul(echoStr, NULL, 10);
          if (echoStatus == 0 ||  echoStatus == 1)
          {
            if (echoStatus == 0) 
            {
                disableEcho();
            }
            else if (echoStatus == 1)
            {
                enableEcho();
            }
            UART1_WriteString("\rOK");
          }
          else
          {
              UART1_Write('?');
          }
        }
    }
    else if  (strcmp(command, "ATI") == 0)
    {
        //print ELM327 version ID string
        UART1_WriteString(ELM327_VERSION);
     
    }
    else if  (strncmp(command, "ATL", 3) == 0)
    {
        //Line feeds on/off 
        //Default OFF
        char *lineFeedStr = command + 3; // Skip "STBR "
        if (argErrCheck(lineFeedStr))
        {
            UART1_Write('?');
        }
        else
        {
          uint32_t lineFeedStatus = strtoul(lineFeedStr, NULL, 10);
          if (lineFeedStatus == 0 ||  lineFeedStatus == 1)
          {
            if (lineFeedStatus == 0) 
            {
                disableLF();
            }
            else if (lineFeedStatus == 1)
            {
                enableLF();
            }
            UART1_WriteString("\rOK");
          }
          else
          {
              UART1_Write('?');
          }
        }
    }
    else if  (strcmp(command, "ATWS") == 0)
    {
        /*Warm start. This command reboots OBDLink, but 
        unlike ATZ, skips the LED test and keeps the user 
        selected baud rate (selected using ATBRD, STBR, or 
        STSBR).*/
        loadDefaultOnWarmReset();
        UART1_WriteString("\rOK");
    }
    else if  (strcmp(command, "ATZ") == 0)
    {
        //Reset device
        asm("reset");
    }
    else if  (strcmp(command, "AT@1") == 0)
    {
        //Display device description saved by STS@1
        printSaved("STS@1");
    }
    else if  (strcmp(command, "AT@2") == 0)
    {
        //Display device identifier
        printSaved("AT@3");
    }
    else if  (strncmp(command, "AT@3", 4) == 0)
    {
        //store device identifier  
        char *DeviceIDStr = command + 4; // Skip "STBR "
        if (argAsciiErrCheck(DeviceIDStr))
        {
           UART1_Write('?');
        }
        else
        {
          writeToNVM("AT@3",DeviceIDStr);
          UART1_WriteString("\rOK");
        } 
    }
    else if  (strncmp(command, "ATPPS", 5) == 0)
    {
        //Print programmable parameter summary.
        printPParameters();
    }
    else if (strncmp(command, "ATPPR", 5) == 0)
    {
        restoreDefaultPParameters();
        UART1_WriteString("\rOK");
    }
    else if (strncmp(command, "ATPPT", 5) == 0)
    {
       // WordWriteExample();
    }
    else if  (strncmp(command, "ATPP", 4) == 0)
    {
       //Turn off/ON programmable parameter xx.
       //Set the value of programmable parameter xx to yy.
       char *atppStr = command + 4; // Skip "STBR "
       if (argAsciiErrCheck(atppStr))
        {
           UART1_Write('?');
        }
        else
        {
          configurePPs(atppStr);
          UART1_WriteString("\rOK");
        }
    }
    
    else if (strncmp(command, "ATPOC", 5) == 0)
    {
       printAllSaved();
    }
    else 
    {
        UART1_Write('?');
    }
    
    
}
//function for processing OBD requests
void processOBDRequest(char *command)
{
    
}

//function for setting values to default on STWS call
void loadDefaultOnWarmReset(void)
{
    enableEcho();
    disableLF();
    //set PP R-Type to default
    //continue
}

//function for setting NVM to default
void resetAllNVMToDefault(void)
{
    //restore PPs
    restoreDefaultPParameters();
    //OBD protocol
    //UART baud rate
    //voltage calibration
    //ATI device ID
    //AT@1
    //POWER Save configuration parameters
    //continue
    //not included AT@3,STSDI,STSAVCAL,ATSD
}