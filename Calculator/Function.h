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
#include <math.h>

#define MAXGRADE UCHAR_MAX
#define MAXDIF 1                    //maximal Deviation for numeric solving
#define STEP 0.0001                  //Step size for numeric solving

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
    double i = 1;
    double bOld = (in[1].y - in[0].y) / (pow(i,in[1].x) - pow(i,in[0].x)) * (pow(i,in[2].x) - pow(i,in[0].x)) - (in[2].y - in[0].y);
    while (1)
    {
        double bTemp = fabs((in[1].y - in[0].y) / (pow(i,in[1].x) - pow(i,in[0].x)) * (pow(i,in[2].x) - pow(i,in[0].x)) - (in[2].y - in[0].y));
        if(bTemp <= MAXDIF && bOld < bTemp)
        {
            out.b = i - STEP;
            break;
        }
        bOld = bTemp;
        i += STEP;
    }
    out.a = (in[1].y - in[0].y) / (pow(out.b,in[1].x) - pow(out.b,in[0].x));
    out.c = in[0].y -(out.a * pow(out.b,in[0].x));
    return out;
}

double exValue(const struct exFunction f, double x)                   //Calculate Functionvalue f(x)
{
    return f.a*pow(f.b,x) + f.c;
}

struct exFunction approxeExFunction(const struct point *in, unsigned int n)     //Aproximate a Exponentinal Function from in with size n+1
{
    struct point ePoints[3];
    ePoints[0] = in[0];
    ePoints[1] = in[n/2];
    ePoints[2] = in[n];

    struct exFunction f = claculateExFunction(ePoints);
    struct exFunction fOut = f;
    double error = 0;
    for (unsigned int l = 0; l < n+1; l++)
    {
        error += pow(fabs((in[l].y - exValue(f, (in[l]).x))),2);
    }

    unsigned long it = 0;

    for (double i = (f.a*0.9); i < (f.a*1.10); i+= STEP*10)
    {
        for (double j = (f.b*0.9); j < (f.b*1.10); j+= STEP)
        {
            for (double k = (f.c - fabs(f.c*0.1)); j > (f.c + fabs(f.c*0.1)); k+= STEP*10)
            {

                struct exFunction fTemp;
                fTemp.a = i;
                fTemp.b = j;
                fTemp.c = k;
                double errorTemp = 0;
                for (unsigned int l = 0; l < n+1; l++)
                {
                    errorTemp += pow(fabs((in[l].y - exValue(fTemp, (in[l]).x))),2);
                    if(errorTemp > error)
                        break;
                }
                it++;
                //printf("i: %g j: %g k: %g errorTemp: %g error: %g attemps: %ld\n", i, j, k, errorTemp, error, it);
                if(errorTemp < error)
                {
                    fOut = fTemp;
                }
            }
            
        }
        
    }
    

    return fOut;
}