//Writen by TjarkG and published under the MIT License

#include <stdio.h>
#include <stdbool.h>
#include <gtk/gtk.h>
#include <errno.h>
#include "biosphere.h"
#include "tty.h"
#include "../reading.h"

#define memb(a) (sizeof(a)/sizeof(a[0]))

GtkBuilder *builder;
const char *labelNames[] = {"outTime","outLight","outITemp","outOTemp","outPress","outAir","outSoil","outBegin","outInv","outLast","outSum","outStorage"};
char labelContent[memb(labelNames)][16];
GtkWidget *labels[memb(labelNames)];
guint timerId = 0;

void initStats(void);

#include "signals.h"

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

    sprintf(buf,"%d lux", in.light);
    gtk_label_set_label(GTK_LABEL(labels[1]), buf);
    sprintf(buf,"%2.1f°C", (in.temperaturOut/5.0));
    gtk_label_set_label(GTK_LABEL(labels[2]), buf);
    sprintf(buf,"%2.1f°C", (in.temperaturIn/10.0));
    gtk_label_set_label(GTK_LABEL(labels[3]), buf);
    if(in.pressure != 0)
    {
    sprintf(buf,"%dhPA", in.pressure);
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
    char buf[64];
    //reset Labels
    for (unsigned int i = 0; i < memb(labelNames); i++)
        gtk_label_set_label(GTK_LABEL(labels[i]), "n/A");

    //get Intervall
    int intervall = getCommand("IG");
    if(intervall % 3600 == 0)
        sprintf(buf,"%d h", intervall/3600);
    else if(intervall % 60 == 0)
        sprintf(buf,"%d min", intervall/60);
    else
        sprintf(buf,"%ds", intervall);
    gtk_label_set_label(GTK_LABEL(labels[8]), buf);


    timerId = g_timeout_add(2000,timerTick,NULL);
    timerTick(NULL);
}

int main(int argc,char **argv) 
{
    if (argc == 1 || argv[1][0] == '-') /* no args or arguments cant be a serial Port: throw error */
    {
        fprintf(stderr, "%s: first argument must be target COM Port\n", argv[0]);
        return -1;
    }
    argc--;
    startUART(argv[1]);


    gtk_init (&argc , &argv);  
    builder = gtk_builder_new(); 
    if(errno = gtk_builder_add_from_file(builder,"PC/bioGui.glade" , NULL) == 0)
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
    initStats();
    gtk_main();
    return 0;
}