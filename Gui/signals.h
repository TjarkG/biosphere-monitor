//Writen by TjarkG and published under the MIT License

#include <stdio.h>
#include <stdbool.h>
#include <gtk/gtk.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include "../PC/biosphere.h"

#define ABOUT "Biosphere Monitor by TjarkG\ngithub.com/tjarkG\nGTK 3.24\n"

GtkBuilder *builder;
guint timerId;
GtkWidget *infoWindow;
GtkWidget *deleteWindow;
int intervall;
bool isConnected;

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
    bool synced;
    if(isConnected)
        synced = synctime();
    else
        synced = true;

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
    if(isConnected)
        setCommand("DEL");
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
    bool success = false;
    if(isConnected)
        success = setIntervall(intervall);
    usleep(50000);

    //Show Confirmation/Error
    GtkWidget *text = GTK_WIDGET (gtk_builder_get_object (builder,"infoText"));
    infoWindow = GTK_WIDGET (gtk_builder_get_object (builder,"infoWindow"));

    gtk_label_set_label(GTK_LABEL(text), success? "Intervall erfolgreich geändert":"Fehler: Intervall konnte nicht gesetzt werden");
    gtk_widget_show_all(infoWindow);
    initStats();
}

//selftest
GtkWidget *selftestWindow;

void selftest(__attribute__((unused)) GtkWidget *widget, __attribute__((unused)) gpointer   data)
{
    unsigned int results;
    if(isConnected)
        results = getCommand("DR");
    else
        results = ~0U;
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

//select source
GtkWidget *sourceWindow;
GtkWidget *portSelect;
void sourceOpen(__attribute__((unused)) GtkWidget *widget, __attribute__((unused)) gpointer   data)
{
    sourceWindow = GTK_WIDGET(gtk_builder_get_object (builder,"sourceWindow"));
    portSelect = GTK_WIDGET(gtk_builder_get_object (builder,"portSelect"));
    gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(portSelect));

    //get possible Ports (tyyUSB and ttyBio)
    char *ports[32];
    unsigned int portn = 0;
    DIR *dp;
    struct dirent *ep;     
    dp = opendir ("/dev");

    if (dp != NULL)
    {
        while ((ep = readdir(dp)))
        {
            if(strncmp("ttyUSB",ep->d_name, 6) == 0 || strncmp("ttyBio",ep->d_name, 6) == 0)
            {
                ports[portn++] = ep->d_name;
            }
        }

        closedir(dp);
    }
    else
        fprintf(stderr, "Error: Couldn't open directory /dev");

    //put these Ports into the ComboBox
    for (unsigned int i = 0; i < portn; i++)
    {
        char buf[255];
        sprintf(buf, "/dev/%s", ports[i]);
        gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(portSelect), "", buf);
    }

    gtk_widget_show_all(sourceWindow);
}

void sourceAbort(__attribute__((unused)) GtkWidget *widget, __attribute__((unused)) gpointer   data)
{
    gtk_widget_hide_on_delete(sourceWindow);
}

void sourceSelect(__attribute__((unused)) GtkWidget *widget, __attribute__((unused)) gpointer   data)
{
    portname = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(portSelect));
    initStats();

    gtk_widget_hide_on_delete(sourceWindow);
}

//save Messurments as a file
GtkWidget *saveWindow;

void saveAsOpen(__attribute__((unused)) GtkWidget *widget, __attribute__((unused)) gpointer   data)
{
    saveWindow = GTK_WIDGET(gtk_builder_get_object (builder,"saveDialog"));

    gtk_widget_show_all(saveWindow);
}

void saveAbbort(__attribute__((unused)) GtkWidget *widget, __attribute__((unused)) gpointer   data)
{
    gtk_widget_hide_on_delete(saveWindow);
}

void saveAs(__attribute__((unused)) GtkWidget *widget, __attribute__((unused)) gpointer   data)
{
    GtkComboBox *typeChooser = GTK_COMBO_BOX(gtk_builder_get_object (builder,"filetypeChooser"));

    const gchar *fileDir = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(saveWindow));
    const gchar *fileName = gtk_file_chooser_get_current_name(GTK_FILE_CHOOSER(saveWindow));
    const gchar *fileType = gtk_combo_box_get_active_id(typeChooser);

    char filePath[255];
    sprintf(filePath, "%s/%s.%s", fileDir, fileName, fileType);
    gtk_widget_hide_on_delete(saveWindow);

    FILE *opf;
    char message[512];
    if ((opf = fopen(filePath, "w")) == NULL) 
    {
        GtkWidget *text = GTK_WIDGET (gtk_builder_get_object (builder,"infoText"));
        infoWindow = GTK_WIDGET (gtk_builder_get_object (builder,"infoWindow"));

        sprintf(message, "Fehler: Datei %s kann nicht geöffnet werden\n", filePath);
        gtk_label_set_label(GTK_LABEL(text), message);
        gtk_widget_show_all(infoWindow);
        return;
    }
    else
    {
        storeReadings(opf,false);
        fclose(opf);
    }
}

//save Messurments as a csv in the default folder
void saveOpen(__attribute__((unused)) GtkWidget *widget, __attribute__((unused)) gpointer   data)
{
    //get username for Path
    struct passwd *p = getpwuid(getuid());

    //get date for Filname
    time_t rawtime;
    time ( &rawtime );
    char tmStr[20];
    struct tm lt;
    lt = *gmtime(&rawtime);
    strftime(tmStr, sizeof(tmStr), "%d-%m-%Y", &lt);

    //open file and save to it
    FILE *opf;
    char filePath[255];
    char fileDir[255];
    sprintf(filePath, "/home/%s/BioData/biosphere-%s-%s.csv", p->pw_name, p->pw_name, tmStr);
    sprintf(fileDir, "/home/%s/BioData", p->pw_name);

    struct stat st = {0};
    if (stat(fileDir, &st) == -1)
        mkdir(fileDir, 0700);

    char message[512];
    if ((opf = fopen(filePath, "w")) == NULL) 
        sprintf(message, "Fehler: Datei %s kann nicht geöffnet werden\n", filePath);
    else
    {
        sprintf(message, "Messdaten als\n%s\ngespeichert\n", filePath);
        storeReadings(opf,false);
        fclose(opf);
    }

    GtkWidget *text = GTK_WIDGET (gtk_builder_get_object (builder,"infoText"));
    infoWindow = GTK_WIDGET (gtk_builder_get_object (builder,"infoWindow"));

    gtk_label_set_label(GTK_LABEL(text), message);
    gtk_widget_show_all(infoWindow);
}