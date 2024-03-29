//Writen by TjarkG and published under the MIT License

#include <stdio.h>
#include <stdbool.h>
#include <gtk/gtk.h>
#include <errno.h>
#include "../PC/biosphere.hpp"
#include "../PC/tty.hpp"
#include "../reading.h"

#define memb(a) (sizeof(a)/sizeof(a[0]))
#define INTV    1   //time between refreshes

GtkBuilder *builder;
const char *labelNames[] = {"outTime","outLight","outOTemp","outITemp","outPress","outAir","outSoil","outBegin","outInv","outLast","outSum","outStorage"};
char labelContent[memb(labelNames)][16];
GtkWidget *labels[memb(labelNames)];
guint timerId = 0;
unsigned int lnCnt = 0;
int intervall = 0;
bool isConnected = false;
char *portname;

void initStats(void);

#include "signals.hpp"

void getLabels(void)
{
    for (unsigned int i = 0; i < memb(labelNames); i++)
        labels[i] = GTK_WIDGET (gtk_builder_get_object (builder, labelNames[i]));
}

void selectableLabels(bool sl)
{
    for (unsigned int i = 0; i < memb(labelNames); i++)
        gtk_label_set_selectable(GTK_LABEL(labels[i]), sl);
}

gboolean timerTick(__attribute__((unused)) gpointer userData)
{
    char buf[64];
    setCommand("CR");
    getUartLine(buf);
    struct reading in = getReading(buf);

    struct tm lt;
    lt = *gmtime(&in.timeRead);
    strftime(buf, sizeof(buf), "%d.%m.%Y %H:%M:%S UTC", &lt);
    gtk_label_set_label(GTK_LABEL(labels[0]), buf);
    if(in.timeRead % intervall < INTV)
    {
        time_t lastMessurment = (in.timeRead - (in.timeRead % intervall));
        lt = *gmtime(&lastMessurment);
        strftime(buf, sizeof(buf), "%d.%m.%Y %H:%M:%S UTC", &lt);

        gtk_label_set_label(GTK_LABEL(labels[9]), buf);
        sprintf(buf,"%d", ++lnCnt);
        gtk_label_set_label(GTK_LABEL(labels[10]), buf);
        sprintf(buf,"%.1f/4096kb", lnCnt/(1024.0/16.0));      //1024 byte per Kb and 16 byte per Messurment
        gtk_label_set_label(GTK_LABEL(labels[11]), buf);
    }

    sprintf(buf,"%d lux", in.light);
    gtk_label_set_label(GTK_LABEL(labels[1]), buf);
    sprintf(buf,"%2.1f°C", in.temperaturOut/(double) SCALE_TOut);
    gtk_label_set_label(GTK_LABEL(labels[2]), buf);
    sprintf(buf,"%2.1f°C", in.temperaturIn/(double) SCALE_TIn);
    gtk_label_set_label(GTK_LABEL(labels[3]), buf);
    if(in.pressure != 0)
    {
    sprintf(buf,"%2.1fhPA", in.pressure/(double) SCALE_Pres);
    gtk_label_set_label(GTK_LABEL(labels[4]), buf);
    }
    if(in.humidityAir != 0)
    {
        sprintf(buf,"%d%%", in.humidityAir);
        gtk_label_set_label(GTK_LABEL(labels[5]), buf);
    }
    sprintf(buf,"%d%%", in.humiditySoil);
    gtk_label_set_label(GTK_LABEL(labels[6]), buf);
    return TRUE;
}

void initStats(void)
{
    isConnected = false;
    if(startUART(portname) != 0)
        return;

    char buf[64];
    //reset Labels
    for (unsigned int i = 0; i < memb(labelNames); i++)
        gtk_label_set_label(GTK_LABEL(labels[i]), "n/A");

    intervall = getCommand("IG");
    if(intervall < 0)       //connection timed out
        return;
    isConnected = true;
    
    //get Intervall
    if(intervall % 3600 == 0)
        sprintf(buf,"%d h", intervall/3600);
    else if(intervall % 60 == 0)
        sprintf(buf,"%d min", intervall/60);
    else
        sprintf(buf,"%ds", intervall);
    gtk_label_set_label(GTK_LABEL(labels[8]), buf);

    //download data to a buffer
    struct reading *buffer = static_cast<reading *>(malloc((sizeof(struct reading) * 262144UL)));
    lnCnt = bufferReadings(buffer);

    //display first and last timestamp
    if(lnCnt > 0)
    {
        struct tm lt;
        lt = *gmtime(&buffer[0].timeRead);
        strftime(buf, sizeof(buf), "%d.%m.%Y %H:%M:%S UTC", &lt);
        gtk_label_set_label(GTK_LABEL(labels[7]), buf);

        lt = *gmtime(&buffer[lnCnt-1].timeRead);
        strftime(buf, sizeof(buf), "%d.%m.%Y %H:%M:%S UTC", &lt);
        gtk_label_set_label(GTK_LABEL(labels[9]), buf);
    }

    //display number of readings
    sprintf(buf,"%d", lnCnt);
    gtk_label_set_label(GTK_LABEL(labels[10]), buf);

    //calculate storage usage
    sprintf(buf,"%.1f/4096kb", lnCnt/(1024.0/16.0));      //1024 byte per Kb and 16 byte per Messurment
    gtk_label_set_label(GTK_LABEL(labels[11]), buf);

    //start timer
    timerId = g_timeout_add(INTV*1000,timerTick,NULL);
    timerTick(NULL);
    free(buffer);
}

int main(int argc,char **argv) 
{
    gtk_init (&argc , &argv);  
    builder = gtk_builder_new(); 
    if(errno = gtk_builder_add_from_file(builder,"/usr/local/src/bioGui.glade" , NULL) == 0)
    {
        printf("gtk_builder_add_from_file failed: %s\n", strerror(errno));
        return(errno);
    }

    //fprintf(stderr, "%s--------------\n", ABOUT);

    GtkWidget *window = GTK_WIDGET (gtk_builder_get_object (builder,"appWindow"));
    getLabels();

    gtk_builder_connect_signals(builder,NULL);
    gtk_widget_show_all (window);
    selectableLabels(true);

    portname = "/dev/ttyUSB0";
    initStats();
    
    gtk_main();
    return 0;
}