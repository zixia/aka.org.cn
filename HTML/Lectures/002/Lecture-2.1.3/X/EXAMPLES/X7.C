#include  <X11/cursorfont.h> 
#include <stdio.h> 
#include <X11/X.h>
#include <X11/Xlib.h> 
#include <X11/Xutil.h> 
#define TRUE 1 
#define FALSE 0 
#define PLANELIM  65536 
#define NUMPLANES     3             /* number of bit planes asked for */ 
#define NUMCOLS    3                /* number of colours asked for    */ 
#define PLNUM     (1 << NUMPLANES)  /* pixel value factor from planes */ 
#define PIXNUM    NUMCOLS * PLNUM   /* total pixel values obtained    */ 
 main (argc, argv) 
 char   *argv[]; 
 int     argc; 
 { 
     XColor   color_cell[PIXNUM];    /* must have room for pixnum */ 
     XSetWindowAttributes attributes; 
     int     Red = 60000; 
     int     Green = 40000; 
     int     Blue = 20000; 
     int     i,j; 
     int     mask; 
     Visual *visual; 
     int depth; 
     int which_color,which_cell,which_plane; 
     Display *display; 
     Visual *v; 
     Window  win; 
     GC gc; 
     Colormap cmap; 
     XEvent event; 
     int     stat; 
     long     plane_masks[NUMPLANES]; 
     long     pixar[NUMCOLS]; 
     char str0[50],str1[50],str2[50],str3[50], str4[50];
     int ret;

     setbuf (stdout, NULL); 
     setbuf (stderr, NULL); 

     display = XOpenDisplay(NULL); 
     visual = DefaultVisual(display,0); 
     depth  = DefaultDepth(display,0); 
     attributes.background_pixel      = XWhitePixel(display,0); 
     attributes.border_pixel          = XBlackPixel(display,0); 
     attributes.override_redirect     = 0; 
  
     win = XCreateWindow(display,XRootWindow(display,0),0,0,507,426,5, 
        depth,  InputOutput,visual ,CWBackPixel+CWBorderPixel+CWOverrideRedirect,
        &attributes); 
  
     XSelectInput(display,win,ExposureMask + ButtonPressMask+ KeyPressMask) ; 
  
  
     gc=XCreateGC(display,win,NULL,NULL); 
  
     XMapWindow(display,win); 
     cmap= XDefaultColormap(display,0); 
     stat = XAllocColorCells(display,cmap,FALSE,plane_masks, NUMPLANES,
            pixar, NUMCOLS); /* claim some colorcells */ 
     
     do{ 
       XNextEvent(display,&event); 
       switch(event.type){

       case Expose: 
              sprintf(str0,"depth is = %d", depth);
              sprintf(str1,"there are %d colorcells", DisplayCells(display,0));
              switch(XDoesBackingStore(display)){
                case WhenMapped : sprintf(str2,"when mapped");
                                break;
                case NotUseful :  sprintf(str2,"not useful");
                                break;
                case Always :     sprintf(str2,"always");
                                break;
                default :         sprintf (str2,"error in DoesBackingStore");
              }
              XDrawImageString(display,win,gc,20,15,str0,strlen(str0)); 
              XDrawImageString(display,win,gc,20,30,str1,strlen(str1)); 
              XDrawImageString(display,win,gc,20,45,str2,strlen(str2)); 

              sprintf(str2,"planes = %d, %d, %d", plane_masks[0],
                          plane_masks[1],plane_masks[2]); 
              sprintf(str3,"pixels = %d, %d, %d.", 
                          pixar[0], pixar[1], pixar[2]); 

              XDrawImageString(display,win,gc,20,60,str2,strlen(str2)); 
              XDrawImageString(display,win,gc,20,75,str3,strlen(str3)); 
              v=DefaultVisual(display,0); 
              switch(v->class){ 
 	        case StaticGray:sprintf(str0,"static gray"); break; 
 	        case GrayScale:sprintf(str0," gray scale"); break; 
                case StaticColor:sprintf(str0,"static color"); break; 
                case PseudoColor:sprintf(str0,"pseudocolor"); break; 
                case TrueColor:sprintf(str0,"true color"); break; 
                case DirectColor:sprintf(str0,"direct color"); break; 
              } 
              XDrawImageString(display,win,gc,250,20,str0,strlen(str0)); 
              XDrawImageString(display,win,gc,400,30,"pix vals used",13); 

  
              for(which_color = 0, which_cell = 0; which_color < NUMCOLS; 
                  which_color++){ 
                 color_cell[ which_cell++].pixel = pixar[which_color]; 
                 for(which_plane = 0; which_plane < PLNUM; which_plane++){
                         /* permute the bits to generate all the */ 
 	            mask = 0;      /* possible pixel values */ 
                    for(i = 1, j = 0; i < PLNUM; i <<= 1, j++) 
           	       if(i & which_plane) 
 		          mask |= plane_masks[j]; 
 	            color_cell[ which_cell++].pixel = pixar[which_color] |  mask; 
                    sprintf(str1," %d",(pixar[which_color]  | mask)); 
                    XDrawImageString(display,win,gc,430,30+12*which_cell,
                       str1,strlen(str1)); 
                 } 
              } 
              for(i = 0; i < PIXNUM; i++){
                 /* generate some arbitrary colours */ 
                 color_cell[i].flags= DoRed | DoGreen | DoBlue; 
                 color_cell[i].red = Red; 
                 color_cell[i].green = Green; 
                 color_cell[i].blue = Blue; 
                 Red = ((Red + 3000) % 65536); 
                 Green = ((Green + 3000) % 65536); 
                 Blue = ((Blue + 3000) % 65536); 
              } 
              XStoreColors(display,cmap, color_cell,PIXNUM); 

              for(i = 0; i < PIXNUM; i++){   
                 /* draw a multi-coloured stripe    */ 
                 XSetForeground(display,gc, color_cell[i].pixel); 
                 XFillRectangle(display,win,gc,10 * i, 100, 10, 200); 
              } 
              XFlush(display); 

              printf("now press button: \n"); 
              break;

       case ButtonPress:
              printf("closing display\n"); 
              XCloseDisplay(display); 
              exit(0); 
       }
   }while(1); 
}
