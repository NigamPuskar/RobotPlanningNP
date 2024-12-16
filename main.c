#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <conio.h>
//#include <windows.h>
#include "rs232.h"
#include "serial.h"

#define bdrate 115200               /* 115200 baud */

void SendCommands (char *buffer );

//Initialises structure that is used to store each word and it's characters
struct Word     
{
    char *characters;    //pointer to the character array
};

//Declares a structure that will be used to store the data from the "SingleStrokeFont.txt" file
struct line
{
    float a0;     //holds the value of the x movement
    float a1;     //holds the value of the y movement
    int a2;     //holds the value of whether the pen is up or down
};

void ReadWord(FILE *fPtr1, struct Word *All_Words, int *RunningP, int word, int WordCount);  //Function prototype for the 'ReadWord' function

int NewLine(int *RunningP, float user_scale, const int X_Limit, int *XPtr, int line, int *characterPtr);   //Function prototype for the 'Newline' function 

float x_coordinate(int *XPtr, int user_scale, int *characterPtr, struct line *all_lines, int p, float X_local);     //Function prototype for the 'X_coordinate' function 

float y_coordinate(int *YPtr, int user_scale, int line, int line_spacing, struct line *all_lines, int p, float Y_local);  //Function prototype for the'Y_coordinate function'

int main()
{
    
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
   

    //This section asks the user an input, which will be used to scale the values in the SingleStrokeFont.txt file

    float user_scale;      //Initialising variable to hold user inputted scale
    const float default_scale = 18.0;      //scale used in the SingleStrokeFont file
    
    printf("Please input the scaling you would like for the font: ");
    scanf("%f", &user_scale);       //Assigns user input to user_scale

    float scale = user_scale / default_scale;   //Works out correct scaling


    //This section copies the data to a structure, where it is multiplied by the scaling values for the relevant x and y movements

    int i;      //counter variable for 'for loop'
    int SingleStrokeFont_NumberOfRows = 1027; //Number of rows (as said in the project brief document) used to work out the number of structural arrays
    
    FILE *fPtr; //Assigns a pointer to the file 
    fPtr = fopen("SingleStrokeFont.txt", "r" );  //Opens the SingleStrokeFont file in read only

    //This checks whether the file was openend correctly or not, if it wasn't then the program will exit
    if (fPtr == NULL) 
    {
        printf("ERROR OPENING FILE: \"SingleStrokeFont!!\"");
        exit(0);
    }

    struct line all_lines[SingleStrokeFont_NumberOfRows]; //Defines a structural array which will contain each line of the SingleStrokeFont file 
    
    //The for loop iterates over the structural array and copies the values from the SingleStrokFont file to it
    for (i = 0; i < SingleStrokeFont_NumberOfRows; i++)
    {
        int temp_a0, temp_a1, temp_a2;  //Declares temporary variables used to store float versions of the data from SingleStrokeFont
        fscanf(fPtr,"%d %d %d", &temp_a0, &temp_a1, &temp_a2); //reads each value (seperated by a space) and copies it to the corresponding value in the 'lines' struct
        
        if (temp_a0 != 999)     //If NOT a0 (therefore working with the lines inbetween 999 - the x and y coordinates), execute the code inside the if statement:
        {
            all_lines[i].a0 = (float)temp_a0 * scale;   //assign the line struct values to the scaled SingleStrokeFont data (x coordinates)
            all_lines[i].a1 = (float)temp_a1 * scale;   //assign the line struct values to the scaled SingleStrokeFont data (y coordinates)
        }

        else    //if a0 is 999 (indicating the start of a new character), execute the code inside the else statement:
        {
          all_lines[i].a0 = temp_a0;    //Simply assigns the value of the temp placeholder to the line struct value
          all_lines[i].a1 = temp_a1;    //Simply assigns the value of the temp placeholder to the line struct value
        }
        all_lines[i].a2 = temp_a2;      //Since a2 does not get affected by scaling, all elements are equated to the temp_a2 value
    }
    fclose(fPtr); //Closes the SingleStrokeFont file


    FILE *fPtr1;
    fPtr1 = fopen("SingleWordTest.txt", "r");

    if (fPtr1 == NULL) 
    {
        printf("ERROR OPENING FILE: \"SingleStrokeTest!\"");
        exit(1);
    }
    //Counts how many words there are in the file to know how many structures are needed
    int WordCount = 0;  //initialises word count
    char ch;   

    while((ch = fgetc(fPtr1)) != EOF)     //Goes through each character in the text file that 'fPtr1' is pointing at 
    {
        if (ch == ' ' || ch == '\n' || ch =='\t')   //If there's a space, new line or tab, execute the if statement
        {
            WordCount++;    //increments the word count by 1
        }
        /*else if ((ch = fgetc(fPtr1)) == EOF)   //else if the end of the file is reached
        {
            WordCount++;   //increments the word count by 1  
        }*/
    }

    rewind(fPtr1);  //Resets the file position to the beginning of the file 

    //printf("%d", WordCount);

    struct Word All_Words[WordCount];  //creates an array of structures, with the amount of structures equalling the wordcount
    //printf("%s", All_Words[0].characters);
  

    //Adding variables for new line function
    int Running_CharCount = 0;  //initalises a running character count variable
    int *RunningP = &Running_CharCount;     //pointer to the memory address of the running char count variable
    const int X_Limit = 100;   //

    int character_position = 0;
    int *characterPtr = &character_position;
    
    
    int X_GlobalOffset = 0;
    int *XPtr = &X_GlobalOffset;

    int Y_GlobalOffset = 0;
    int *YPtr = &Y_GlobalOffset; 
    int p;
    int j = 0;
    int line = 0;
    float X_local = 0;
    float Y_local = 0;
    int line_spacing = 5;   //Spacing between each line of characters

    
    for (int i = 0; i <= WordCount; i++) 
    {
        All_Words[i].characters = NULL;  // Initialize the characters pointer to NULL

        // Check if the characters pointer is NULL and print the result
        if (All_Words[i].characters == NULL) 
        {
            printf("All_Words[%d].characters is NULL\n", i);
        } 
        else 
        {
            printf("All_Words[%d].characters is NOT NULL\n", i);
        }
    }
    
    //NewLine(RunningP, scale, X_Limit, &X_GlobalOffset, line);
    //printf("%s", All_Words->characters);

    printf("%f\n", all_lines[0].a0);

    for (int word = 0; word <= WordCount; word++)
    {
        //printf("After strcpy: All_Words[%d].characters = '%s'\n", word, All_Words[word].characters);
        ReadWord(fPtr1, All_Words, &Running_CharCount, word, WordCount);
        //printf("In main: All_Words[%d].characters = %p, contents = %s\n", word, (void *)All_Words[word].characters, All_Words[word].characters);
        NewLine(&Running_CharCount, user_scale, X_Limit, XPtr, line, &character_position);
        for (int w = 0; All_Words[word].characters[w] != '\0'; w++ ) //Does it reach that?
        {
            if (All_Words[word].characters == NULL) 
            {
                printf("Error: All_Words[%d].characters is NULL\n", word);
                continue;
            }
            for (p = 0; p <= SingleStrokeFont_NumberOfRows; p++)
            {
                if (all_lines[p].a0 == 999 && all_lines[p].a1 == All_Words[word].characters[w])
                {
                    p++;
                    while (all_lines[p].a0 != 999)
                    {
                        //printf("Calling x_coordinate for p = %d\n", p);  // Debug log
                        float x_local = x_coordinate(&X_GlobalOffset, user_scale, &character_position, all_lines, p, X_local);
                        float y_local = y_coordinate(&Y_GlobalOffset, user_scale, line, line_spacing, all_lines, p, Y_local);
                        snprintf(buffer, 100, "G%d X%f Y%f\n", all_lines[p].a2, x_local, y_local);
                        //SendCommands(buffer);
                        printf ("%s", buffer);
                        p++;
                    }
                    (*characterPtr)++;
                }
            }
        }
    }
    
    free(All_Words);
}  

//Function to read a word and allocate it to the structure 'word'
void ReadWord(FILE *fPtr1, struct Word *All_Words, int *RunningP, int word, int WordCount) //arguments for the function
{ 
    int charCount = 0;  //initialises character count to 0
    char temp[50] = {0};  //temporary array used to store the characters, which will be copied to the array in '*characters'
    char ch;    
    while((ch = fgetc(fPtr1)) != EOF)   //Goes through each character in the text file that 'fPtr1' is pointing at
    {
        if (ch != ' ' && ch != '\n' && ch !='\t')      //If there isn't a space, new line or tab, execute the if statement
        {
            temp[charCount] = ch;   //Add the character to the temp array
            charCount++;    //increment the character count by 1
            (*RunningP)++;     //increments the running count value by 1
        }
        else //If there's a space, new line or tab, execute the else if statement
        {
            if (charCount >= sizeof(temp) - 1) 
            {
                printf("Error: Word too long, exceeding buffer size!\n");
                exit(1);
            }   
            
            if (word >= WordCount) 
            {
                printf("Word index out of bounds: %d\n", word);
                exit(1);
            }
            temp[charCount] = ch;
            charCount++;    //increment the character count by 1
            if (ch == ' ') 
            {
                (*RunningP)++;  // Increment running position for spaces
            
            }
            else if (ch == '\t') 
            {
                (*RunningP)+=2;  // Increment running position for tab
            
            }
            temp[charCount] = '\0'; //append the temp array with a null terminator (used to copy the string in temp into the 'All_words.characters array)
            
            /*if (All_Words[word].characters != NULL) 
            {
                //free(All_Words[word].characters);
                All_Words[word].characters = NULL;
            }*/

            //printf("Before allocation: All_Words[%d].characters = %p\n", word, (void *)All_Words[word].characters);
            //printf("befre allocation : All_Words[%d].characters = '%s'\n", word, All_Words[word].characters);
            All_Words[word].characters = (char *)malloc((strlen(temp) + 1) * sizeof(char));    //dynamically allocate memory to the characters array 
            //printf("After allocation: All_Words[%d].characters = %p\n", word, (void *)All_Words[word].characters);
            //printf("after allocation : All_Words[%d].characters = '%s'\n", word, All_Words[word].characters);
            if (All_Words[word].characters == NULL)    //Execute if statement that exits the if loop if there's not sufficient memory
            {
                printf("ERROR ALLOCATING MEMORY!\n");
                exit(1);
            }
            //printf("temp length: %zu\n", strlen(temp));
            //printf("Before strcpy: temp = '%s'\n", temp);
            strcpy(All_Words[word].characters, temp);  //copies the contents of the temp array to the strucuture all_words.characters
            //printf("After allocation: All_Words[%d].characters = %p\n", word, (void *)All_Words[word].characters);
            //printf("After strcpy: All_Words[%d].characters = '%s'\n", word, All_Words[word].characters);
            //(*RunningP)++;     //increments the running count value by 1
            //printf("word index: %d, WordCount: %d\n", word, WordCount);
            return;
        }
    }
    if (charCount > 0) 
    { 
        temp[charCount] = '\0';  // Null-terminate the temp array
        All_Words[word].characters = (char *)malloc((strlen(temp) + 1) * sizeof(char));
        if (All_Words[word].characters == NULL) 
        { 
            printf("ERROR ALLOCATING MEMORY!\n");
            exit(1);
        }
        strcpy(All_Words[word].characters, temp);  // Copy temp to the structure
    }
}

int NewLine(int *RunningP, float user_scale, const int X_Limit, int *XPtr, int line, int *characterPtr)
{
    int length = (*RunningP) * user_scale;  //float to int????
    printf("length is %d\n", length);
    if (length > X_Limit)
    {
        *XPtr = 0;
        *RunningP = 0;
        line++;
        *characterPtr = 0;
        return line;
    } 
}


float x_coordinate(int *XPtr, int user_scale, int *characterPtr, struct line *all_lines, int p, float X_local)
{
    *XPtr = user_scale * (*characterPtr);
    X_local = *XPtr + (all_lines[p].a0);
    //printf("%f  %d   %f\n", all_lines[p].a0, *XPtr, X_local);
    return X_local;
}

float y_coordinate(int *YPtr, int user_scale, int line, int line_spacing, struct line *all_lines, int p, float Y_local)
{
    *YPtr = 0 - user_scale - (user_scale * line) - (line_spacing * line);      //Works out global Y coordinate
    Y_local = *YPtr + (all_lines[p].a1);
    //printf("%f  %d   %f\n", all_lines[p].a1, *YPtr, Y_local);
    return Y_local;
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