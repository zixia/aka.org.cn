/* This example deals with keyboard input. 
cc -I/usr/include/X11 -L/usr/lib/X11 -lX11 
*/  


#include  <X11/cursorfont.h> 
#include <stdio.h> 
#include <X11/Xlib.h> 
#include<X11/keysym.h>
 char workstation[] = {""}; 
 char str[80];
      Display *display; 
      Window  win1; 
      XSetWindowAttributes attributes; 
      XFontStruct *fontinfo; 
      GC gr_context1; 
      Visual *visual; 
      int screen;
      int depth; 

 main (argc, argv) 
 char   *argv[]; 
 int     argc; 
 { 
      XGCValues gr_values; 
      XEvent event; 
  
      setbuf (stdout, NULL); 
      setbuf (stderr, NULL); 
      display = XOpenDisplay(workstation); 
      screen = DefaultScreen(display);
      visual = DefaultVisual(display,screen); 
      depth  = DefaultDepth(display,screen); 
      attributes.background_pixel      = XWhitePixel(display,screen); 
      attributes.border_pixel          = XBlackPixel(display,screen); 
      attributes.override_redirect     = 0; 
  
      win1=  XCreateWindow(display, XRootWindow(display,screen),
             200,200, 300,200,5, depth, InputOutput, visual,
             CWBackPixel | CWBorderPixel | CWOverrideRedirect,&attributes);
  
      XSelectInput(display,win1,ExposureMask | ButtonPressMask | KeyPressMask);
  
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
      XFlush(display); 
  
      do{ 
          XNextEvent(display,&event); 
          switch(event.type){
          case ButtonPress:
            sprintf(str, "x=%d, y=%d", event.xbutton.x, event.xbutton.y);
            XDrawImageString(display,win1,gr_context1,event.xbutton.x, 
                event.xbutton.y, str, strlen(str));
            break;
          case KeyPress:
            deal_with_keys(&event);
            break;
          case Expose:
            XClearWindow(display,win1);
            XDrawImageString(display,win1,gr_context1,0,20,
                             "Press a key or Button. Use Break to exit", 40);
          }
      }while  (1); 
 } 
  
  
deal_with_keys(event)
XKeyEvent *event;
{
int count;
int buffer_size = 80;
char buffer[80];
KeySym keysym;
/* XComposeStatus compose; is not used, though it's in some books */
   count = XLookupString(event,buffer,buffer_size, &keysym);
   
   if ((keysym >= XK_space) && (keysym <= XK_asciitilde)){
      printf ("Ascii key:- ");
      if (event->state & ShiftMask)
             printf("(Shift) %s\n", buffer);
      else if (event->state & LockMask)
             printf("(Caps Lock) %s\n", buffer);
      else if (event->state & ControlMask)
             printf("(Control) %c\n", 'a'+ buffer[0]-1) ;
      else printf("%s\n", buffer) ;
   }
   else if ((keysym >= XK_Shift_L) && (keysym <= XK_Hyper_R)){
      printf ("modifier key:- ");
      switch (keysym){
      case XK_Shift_L: printf("Left Shift\n"); break;
      case XK_Shift_R: printf("Right Shift\n");break;
      case XK_Control_L: printf("Left Control\n");break;
      case XK_Control_R: printf("Right Control\n");	break;
      case XK_Caps_Lock: printf("Caps Lock\n");	break;
      case XK_Shift_Lock: printf("Shift Lock\n");break;
      case XK_Meta_L: printf("Left Meta\n");	break;
      case XK_Meta_R: printf("Right Meta\n");	break;

      }
    }
   else if ((keysym >= XK_Left) && (keysym <= XK_Down)){
      printf("Arrow Key:-");
      switch(keysym){
      case XK_Left: printf("Left\n");break;
      case XK_Up: printf("Up\n");break;
      case XK_Right: printf("Right\n");break;
      case XK_Down: printf("Down\n");break;	
      }
    }
   else if ((keysym >= XK_F1) && (keysym <= XK_F35)){
      printf ("function key %d pressed\n", keysym - XK_F1);

      if (buffer == NULL)
         printf("No matching string\n");
      else
         printf("matches <%s>\n",buffer);
   }

   else if ((keysym == XK_BackSpace) || (keysym == XK_Delete)){
      printf("Delete\n");
   }

   else if ((keysym >= XK_KP_0) && (keysym <= XK_KP_9)){
       printf("Number pad key %d\n", keysym -  XK_KP_0);
   }
   else if (keysym == XK_Break) {
        printf("closing display\n"); 
        XCloseDisplay(display); 
        exit (0);
   }else{
      printf("Not handled\n");
    }
}


/*
  if one wants to find out if another client has changed the key mappings,
  select MappingNotify and do
      XRefreshKeyboardMapping(Event *event);
  
 */
