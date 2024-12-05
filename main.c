#include <stdio.h>
#include <stdlib.h>
//#include <conio.h>
//#include <windows.h>
#include "rs232.h"
#include "serial.h"

#define bdrate 115200               /* 115200 baud */

void SendCommands (char *buffer );

int main()
{
    /*
    //char mode[]= {'8','N','1',0};
    char buffer[100];

    // If we cannot open the port then give up immediately
    if ( CanRS232PortBeOpened() == -1 )
    {
        printf ("\nUnable to open the COM port (specified in serial.h) ");
        exit (0);
    }

    // Time to wake up the robot
    printf ("\nAbout to wake up the robot\n");

    // We do this by sending a new-line
    sprintf (buffer, "\n");
     // printf ("Buffer to send: %s", buffer); // For diagnostic purposes only, normally comment out
    PrintBuffer (&buffer[0]);
    Sleep(100);

    // This is a special case - we wait  until we see a dollar ($)
    WaitForDollar();

    printf ("\nThe robot is now ready to draw\n");
    */

    FILE *fPtr; //Assigns a pointer to the file 
    fPtr = fopen("SingleStrokeFont.txt", "r" );  //Opens the SingleStrokeFont file in read only

    //This checks whether the file was openend correctly or not, if it wasn't then the program will exit
    if (fPtr == NULL) 
    {
        printf("ERROR OPENING FILE: \"SingleStrokeFont!!\"");
        exit(0);
    }

    int i = 0;
    int SingleStrokeFont_NumberOfRows = 1027; //Number of rows (as said in the project brief document) used to work out the number of structural arrays

    //Declares a structure that will be used to store the data from the "SingleStrokeFont.txt" file
    struct line
    {
        int a0;     //holds the value of the x movement
        int a1;     //holds the value of the y movement
        int a2;     //holds the value of whether the pen is up or down
    };

    struct line all_lines[SingleStrokeFont_NumberOfRows]; //Defines a structural array which will contain each line of the SingleStrokeFont file 
    

    //The for loop iterates over the structural array and copies the values from the SingleStrokFont file to it
    for (i = 0; i < SingleStrokeFont_NumberOfRows; i++)
    {
        fscanf(fPtr,"%d %d %d", &all_lines[i].a0, &all_lines[i].a1, &all_lines[i].a2); //reads each value (seperated by a space) and copies it to the corresponding value in the 'lines'  struct
    }
    fclose(fPtr); //Close the SingleStrokeFont file
}










/*

        //These commands get the robot into 'ready to draw mode' and need to be sent before any writing commands
    sprintf (buffer, "G1 X0 Y0 F1000\n");
    SendCommands(buffer);
    sprintf (buffer, "M3\n");
    SendCommands(buffer);
    sprintf (buffer, "S0\n");
    SendCommands(buffer);


    // These are sample commands to draw out some information - these are the ones you will be generating.
    sprintf (buffer, "G0 X-13.41849 Y0.000\n");
    SendCommands(buffer);
    sprintf (buffer, "S1000\n");
    SendCommands(buffer);
    sprintf (buffer, "G1 X-13.41849 Y-4.28041\n");
    SendCommands(buffer);
    sprintf (buffer, "G1 X-13.41849 Y0.0000\n");
    SendCommands(buffer);
    sprintf (buffer, "G1 X-13.41089 Y4.28041\n");
    SendCommands(buffer);
    sprintf (buffer, "S0\n");
    SendCommands(buffer);
    sprintf (buffer, "G0 X-7.17524 Y0\n");
    SendCommands(buffer);
    sprintf (buffer, "S1000\n");
    SendCommands(buffer);
    sprintf (buffer, "G0 X0 Y0\n");
    SendCommands(buffer);

    // Before we exit the program we need to close the COM port
    CloseRS232Port();
    printf("Com port now closed\n");

    return (0);
}

// Send the data to the robot - note in 'PC' mode you need to hit space twice
// as the dummy 'WaitForReply' has a getch() within the function.
void SendCommands (char *buffer )
{
    // printf ("Buffer to send: %s", buffer); // For diagnostic purposes only, normally comment out
    PrintBuffer (&buffer[0]);
    WaitForReply();
    Sleep(100); // Can omit this when using the writing robot but has minimal effect
    // getch(); // Omit this once basic testing with emulator has taken place
}


*/