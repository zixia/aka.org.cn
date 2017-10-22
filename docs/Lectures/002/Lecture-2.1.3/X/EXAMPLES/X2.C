/* compile using cc -I/usr/include/X11 -L/usr/lib/X11 -o demo demo.c -lX11 */
#include  <X11/cursorfont.h> 
#include <stdio.h> 
#include <X11/Xlib.h> 
  
 main (argc, argv) 
 char   *argv[]; 
 int     argc; 
 { 
      Display *display; 
      Window  win1; 
      XEvent event;
      XSetWindowAttributes attributes; 
      Cursor cursor_shape; 
      XFontStruct *fontinfo; 
      GC gr_context1, gr_context2; 
      XGCValues gr_values; 
      int     screen; 
      int     i; 
  
      display = XOpenDisplay(NULL); 
      screen  = XDefaultScreen(display);
  
      attributes.background_pixel      = XWhitePixel(display,screen); 
      attributes.border_pixel          = XBlackPixel(display,screen); 
  
      win1= XCreateWindow( display,XRootWindow(display,screen),0,200, 
                           200, /*XDisplayWidth(display,screen)-400, */
                           200, /*XDisplayHeight(display,screen)-400,*/
                           5, 6,
                           InputOutput, XDefaultVisual(display,screen),
                           CWBackPixel| CWBorderPixel, &attributes);

      XSelectInput(display,win1,ExposureMask | KeyPressMask) ;
  
      gr_values.function =   GXcopy; 
      gr_values.plane_mask = AllPlanes; 
      gr_values.foreground = BlackPixel(display,screen); 
      gr_values.background = WhitePixel(display,screen); 
      gr_context1=XCreateGC(display,win1, 
                  GCFunction | GCPlaneMask | GCForeground | GCBackground, 
                  &gr_values); 
  
      gr_values.function =   GXxor; 
      gr_values.foreground = WhitePixel(display,screen); 
      gr_values.background = BlackPixel(display,screen); 
      gr_context2=XCreateGC(display,win1, 
                  GCFunction | GCPlaneMask | GCForeground | GCBackground, 
                  &gr_values); 
  
      fontinfo = XLoadQueryFont(display,"6x10"); 
  
      cursor_shape=XCreateFontCursor(display,XC_heart); 
      XDefineCursor(display,win1,cursor_shape); 
  
      XSetFont(display,gr_context1,fontinfo->fid); 
      XSetFont(display,gr_context2,fontinfo->fid); 
  
      XMapWindow(display,win1); 

      while(1){
        XNextEvent(display,&event);

        switch(event.type){
        case Expose:
             XClearWindow(display,win1);
             XDrawString(display,win1,gr_context1,50,50,"Hello",5); 
             XDrawImageString(display,win1,gr_context2,20,20,"Hello",5); 
  
             XFillRectangle(display,win1,gr_context1,150,150,111,111); 
             XFillRectangle(display,win1,gr_context2,200,180,111,111); 
             break;
        case KeyPress: 
             XCloseDisplay(display); 
             exit(0);
        }
     }
}
