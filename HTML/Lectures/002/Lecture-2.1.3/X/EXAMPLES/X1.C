/* compile using cc -I/usr/include/X11 -L/usr/lib/X11 -o demo demo.c -lX11 */
#include <stdio.h>
#include <X11/Xlib.h>
 /* This program draws a red line and some text in a chosen font.
  * 
  */
     Display *display;
     Window  window;
     XSetWindowAttributes attributes;
     XGCValues gr_values;
     XFontStruct *fontinfo;
     GC gr_context;
     Visual *visual;
     int depth;
     int screen;
     XEvent event;
     XColor    color, dummy;
 
main (argc, argv)
char   *argv[];
int     argc;
{
     display = XOpenDisplay(NULL);
     screen = DefaultScreen(display);
     visual = DefaultVisual(display,screen);
     depth  = DefaultDepth(display,screen);
     attributes.background_pixel = XWhitePixel(display,screen);
 
     window = XCreateWindow( display,XRootWindow(display,screen),
                            200, 200, 350, 200, 5, depth,  InputOutput,
                            visual ,CWBackPixel, &attributes);
     XSelectInput(display,window,ExposureMask | KeyPressMask) ;
     fontinfo = XLoadQueryFont(display,"6x10");
     
     XAllocNamedColor(display, DefaultColormap(display, screen),"red",
                      &color,&dummy);
 
     gr_values.font = fontinfo->fid;
     gr_values.foreground = color.pixel;
     gr_context=XCreateGC(display,window,GCFont+GCForeground, &gr_values);
     XFlush(display);
     XMapWindow(display,window);
     XFlush(display);

     while(1){
        XNextEvent(display,&event);

        switch(event.type){
        case Expose:
             XDrawLine(display,window,gr_context,0,0, 100, 100);
             XDrawString(display,window,gr_context,100,100,"hello",5);
             break;
        case KeyPress: 
             XCloseDisplay(display); 
             exit(0);

        }
     }
}
