#include <stdio.h>

struct line
{ 
    int a0;
    int a1;
    int a2;
};

struct ASCIItable
{
    int b0;
    char b1;
};

int main()
{
    int iSize = 1027; 
    int iSizeTable = 127;
    int i;
    int j;
    int k;
    struct line AllLines[iSize];
    struct ASCIItable AllTable[iSizeTable];

    FILE *fPtr, *fPtrASCII; 
    fPtr = fopen("SingleStrokeFont.txt", "r" );
    fPtrASCII = fopen("ASCII.txt", "r");

    if (fPtr == NULL)
    {
        printf("NO!");
    } 
    else if (fPtrASCII == NULL)
    {
        printf("NO AGAIN!");
    }

    else
    {
        for (i = 0; i < iSize; i++)
        {
            fscanf(fPtr, "%d %d %d ", &AllLines[i].a0, &AllLines[i].a1, &AllLines[i].a2);  
        }
    }

    /*for (int i = 0; i < 9; i++)
    {
        printf("%d %d %d\n", AllLines[i].a0, AllLines[i].a1, AllLines[i].a2);
    } */


    int ASCII;
    printf("Please enter the ASCII symbol you want to type in: ");
    scanf("%d", &ASCII);
    
    for (k = 0 ; k < iSize ; k++)
    { 
        if (AllLines[k].a0 == 999 && AllLines[k].a1 == ASCII)
        {   
            printf("The Ascii code for %d is:\n", ASCII);
            do
            {
                printf("%d %d %d\n", AllLines[k].a0, AllLines[k].a1, AllLines[k].a2);
                k++; 
            }
            while (AllLines[k].a0 != 999);
        }   
        else
        {
            printf("invalid number");
        }
    }   

    fclose(fPtr);
    return 0;
}