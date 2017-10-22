#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
 /* This program draws a red line and some text in a chosen font.
  * button1 growbox. button2 dump pixmap
  */
#define PIXMAP_W 200
#define PIXMAP_H 200

static char *event_names[] = {
"",
"",
"KeyPress",
"KeyRelease",
"ButtonPress",
"ButtonRelease",
"MotionNotify",
"EnterNotify",
"LeaveNotify",
"FocusIn",
"FocusOut",
"KeymapNotify",
"Expose",
"GraphicsExpose",
"NoExpose",
"VisibilityNotify",
"CreateNotify",
"DestroyNotify",
"UnmapNotify",
"MapNotify",
"MapRequest",
"ReparentNotify",
"ConfigureNotify",
"ConfigureRequest",
"GravityNotify",
"ResizeRequest",
"CirculateNotify",
"CirculateRequest",
"PropertyNotify",
"SelectionClear",
"SelectionRequest",
"SelectionNotify",
"ColormapNotify",
"ClientMessage",
"MappingNotify" };

     Display *display;
     Window  window;
     XSetWindowAttributes attributes;
     XGCValues gr_values;
     XFontStruct *fontinfo;
     GC gr_context;
     Visual *visual;
     Pixmap pix;
     Atom type_asked_for = -1;
     Atom type_offered;
     int depth;
     int screen;
     XEvent event;
     XColor    color, dummy;
     Atom target_property;
     Bool only_if_exists;
     unsigned char* data;
     int  ret_bytes;
     unsigned long ret_remaining, ret_length; 
     XEvent ev;
     XSelectionEvent *ev_ptr;
     Atom ret_atom;
     int ret_format;
     long datalength;
     int ret;
     int x_orig, y_orig, x, y, x_final, y_final; 
     int x_paste, y_paste;
     char str[1024];

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
     XSelectInput(display,window,
                  ExposureMask | KeyPressMask | ButtonPressMask | 
                  ButtonMotionMask | ButtonReleaseMask ) ;
     fontinfo = XLoadQueryFont(display,"6x10");
     
     XAllocNamedColor(display, DefaultColormap(display, screen),"red",
                      &color,&dummy);
     setbuf(stdout,NULL);
     setbuf(stderr,NULL);

 
     gr_values.font = fontinfo->fid;
     gr_values.foreground = color.pixel;
     gr_context=XCreateGC(display,window,GCFont+GCForeground, &gr_values);
     XFlush(display);
     pix=XCreatePixmap(display, window, PIXMAP_W, PIXMAP_H, depth);     

     /* create Atom type to contain graphics info */
     target_property = XInternAtom(display, "COORDS", only_if_exists = False);

     ev_ptr= (XSelectionEvent*) &ev;

     XMapWindow(display,window);

     do{
      XNextEvent(display, &event);
      printf("In main loop, got a %s event\n", event_names[event.type]);
      switch(event.type){
      case ButtonPress:
         /* if middle button, paste */   /*REQUESTOR*/
         if (event.xbutton.button == Button2){
           XStoreName(display, window,"Requestor");
           /*create Atom to contain graphics (for sender)*/
           x_paste = event.xbutton.x;
           y_paste = event.xbutton.y;
           target_property = XInternAtom(display, "COORDS", 
                           only_if_exists = False);


           if (target_property == None){
             printf("sender hasn't created Atom\n");     
             break;
           }
    
           /* tell the server to send an event to the present owner
            * of the selection, asking the owner to convert the data in 
            * the selection to the required type
            */

           type_asked_for = XA_PIXMAP;
           printf("asking for Pixmap\n");
           XConvertSelection(display, XA_PRIMARY, type_asked_for, target_property, 
                      window, event.xbutton.time);


         } /* end of button2 */

         /* if left button, cut */              /*OWNER*/
         if (event.xbutton.button == Button1){
            XSetFunction(display, gr_context, GXxor);
            if (XGetSelectionOwner(display, target_property) == window)
               draw_box(x_orig,y_orig,x_final,y_final);
            x_orig=x=event.xbutton.x;
            y_orig=y=event.xbutton.y;

            draw_box(x_orig,y_orig,x,y);
            do{
              XNextEvent(display, &event);
              if(event.type == MotionNotify){
                draw_box(x_orig,y_orig,x,y);
                draw_box(x_orig,y_orig,event.xmotion.x,event.xmotion.y);
                x=event.xmotion.x;
                y=event.xmotion.y;
              }
            }while(event.type != ButtonRelease);
            x_final= x;
            y_final= y;
            XSetFunction(display, gr_context, GXcopy);

            XSetSelectionOwner(display, target_property, window, CurrentTime);
            printf("claiming selection\n");
            if (XGetSelectionOwner(display, target_property) != window){
                printf("failed to own selection\n");
	    }
            else
                XStoreName(display, window,"Owner");

         }
       break;

      case SelectionRequest: /*OWNER*/
           /* the owner of the selection receives a request to put info
            * into selection.
            */

           if ((mod(x_orig, x_final)>PIXMAP_W)||
                  (mod(y_orig, y_final)>PIXMAP_H)){
              type_offered = XA_STRING;
              printf("offering a string\n");
           }
           else{            
              type_offered = XA_PIXMAP;
              printf("offering a pixmap\n");
           }
           if (event.xselectionrequest.target == type_offered){
               printf("requestor wants what's on offer\n");
               if (type_offered == XA_STRING){
                  sprintf(str,"%d %d %d %d",x_orig,y_orig,x_final,y_final);
                  printf("str in %s\n", str);

                  ret = XChangeProperty(display,window, target_property, 
			XA_STRING,8, PropModeReplace, (unsigned char*)str, (int) strlen(str)+1);
               
                  /* check to see if changed ok */
                  ret=XGetWindowProperty(display, window,target_property,
                      0L, strlen(str)+1  ,False, AnyPropertyType, &ret_atom, 
                      &ret_format, &ret_length, &ret_remaining, &data);
                  if (ret != Success)
                      fprintf(stderr, "XGetWindowProperty failed in selectionrequest\n");
                  else
                      fprintf(stderr, "data after being set in selectionrequestis %s\n", data);

               }else if (type_offered == XA_PIXMAP){
                  ret= XCopyArea(display,window,pix,gr_context,x_orig, y_orig,
                     mod(x_orig,x_final), mod(y_orig, y_final), 0, 0);

                  switch(ret){
                  case BadDrawable:
                    break;
                  case BadGC:
                    break;
                  case BadMatch:
                    break;
                  }
                  ret=XChangeProperty(display,window,target_property, XA_PIXMAP,8, PropModeReplace, (unsigned char*)pix, (int) sizeof((Pixmap)pix));

                  ret=XGetWindowProperty(display, window,target_property,
                       0L, (int) sizeof(pix)  ,False, AnyPropertyType, 
                       &ret_atom, &ret_format, &ret_length, &ret_remaining, 
                       &data);
                  if (ret != Success)
                       fprintf(stderr, "XGetWindowProperty failed in selectionrequest\n");
                  else
                       fprintf(stderr, "data after being set in selectionrequestis %s\n", data);
                }
           }else{
                  printf("can't provide the type on offer\n");
                  type_offered = None;
           }

           /* tell the requestor if the data is ready to read and is in 
            * the requested form.
            */

           ev_ptr->display   = event.xselectionrequest.display;
           if (type_offered == None)
              ev_ptr->property =None;
           else
              ev_ptr->property  = event.xselectionrequest.property;
           ev_ptr->selection = event.xselectionrequest.selection;
           ev_ptr->target    = type_offered;
           ev_ptr->type      = SelectionNotify;       
           ev_ptr->requestor = event.xselectionrequest.requestor;
           ev_ptr->time      = CurrentTime; /*event->xselectionrequest.time*/
           ev_ptr->send_event = True;

           printf("Sending the event off\n");
           ret=XSendEvent(display, event.xselectionrequest.requestor,
                      False,0L, (XEvent*) ev_ptr);

           if ((ret==BadValue) || (ret==BadWindow))
                printf("SendEvent failed\n");    
           break;

      case SelectionClear: /*OWNER*/
           /* something else is the owner now.
            */
           /* if still transferring, wait */
           XSetFunction(display, gr_context, GXxor);
           draw_box(x_orig,y_orig,x_final,y_final);
           XSetFunction(display, gr_context, GXcopy);
           XStoreName(display, window,"Ex owner");
           break;

      case Expose:
             XDrawLine(display,window,gr_context,0,0, 100, 100);
             XDrawString(display,window,gr_context,100,100,"hello",5);
             break;
      case KeyPress: 
             XCloseDisplay(display); 
             exit(0);

       case SelectionNotify: /*REQUESTOR*/
           /*the owner of the selection has sent the requestor an
            * event
            */
           if (event.xselection.property == target_property )
              printf("property ok in selectionnotify\n");
           else if(event.xselection.property == None){
              printf("owner couldn't provide requested type\n");   
              /* so ask for another type */
              if(type_asked_for == XA_PIXMAP){
                   printf("couldn't get pixmap so ask for string\n");
                   type_asked_for = XA_STRING;
                   XConvertSelection(display, XA_PRIMARY, type_asked_for, 
                      target_property, window, event.xbutton.time);
                   break;
              }
              else{
                   printf("can't provide anything\n");
		   exit(1);
              }
          }


           if (type_asked_for == XA_STRING)
               datalength = strlen(str)+1;
           else
               datalength = sizeof(pix);

           ret=XGetWindowProperty(display, window,event.xselection.property,
               0L,datalength,False,
           AnyPropertyType, &ret_atom, &ret_format, 
               &ret_length, &ret_remaining, &data);

           if (ret != Success)
              printf("XGetWindowProperty failed in selectionnotify\n");

           if (type_asked_for == XA_STRING)
               XDrawString(display, window, gr_context, 
                     x_paste, y_paste, (char *)data, strlen(data));
           else{
              ret= XCopyArea(display, *data ,window,gr_context,0,0,
                    mod(x_orig,x_final), mod(y_orig, y_final),
                    x_paste, y_paste);

              switch(ret){
              case BadDrawable:
                break;
              case BadGC:
                break;
              case BadMatch:
                break;
              }

              /* XFree(data); */

         /* the property in xselection should be deleted using
          * XGetWindowproperty with delete set to true so that 
          * the owner knows when data has been transferred.
          */
            }
       } /* end of switch */
   }while(1);     
}


draw_box(x1,y1,x2,y2)
int x1,y1,x2,y2;
{
XDrawLine(display,window,gr_context,x1,y1,x2,y1);
XDrawLine(display,window,gr_context,x2,y1,x2,y2);
XDrawLine(display,window,gr_context,x2,y2,x1,y2);
XDrawLine(display,window,gr_context,x1,y2,x1,y1);
}

int mod (a,b)
int a,b;
{
  if (a >=b)
     return a-b;
  else
     return b-a;
}



test_ret_of_XChange_Property(ret)
int ret;
{
         switch(ret){
         case BadAtom: printf("Change Prop BadAtom\n"); break;
         case BadMatch: printf("Change Prop BadMatch\n"); break;
         case BadValue: printf("Change Prop BadValue\n"); break;
         case BadAlloc: printf("Change Prop BadAlloc\n"); break;
         case BadWindow: printf("Change Prop BadWindow\n"); break;
         default: printf("Change Prop ok\n");
         }

         /* should look at propertynotify of requesting window to know when
          * transfer successful
          */

}
