/*
 * Function.c
 *
 * Function structs and functions to use them
 *
 * Created: 13.09.2021 20:40:50
 * Author : Tjark Gaudich
 */

#include <limits.h>
#include <stdio.h>

#define MAXGRADE UCHAR_MAX

struct rtFunction                   //rational Function
{
    unsigned char grade;            //grade of the function
    double a[MAXGRADE];             //coefficient for each grade (e.g. a[2] for x²)

    double min;                     //lower and upper boundery of the domain of definition
    double max;
};

struct exFunction                   //exponential function f(x) = a*b^x +c
{
    double a;
    double b;
    double c;
};

struct point                        //Struct containing a point
{
    double x;
    double y;
};

void printRtFunction(const struct rtFunction in, FILE *ofp)
{
    fprintf(ofp, "f(x) = ");
    for (int i = in.grade; i > 0; i--)
    {
        fprintf(ofp, "%gx^%d + ",in.a[i],i);
    }
    fprintf(ofp, "%g ",in.a[0]);
    fprintf(ofp, "for ID = {%g < x < %g}\n",in.min, in.max);
}

void printExFunction(const struct exFunction in, FILE *ofp)
{
    fprintf(ofp, "f(x) = %g*%g^x + %g\n", in.a, in.b, in.c);
}

void printPoint(const struct point in, FILE *ofp)
{
    fprintf(ofp, "P (%g|%g)\n", in.x, in.y);
}

void printPointArray(const struct point *in, const unsigned int size, FILE *ofp)
{
    for (int i = 0; i < size; i++)
    {
        printPoint(in[i], ofp);
    }
    
}

struct exFunction claculateExFunction(const struct point *in)       //calculate Exponential Function from an array of 3 Points
{
    struct exFunction out;
    out.c = 0;
    out.b = 0;
    out.a = 0;
    return out;
}

double exValue(const struct exFunction f, double x)                   //Calculate Functionvalue f(x)
{
    return f.a*pow(f.b,x) + f.c;
}