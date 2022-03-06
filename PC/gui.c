//Writen by TjarkG and published under the MIT License

#include <stdio.h>
#include <stdbool.h>
#include <gtk/gtk.h>
#include <errno.h>
#include "biosphere.h"
#include "tty.h"

#define memb(a) (sizeof(a)/sizeof(a[0]))

GtkBuilder *builder;
const char *labelNames[] = {"outTime","outLight","outITemp","outOTemp","outPress","outAir","outSoil","outBegin","outInv","outLast","outSum","outStorage"};
GtkWidget *labels[memb(labelNames)];

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

void setLabels(const char **in)
{
    for (unsigned int i = 0; i < memb(labelNames); i++)
        gtk_label_set_label(GTK_LABEL(labels[i]), in[i]);
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
    gtk_main();
    return 0;
}