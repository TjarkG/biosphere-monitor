//Writen by TjarkG and published under the MIT License

#include <stdio.h>
#include <stdbool.h>
#include <gtk/gtk.h>
#include <errno.h>
#include <unistd.h>
#include "biosphere.h"

#define ABOUT "Biosphere Monitor by TjarkG\ngithub.com/tjarkG\nGTK 3\n"

GtkBuilder *builder;
guint timerId;
GtkWidget *infoWindow;
GtkWidget *deleteWindow;
unsigned int intervall;

gboolean windowDelete(__attribute__((unused)) GtkWidget *widget, __attribute__((unused)) GdkEvent  *event, __attribute__((unused)) gpointer   data)
{
	gtk_main_quit();
	return TRUE;   
}

void aboutOpen(__attribute__((unused)) GtkWidget *widget, __attribute__((unused)) gpointer   data)
{
    GtkWidget *text = GTK_WIDGET (gtk_builder_get_object (builder,"infoText"));
    infoWindow = GTK_WIDGET (gtk_builder_get_object (builder,"infoWindow"));
    
    gtk_label_set_label(GTK_LABEL(text), ABOUT);
    gtk_widget_show_all(infoWindow);
    gtk_label_set_selectable(GTK_LABEL(text), TRUE);
}

void closeWindow(__attribute__((unused)) GtkWidget *widget, __attribute__((unused)) gpointer   data)
{
    gtk_widget_hide_on_delete(infoWindow);
}

void syncTime(__attribute__((unused)) GtkWidget *widget, __attribute__((unused)) gpointer   data)
{
    bool synced = synctime();

    GtkWidget *text = GTK_WIDGET (gtk_builder_get_object (builder,"infoText"));
    infoWindow = GTK_WIDGET (gtk_builder_get_object (builder,"infoWindow"));

    gtk_label_set_label(GTK_LABEL(text), synced? "Fehler: Übertragung fehlgeschlagen":"Zeit erfolgreich übertragen");
    gtk_widget_show_all(infoWindow);
}

//Delete local Data
void deleteOpen(__attribute__((unused)) GtkWidget *widget, __attribute__((unused)) gpointer   data)
{
    deleteWindow = GTK_WIDGET (gtk_builder_get_object (builder,"deleteWindow"));

    gtk_widget_show_all(deleteWindow);
}

void delete(__attribute__((unused)) GtkWidget *widget, __attribute__((unused)) gpointer   data)
{
    if(timerId != 0)
        g_source_remove(timerId);
    usleep(50000);
    //setCommand("DEL");
    gtk_widget_hide_on_delete(deleteWindow);

    //Show Confirmation/Error
    GtkWidget *text = GTK_WIDGET (gtk_builder_get_object (builder,"infoText"));
    infoWindow = GTK_WIDGET (gtk_builder_get_object (builder,"infoWindow"));

    gtk_label_set_label(GTK_LABEL(text), "TODO: delete");
    gtk_widget_show_all(infoWindow);
    initStats();
}

void deleteAbort(__attribute__((unused)) GtkWidget *widget, __attribute__((unused)) gpointer   data)
{
    gtk_widget_hide_on_delete(deleteWindow);
}

//Intervall Setter
GtkWidget *intervallWindow;

void intervallOpen(__attribute__((unused)) GtkWidget *widget, __attribute__((unused)) gpointer   data)
{
    intervallWindow = GTK_WIDGET(gtk_builder_get_object (builder,"intervallWindow"));

    gtk_widget_show_all(intervallWindow);
}

void intervallAbort(__attribute__((unused)) GtkWidget *widget, __attribute__((unused)) gpointer   data)
{
    gtk_widget_hide_on_delete(intervallWindow);
}

void intervallTransfer(__attribute__((unused)) GtkWidget *widget, __attribute__((unused)) gpointer   data)
{
    if(timerId != 0)
        g_source_remove(timerId);
    GtkWidget *value = GTK_WIDGET(gtk_builder_get_object (builder,"intervallValue"));
    const char* buf = gtk_entry_get_text(GTK_ENTRY(value));
    intervall = atoi(buf);

    GtkWidget *unit = GTK_WIDGET(gtk_builder_get_object (builder,"intervallUnit"));
    buf = gtk_combo_box_get_active_id(GTK_COMBO_BOX(unit));
    intervall *= atoi(buf);

    gtk_widget_hide_on_delete(intervallWindow);

    usleep(50000);
    bool sucess = setIntervall(intervall);
    usleep(50000);

    //Show Confirmation/Error
    GtkWidget *text = GTK_WIDGET (gtk_builder_get_object (builder,"infoText"));
    infoWindow = GTK_WIDGET (gtk_builder_get_object (builder,"infoWindow"));

    gtk_label_set_label(GTK_LABEL(text), sucess? "Intervall erfolgreich geändert":"Fehler: Intervall konnte nicht gesetzt werden");
    gtk_widget_show_all(infoWindow);
    initStats();
}

//selftest
GtkWidget *selftestWindow;

void selftest(__attribute__((unused)) GtkWidget *widget, __attribute__((unused)) gpointer   data)
{
    unsigned int results = getCommand("DR");
    selftestWindow = GTK_WIDGET(gtk_builder_get_object (builder,"selftestWindow"));

    for (int i = 0; i < 15; i++)
    {
        char buf[8];
        sprintf(buf, "out%d", i+1);

        GtkWidget *text = GTK_WIDGET (gtk_builder_get_object (builder,buf));
        gtk_label_set_label(GTK_LABEL(text), results & (1UL << i)? "Fehler":"Ok");
    }

    GtkWidget *text = GTK_WIDGET (gtk_builder_get_object (builder,"selftestOut"));
    gtk_label_set_label(GTK_LABEL(text), results? "Es wurden Fehler gefunden":"Selbsttest bestanden");
    
    gtk_widget_show_all(selftestWindow);
}

void selftestAbort(__attribute__((unused)) GtkWidget *widget, __attribute__((unused)) gpointer   data)
{
    gtk_widget_hide_on_delete(selftestWindow);
}

void selftestRetry(__attribute__((unused)) GtkWidget *widget, __attribute__((unused)) gpointer   data)
{
    selftestAbort(NULL, NULL);
    selftest(NULL, NULL);
}