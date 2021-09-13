/*
 * Calculator.c
 *
 * Tool to convert a List of Points provided in a csv (first argument) to a rational function and compute the closest aproximation each lower grade
 *
 * Created: 13.09.2021 18:36:45
 * Author : Tjark Gaudich
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include "rtFunction.h"

#define FLEND ".csv"
#define MAXFIELD 16

bool getField(FILE *ifp, char *field);

int main(int argc, char *argv[])
{
    if (argc == 1 )             // no args: throw error
    {
        fprintf(stderr, "Error: first argument must be input File\n");
        return -1;
    }

    char *infile = argv[1];     //Check if the input File is a .csv
    if(strncmp(FLEND,(infile+strlen(infile)+1-sizeof(FLEND)),sizeof(FLEND)))
    {
        fprintf(stderr, "Error: Input File must be .csv\n");
        return -2;
    }

    FILE *in;                   //Open File
    errno =  0;
    if ((in = fopen(infile, "r")) == NULL) 
    {
        fprintf(stderr, "Error: opening %s: %s\n", infile, strerror(errno));
        return -3;
    }

    struct point nPoints[MAXGRADE];     //store limits and grade in nFunction and all points in nPoints
    struct rtFunction nFunction;
    int i = -2;
    while (i <= 2*MAXGRADE)
    {
        char field[MAXFIELD];
        bool eof = getField(in, field);

        if(isdigit(field[0]))
        {
            if(i == -2)
                nFunction.min = atof(field);
            else if(i == -1)
                nFunction.max = atof(field);
            else if(i % 2 == 0)
                nPoints[i/2].x = atof(field);
            else
                nPoints[i/2].y = atof(field);
            i++;
        }

        if(eof)
            break;
    }
    nFunction.grade = (i/2)-1;

    printPointArray(nPoints, nFunction.grade+1, stdout);
    printFunction(nFunction, stdout);       //Print found function
    
    fclose(in);
    return 0;
}

bool getField(FILE *ifp, char *field)       //puts current input field in *field, returns 1 for end of File found
{
    for(int i = 0; i<MAXFIELD; i++)
    {
        char c = getc(ifp);
        field[i] = '\0';
        if(c == EOF)
            return 1;
        else if(c == ',' || c == '\n')
            return 0;
        else
            field[i] = c;
    }
    fprintf(stderr, "Error: Input Field larger than %d characters\n", MAXFIELD);
}