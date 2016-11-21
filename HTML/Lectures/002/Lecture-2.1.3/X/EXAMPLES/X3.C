/* This uses pixmaps, some graphics routines, and events.*/   
#include  <X11/cursorfont.h> 
#include <stdio.h> 
#include <X11/Xlib.h> 
      Display *display; 
      Window  win1; 
      XSetWindowAttributes attributes; 
      XFontStruct *fontinfo; 
      GC gr_context1; 
      XArc arcs[10]; 
      Pixmap pixmap; 
      Visual *visual; 
      int screen;
      int depth; 
      int i; 
 main (argc, argv) 
 char   *argv[]; 
 int     argc; 
 { 
      XGCValues gr_values; 
      XEvent event; 
  
      setbuf (stdout, NULL); 
      setbuf (stderr, NULL); 
      display = XOpenDisplay(NULL); 
      screen = DefaultScreen(display);
      visual = DefaultVisual(display,screen); 
      depth  = DefaultDepth(display,screen); 
      attributes.background_pixel      = XWhitePixel(display,screen); 
      attributes.border_pixel          = XBlackPixel(display,screen); 
      attributes.override_redirect     = 0; 
  
      for(i=0;i<10;i++){ 
      	   arcs[i].x = 100;arcs[i].y = 50; 
           arcs[i].width = 100;arcs[i].height = 50; 
      } 
      for(i=0;i<5;i++){ 
           arcs[i].angle1 = 72*64*i; 
           arcs[i].angle2 = 35*64; 
      } 
      for(i=5;i<10;i++){ 
           arcs[i].angle1 = 72*64*i + 36*64; 
           arcs[i].angle2 = 35*64; 
      } 
  
      win1= XCreateWindow(display, XRootWindow(display,screen),
                   200,200, 300,200,5, depth, InputOutput, visual,
                   CWBackPixel | CWBorderPixel | CWOverrideRedirect,&attributes);
  
      XSelectInput(display,win1,ExposureMask | ButtonPressMask | KeyPressMask);
      pixmap = XCreatePixmap(display,win1,200,100,depth);   
      fontinfo = XLoadQueryFont(display,"6x10"); 
      gr_values.font =   fontinfo->fid; 
      gr_values.function =   GXcopy; 
      gr_values.plane_mask = AllPlanes; 
      gr_values.foreground = BlackPixel(display,screen); 
      gr_values.background = WhitePixel(display,screen); 
      gr_context1=XCreateGC(display,win1, 
              GCFont | GCFunction | GCPlaneMask | GCForeground | GCBackground, 
              &gr_values); 
  
      XDefineCursor(display,win1,XCreateFontCursor(display,XC_heart)); 
      XMapWindow(display,win1); 
  
      do{ 
        XNextEvent(display,&event); 
        if (event.type == Expose){ 
              draw_ellipse(); 
              XCopyArea(display,win1,pixmap,gr_context1,50,25,200,100,0,0); 
              XSetFunction(display,gr_context1,GXinvert); 
              XDrawImageString(display,pixmap,gr_context1,80,45,"pixmap",6); 
              XDrawImageString(display,pixmap,gr_context1,90,60,"copy",4); 
              XSetFunction(display,gr_context1,GXcopy); 
              XDrawString(display,win1,gr_context1,10,20,
                  "Press a key in this window",26); 
        } 
      }while  (event.type !=KeyPress); 
  
      XCopyArea(display,pixmap,win1,gr_context1,0,0,200,100,100,125); 
      XDrawString(display,win1,gr_context1,10,32,
                  "Now press a key to exit",23); 
      XFlush(display); 
  
      do{ 
        XNextEvent(display,&event); 
      }while  (event.type !=KeyPress); 
  
      printf("closing display\n"); 
      XCloseDisplay(display); 
 } 
  
 draw_ellipse() 
 { 
      XSetArcMode(display,gr_context1,ArcPieSlice); 
      XFillArcs(display,win1,gr_context1,arcs,5); 
      XSetArcMode(display,gr_context1,ArcChord); 
      XFillArcs(display,win1,gr_context1,arcs+5,5); 
 }
