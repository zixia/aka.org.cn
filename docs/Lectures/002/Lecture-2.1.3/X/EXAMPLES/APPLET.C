一般的非applet程序


 #include 
 int main(int argc, char **argv)
 {
    /* ... we build the user interface ... */
    gtk_init(&argc, &argv); /* #1 */
    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL); /* #2 */
    gtk_window_set_title(GTK_WINDOW(window), PACKAGE);
    /* controls is the name of the container all our widgets are in */
    gtk_container_add(GTK_CONTAINER(window), controls); /* #3 */
    gtk_widget_show(window);

    /* Everything's ready to begin our main loop */
    gtk_main(); /* #4 */
    return 0;
 }

将其改造为一个applet


 #include 
 int main(int argc, char **argv)
 {
    applet_widget_init(PACKAGE, VERSION, argc, argv,
                       NULL, 0, NULL); /* #1 */
    GtkWidget* applet = applet_widget_new(PACKAGE); /* #2 */
    applet_widget_add(APPLET_WIDGET(applet), controls); /* #3 */
    gtk_widget_show(applet);
    applet_widget_gtk_main(); /* #4 */
    return 0;
 }

