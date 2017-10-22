#include <stdio.h> 
#include <X11/X.h>
#include <X11/Xlib.h> 
#define NAME "red"  
 main (argc, argv) 
 char   *argv[]; 
 int     argc; 
 { 
     XColor   color_cell, truecolor;
     XSetWindowAttributes attributes; 
     Visual *visual; 
     int depth; 
     int which_color,which_cell,which_plane; 
     Display *display; 
     Window  win; 
     GC gc; 
     Colormap cmap; 
     XEvent event; 
     int screen;
     setbuf (stdout, NULL); 
     setbuf (stderr, NULL); 

     display = XOpenDisplay(NULL); 
     screen = DefaultScreen(display);
     visual = DefaultVisual(display,screen); 
     depth  = DefaultDepth(display,screen); 
     cmap= XDefaultColormap(display,screen); 

     attributes.background_pixel      = XWhitePixel(display,screen); 
     attributes.border_pixel          = XBlackPixel(display,screen); 
     attributes.override_redirect     = 1; 
  
     win = XCreateWindow(display,XRootWindow(display,screen),0,0,507,426,5,
           depth, InputOutput,visual,
           CWBackPixel | CWBorderPixel | CWOverrideRedirect,&attributes);
  
     XSelectInput(display,win,ButtonPressMask+ KeyPressMask) ; 
  
  
     gc=XCreateGC(display,win,NULL,NULL); 
  
     XMapWindow(display,win); 
  
       /* first by RGB values */
       color_cell.flags= DoRed | DoGreen | DoBlue; 
       color_cell.red = 10000; 
       color_cell.green = 40000; 
       color_cell.blue = 60000;        
       if (XAllocColor(display,cmap,&color_cell)==0)
           fprintf(stderr, "XAllocColor failed\n");
       XSetForeground(display,gc,color_cell.pixel);
  
       XDrawImageString(display,win,gc,250,50,"RGB color",9); 
       XFlush(display);
  
       /* now by name */
  
       if (XAllocNamedColor(display,cmap,
                             NAME, &color_cell, &truecolor) == 0) {
            fprintf(stderr, "Color '%s' unknown\n", NAME);
       }
       if (truecolor.red != color_cell.red ||
                truecolor.green != color_cell.green ||
                truecolor.blue != color_cell.blue) {
            fprintf(stderr, "Warning: %s color may be wrong\n", NAME);
       }
       XSetForeground(display,gc,color_cell.pixel);
       XDrawImageString(display,win,gc,250,80, "named color",11); 
       /*XFlush(display);*/ 
  
       printf("now press button: \n"); 
       do{ 
         XNextEvent(display,&event); 
       }while(event.type != ButtonPress); 
  
       printf("closing display\n"); 
       XCloseDisplay(display); 
       exit(0); 
 }
