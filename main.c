#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <conio.h>
//#include <windows.h>
#include "rs232.h"
#include "serial.h"

#define bdrate 115200               /* 115200 baud */

//Function prototype to send commands to the robot
void SendCommands (char *buffer );


//Initialises structure that is used to store each word from a text file and it's characters
struct single_word     
{
    char *characters;    //pointer to dynamically store the characters array
};


//Initialises a structure that will be used to store the data from the "SingleStrokeFont.txt" file
struct SSF_char
{
    float a0;     //holds the value of the x movement
    float a1;     //holds the value of the y movement
    int a2;     //holds the value of whether the pen is up or down
};

//Function Prototypes for tasks in the program
void scale_SSFData(FILE *fPtr, struct SSF_char *SSF_lines, int SSF_NumberOfRows, float scale);
void ReadWord(int *inword_characterPtr, FILE *fPtr1, struct single_word *all_words, int *runningPtr, int word, int WordCount);  //Function prototype to read a single word and process it
int NewLine(int *runningPtr, float user_scale, const int X_Limit, int *XPtr, int line, int *characterPtr, int *inword_characterPtr);   //Function prototype to assess whether a new line is needed 
float x_coordinate(int *XPtr, int user_scale, int *characterPtr, struct SSF_char *SSF_lines, int p, float X_local);     //Function prototype to work out the X coordinate
float y_coordinate(int *YPtr, int user_scale, int line, const int line_spacing, struct SSF_char *SSF_lines, int p, float Y_local);  //Function prototype to work out the y coordinate


int main()
{
    
    char mode[]= {'8','N','1',0};
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
   

    //asks the user for an input which will be used for font scaling the SingleStrokeFont.txt file
    float user_scale;      //Initialising variable to hold user inputted scale
    const float default_scale = 18.0;      //default scale used in the SingleStrokeFont file
    
    do
    {
        printf("Please input the scaling you would like for the font (must be between 4 and 10, inclusive): ");
        scanf("%f", &user_scale);       //Assigns user input to user_scale

        if (user_scale < 4 || user_scale > 10)
        {
            printf("ERROR: invalid input, the scaling must be between 4 and 10. \n");
        }
    } while (user_scale < 4 || user_scale > 10);

    float scale = user_scale / default_scale;   //calculates correct scaling


    /*This section copies the data to the 'line' structure, where it is multiplied by the scaling values 
    for the relevant x and y movements*/  //MENTIONED LINE????
    const int SSF_NumberOfRows = 1027; //total number of rows (as said in the project brief document) used to work out the number of structural arrays
    
    FILE *fPtr; //Assigns a pointer to the file 
    fPtr = fopen("SingleStrokeFont.txt", "r" );  //Opens the SingleStrokeFont file in read only

    //This checks whether the file was openend correctly or not, if it wasn't then the program will exit
    if (fPtr == NULL) 
    {
        printf("ERROR: Could not open \"SingleStrokeFont!\"");
        exit(1);
    }


    struct SSF_char SSF_lines[SSF_NumberOfRows]; //Defines a structural array which will contain each line of the SingleStrokeFont file 
    
    scale_SSFData(fPtr, SSF_lines, SSF_NumberOfRows, scale);    //Calls function to 

    fclose(fPtr); //Closes the SingleStrokeFont file


    //Opens the "SingleWordTest" file for reading input text
    FILE *fPtr1; //Assigns a pointer to the file 
    fPtr1 = fopen("SingleWordTest.txt", "r");

    //This checks whether the file was openend correctly or not, if it wasn't then the program will exit
    if (fPtr1 == NULL) 
    {
        printf("ERROR: Could not open \"SingleStrokeTest!\"");
        exit(1);
    }


    //Counts how many words there are in the file to know how many structures are needed
    int WordCount = 0;  //initialises word count
    char ch;    //Used to loop over the file characters

    while((ch = fgetc(fPtr1)) != EOF)     //Goes through each character in the text file that 'fPtr1' is pointing at 
    {
        //If there's a space, new line or tab, increment the word count
        if (ch == ' ' || ch == '\n' || ch =='\t')   
        {
            WordCount++;   
        }
    }

    rewind(fPtr1);  //Resets the file position to the beginning of the file 

     //creates an array of structures to store each word from the input file
    struct single_word all_words[WordCount];   


    //Initialising variables for the rest of the code and functions
    //variables to track character positioning
    int running_CharCount = 0;  //Keeps a running count of ALL characters (multiple words)
    int *runningPtr = &running_CharCount;     //pointer to the running char count variable

    int character_position = 0;  //Keeps a count of the current character position in a SINGLE word (for iterating through a word)
    int *characterPtr = &character_position;    //pointer to the character position variable

    int inword_CharCount;    //Keeps a count of the character count INSIDE a single word
    int *inword_characterPtr = &inword_CharCount;        //pointer to the Inword charCount variable 
    
    //Variables for tracking x and y coordinates
    int X_GlobalOffset = 0;    // X-axis global offset (used to shift the drawing position of the letters)
    int *XPtr = &X_GlobalOffset;    //pointer to the X-axis global offset
    float X_local = 0;   //Local X-coordinate used in drawing the characters 

    int Y_GlobalOffset = 0;     // Y-axis global offset (used to shift the drawing position of the letters)
    int *YPtr = &Y_GlobalOffset;    //pointer to the Y-axis global offset
    float Y_local = 0;   //Local y-coordinate used in drawing the characters

    const int X_Limit = 100;   //the x-axis limit for the character drawing
    const int line_spacing = 5;   //Spacing between each line of characters
    int line = 0;   //Initialises line count (used to adjust the Y-coordinates)
    

    //Loops through each word in the all_words array to set each 'characters' pointer to NULL
    for (int i = 0; i <= WordCount; i++) 
    {
        all_words[i].characters = NULL;  // Initialize the characters pointer to NULL

        // Check if the characters pointer is NULL and print the result
        if (all_words[i].characters == NULL) 
        {
            printf("all_words[%d].characters is NULL\n", i);
        } 
        else 
        {
            printf("all_words[%d].characters is NOT NULL\n", i);
        }
    }

    
    //These commands get the robot into 'ready to draw mode' and need to be sent before any writing commands
    /*sprintf (buffer, "G1 X0 Y0 F1000\n");
    SendCommands(buffer);
    sprintf (buffer, "M3\n");
    SendCommands(buffer);
    sprintf (buffer, "S0\n");
    SendCommands(buffer);*/

    /*For loop used to print out the G-Code for each character movement. The array reads in a single word,
    decides if it's within the x-axis width limit then loops through the characters in the 
    all_words.characters array. It then scans the SSF_lines struct for the specific character movements
    corresponding to the current letter, and outputs it.*/

    //Processes each word in the all_words array, word by word
    for (int word = 0; word <= WordCount; word++)   
    {
        ReadWord(&inword_CharCount, fPtr1, all_words, &running_CharCount, word, WordCount);  //Reads a word from the text file and updates the all_words[word].characters array

        line = NewLine(&running_CharCount, user_scale, X_Limit, XPtr, line, &character_position, &inword_CharCount);  //Determines whether a newline is needed or not 
        
        //Loops through each character in a word
        for (int w = 0; all_words[word].characters[w] != '\0'; w++ ) 
        {
            //Checks if the character array is NULL, and prints an error message
            if (all_words[word].characters == NULL) 
            {
                printf("ERROR: all_words[%d].characters is NULL\n", word);
                continue;   //Skip to the next iteration of the character array is NULL
            }
            //iterates through each line of the 'SingleStrokeFont' file to get the character movements
            for (int p = 0; p <= SSF_NumberOfRows; p++)
            {
                //Checks in between each a0 == 999 and a1 == current character value to find the character movements
                if (SSF_lines[p].a0 == 999 && SSF_lines[p].a1 == all_words[word].characters[w])
                {
                    p++;    //Moves to the line after a0 == 999 to access character movement data
                    //While the line is inbetween 999 (i.e. while it doesn't indicate the end of the character movements of a letter)
                    while (SSF_lines[p].a0 != 999)
                    {
                        //Calls functions to calculate character movement in the X and Y coordinates for each letter 
                        float x_local = x_coordinate(&X_GlobalOffset, user_scale, &character_position, SSF_lines, p, X_local);   
                        float y_local = y_coordinate(&Y_GlobalOffset, user_scale, line, line_spacing, SSF_lines, p, Y_local);
                        snprintf(buffer, 100, "G%d X%f Y%f\n", SSF_lines[p].a2, x_local, y_local);
                        //SendCommands(buffer); //Function commented out to allow for testing on G-Code simulator
                        
                        printf ("%s", buffer); //Prints out the G-Code to allow for tesing on G-code simulator
                        p++;    //Moves to the next line of the SSF_lines structure
                    }
                    (*characterPtr)++;  //Moves to the next character in the word
                }
            }
        }
    }
    snprintf(buffer, 100, "G0 X0 Y0\n");
    printf ("%s", buffer); //Prints out the G-Code to allow for tesing on G-code simulator
    //free(all_words);    //dynamically frees the memory located in the all_words array
    
    // Before we exit the program we need to close the COM port
    /*CloseRS232Port();
    printf("Com port now closed\n");*/

    return (0);
}  

//Function to scale the values of the SSF_char arrays based on the user scale
void scale_SSFData(FILE *fPtr, struct SSF_char *SSF_lines, int SSF_NumberOfRows, float scale)
{
    //The for loop iterates over the structural array and copies the values from the SingleStrokFont file to it
    for (int i = 0; i < SSF_NumberOfRows; i++)
    {
        int temp_a0, temp_a1, temp_a2;  //Declares temporary variables used to store float versions of the data from SingleStrokeFont
        fscanf(fPtr,"%d %d %d", &temp_a0, &temp_a1, &temp_a2); //reads each value (seperated by a space) and copies it to the corresponding value in the 'lines' struct
        
        if (temp_a0 != 999)     //If NOT a0 (therefore working with the lines inbetween 999 - the x and y coordinates), execute the code inside the if statement:
        {
            SSF_lines[i].a0 = (float)temp_a0 * scale;   //assign the SSF_char struct values to the scaled SingleStrokeFont data (x coordinates)
            SSF_lines[i].a1 = (float)temp_a1 * scale;   //assign the SSF_char struct values to the scaled SingleStrokeFont data (y coordinates)
        }

        else    //if a0 IS 999 (indicating the start of a new character), execute the code inside the else statement:
        {
          SSF_lines[i].a0 = temp_a0;    //Simply assigns the value of the temp placeholder to the SSF_char struct value
          SSF_lines[i].a1 = temp_a1;    //Simply assigns the value of the temp placeholder to the SSF_char struct value
        }
        SSF_lines[i].a2 = temp_a2;      //Since a2 does not get affected by scaling, all elements are equated to the temp_a2 value
    }
}

//Function to read a word and allocate it to the structure 'single_word'
void ReadWord(int *inword_characterPtr, FILE *fPtr1, struct single_word *all_words, int *runningPtr, int word, int WordCount) //arguments for the function
{ 
    *inword_characterPtr = 0;  //initialises character count to 0
    char temp[50] = {0};  //temporary array used to store the characters, which will be copied to the array in '*characters'
    char ch;    
    while((ch = fgetc(fPtr1)) != EOF)   //Goes through each character in the text file that 'fPtr1' is pointing at until the EOF is reached
    {
        //If there isn't a space, new line or tab, execute the if statement
        if (ch != ' ' && ch != '\n' && ch !='\t')      
        {
            temp[*inword_characterPtr] = ch;   //Add the character to the temp array
            (*inword_characterPtr)++;    //increment the character count by 1
            (*runningPtr)++;     //increments the running character count value by 1
        }
        //If there's a space, new line or tab, execute the else statement
        else 
        {
            //If statement to ensure the word is smaller than the temp array
            if (*inword_characterPtr >= sizeof(temp) - 1) 
            {
                printf("ERROR: Word length exceeds the temporary buffer size.\n");
                exit(1);    //Exits if the word is longer than the temp array
            }   
            
            //Ensures that the indexing of the word is correct and does not go out of bounds
            if (word >= WordCount) 
            {
                printf("Word index out of bounds: %d\n", word);
                exit(1);    //Exits if the word indexing is out of bounds
            }

            //If the characters array is not NULL, the if statement will declare it NULL 
            if (all_words[word].characters != NULL) 
            {
                all_words[word].characters = NULL;  
            }

            temp[*inword_characterPtr] = ch;    //adds a space/tab/newline to the temp array
            //(*inword_characterPtr)++;    //increment the character count by 1

            //Adjusts the running and character count by 1 for spaces
            if (ch == ' ') 
            {
                (*runningPtr)++;  
                (*inword_characterPtr)++;  
            
            }
            //Adjusts the running and character count by 4 for tabs
            else if (ch == '\t') 
            {
                (*runningPtr)+=4;  
                (*inword_characterPtr)+=4;    
            }

            temp[*inword_characterPtr] = '\0'; //append the temp array with a null terminator (used to copy the string in temp into the 'all_words.characters array)
            
            all_words[word].characters = (char *)malloc((strlen(temp) + 1) * sizeof(char));    //dynamically allocates memory to the characters array based on the word length 
            
            //Execute if statement that exits the if loop if there's not sufficient memory
            if (all_words[word].characters == NULL)  
            {
                printf("ERROR: Not enough memory allocated.\n");
                exit(1);    //Exit if memory allocation fails
            }

            strcpy(all_words[word].characters, temp);  //copies the contents of the temp array to the strucuture all_words.characters array
            return;     //Exits the function after processing the word
        }
    }
    //For words at the end of the file without a space/tab/newline at the end
    if (*inword_characterPtr > 0) 
    { 
        temp[*inword_characterPtr] = '\0';  // Null-terminate the temp array
        all_words[word].characters = (char *)malloc((strlen(temp) + 1) * sizeof(char)); //dynamically allocates memory to the characters array based on the word length 
        
        //Execute if statement that exits the if loop if there's not sufficient memory
        if (all_words[word].characters == NULL) 
        { 
            printf("ERROR: Not enough memory allocated.\n");
            exit(1);    //Exit if memory allocation fails
        }
        strcpy(all_words[word].characters, temp);  //copies the contents of the temp array to the strucuture all_words.characters array
        return;     //Exits the function after processing the word
    }
}

//Function that checks the current position of a word, and determines if it should go on a new line
int NewLine(int *runningPtr, float user_scale, const int X_Limit, int *XPtr, int line, int *characterPtr, int *inword_characterPtr)
{  
    int length = (*runningPtr) * user_scale;  //Calculates the length of a word based on the running character position and the user_scale
    //Checks if the current word length is over the X limit
    if (length > X_Limit)
    {
        //*XPtr = 0;      //Resets the X position (starts a new line)
        *runningPtr = *inword_characterPtr;   //Updates the running character position of the new line depending on the length of the current word read
        line++; //Moves the word to the next line
        *characterPtr = 0;  //Resets the character position for the new line
        return line;    //returns the incremented line number
    } 
    return line;    //returns the current line number if no new line is needed
}

//Function to calculate the X coordinate based on the scaling and current character position
float x_coordinate(int *XPtr, int user_scale, int *characterPtr, struct SSF_char *SSF_lines, int p, float X_local)
{
    *XPtr = user_scale * (*characterPtr);   //Scale the X position based on the user-defined scale and character position
    X_local = *XPtr + (SSF_lines[p].a0);    //Add the character's offset (a0) to the scaled X position
    return X_local;     //Return the final local X coordinate
}

//Function to calculate the Y coordinate based on the scaling and current line
float y_coordinate(int *YPtr, int user_scale, int line, const int line_spacing, struct SSF_char *SSF_lines, int p, float Y_local)
{
    *YPtr = 0 - user_scale - (user_scale * line) - (line_spacing * line);      //Scale the Y position based on the user-defined scale and current line
    Y_local = *YPtr + (SSF_lines[p].a1);       //Add the character's offset (a1) to the scaled Y position
    return Y_local;  //Return the final local Y coordinate
}

/*



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