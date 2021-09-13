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

#define FLEND ".csv"

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
    printf("opend File! %s\n", infile);
    fclose(in);
}