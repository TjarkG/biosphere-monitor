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
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "Function.h"

#define mat_elem(a, y, x, n) (a + ((y) * (n) + (x)))

#define FLEND ".csv"
#define MAXFIELD 16

bool getField(FILE *ifp, char *field);
void swap_row(double *a, struct point *p, int r1, int r2, int n);
void gauss_eliminate(struct point *p, double *x, int n);

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
    struct exFunction eFunction;
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

    printPointArray(nPoints, nFunction.grade+1, stdout); //Print found Points
 
	gauss_eliminate(nPoints, nFunction.a, nFunction.grade +1); //Get rational function

    printRtFunction(nFunction, stdout);       //Print found function

    printExFunction(eFunction, stdout);       //Print found function
    
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

void swap_row(double *a, struct point *p, int r1, int r2, int n)
{
	double tmp, *p1, *p2;
	int i;
 
	if (r1 == r2) return;
	for (i = 0; i < n; i++)
    {
		p1 = mat_elem(a, r1, i, n);
		p2 = mat_elem(a, r2, i, n);
		tmp = *p1, *p1 = *p2, *p2 = tmp;
	}
	tmp = p[r1].y, p[r1] .y= p[r2].y, p[r2].y = tmp;
}
 
void gauss_eliminate(struct point *p, double *x, int n)
{
#define A(y, x) (*mat_elem(a, y, x, n))

    double a[n * n];
    for (int i = 0; i < n; i++)     //safe all needed exponents
    {
        for (int j = 0; j < n; j++)
        {
            int pos = (i*(n))+j;
            a[pos] = pow(p[i].x, j);
        }
    }

	int i, j, col, row, max_row,dia;    //Perform Gaussia Elimination
	double max, tmp;
 
	for (dia = 0; dia < n; dia++)
    {
		max_row = dia, max = A(dia, dia);
 
		for (row = dia + 1; row < n; row++)
			if ((tmp = fabs(A(row, dia))) > max)
				max_row = row, max = tmp;
 
		swap_row(a, p, dia, max_row, n);
 
		for (row = dia + 1; row < n; row++)
        {
			tmp = A(row, dia) / A(dia, dia);
			for (col = dia+1; col < n; col++)
				A(row, col) -= tmp * A(dia, col);
			A(row, dia) = 0;
			p[row].y -= tmp * p[dia].y;
		}
	}
	for (row = n - 1; row >= 0; row--)
    {
		tmp = p[row].y;
		for (j = n - 1; j > row; j--)
			tmp -= x[j] * A(row, j);
		x[row] = tmp / A(row, row);
	}
#undef A
}