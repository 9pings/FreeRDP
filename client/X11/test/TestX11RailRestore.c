#include "../xfreerdp.h"
#include "../xf_rail.h"
#include "../xf_window.h"
#include <X11/Xutil.h>
#include <stdio.h>
int main(void)
{
 xfContext x = {0}; xfAppWindow w = {0}; int failed = 0;
 x.display = XOpenDisplay(NULL); if (!x.display) return 77;
 x.screen = DefaultScreenOfDisplay(x.display); x.screen_number = DefaultScreen(x.display);
 x.log = WLog_Get("test.rail.restore");
 x.common.context.settings = freerdp_settings_new(0);
 x.NET_WM_STATE = XInternAtom(x.display,"_NET_WM_STATE",False);
 x.NET_WM_STATE_MAXIMIZED_VERT = XInternAtom(x.display,"_NET_WM_STATE_MAXIMIZED_VERT",False);
 x.NET_WM_STATE_MAXIMIZED_HORZ = XInternAtom(x.display,"_NET_WM_STATE_MAXIMIZED_HORZ",False);
 w.handle=XCreateSimpleWindow(x.display,RootWindowOfScreen(x.screen),0,0,200,100,0,0,0);
 XSelectInput(x.display,RootWindowOfScreen(x.screen),SubstructureNotifyMask);
 w.dwStyle=WS_CAPTION;
 const Atom active=XInternAtom(x.display,"_NET_ACTIVE_WINDOW",False);
 const BYTE states[]={WINDOW_SHOW,WINDOW_SHOW_MAXIMIZED};
 for (size_t i=0;i<2;i++) {
  XUnmapWindow(x.display,w.handle); XSync(x.display,False);
  w.rail_state=WINDOW_SHOW_MINIMIZED; w.minimized=TRUE; w.maxVert=TRUE; w.maxHorz=TRUE;
  xf_ShowWindow(&x,&w,states[i]); XSync(x.display,False);
  int activations=0; XEvent event;
  while(XPending(x.display)){XNextEvent(x.display,&event);if(event.type==ClientMessage && event.xclient.message_type==active)activations++;}
  if(activations!=1){fprintf(stderr,"Restore activation count %d\n",activations);failed=1;}
  xf_ShowWindow(&x,&w,states[i]);XSync(x.display,False);
  while(XPending(x.display)){XNextEvent(x.display,&event);if(event.type==ClientMessage && event.xclient.message_type==active){fprintf(stderr,"Repeated show stole focus\n");failed=1;}}
  XWindowAttributes a={0}; XGetWindowAttributes(x.display,w.handle,&a);
  if(w.minimized || a.map_state!=IsViewable || w.rail_state!=states[i] ||
     w.maxVert!=(states[i]==WINDOW_SHOW_MAXIMIZED) || w.maxHorz!=(states[i]==WINDOW_SHOW_MAXIMIZED)) {
   fprintf(stderr,"Restore state %u: minimized=%d mapped=%d max=%d/%d\n",states[i],w.minimized,a.map_state,w.maxVert,w.maxHorz); failed=1;
  }
 }
 /* A stale expose beyond the resized window must be ignored, not cast
  * from a negative size to unsigned (the observed offline crash). */
 w.surfaceId=UINT16_MAX; w.width=64; w.height=64; w.pixmapWidth=64; w.pixmapHeight=64;
 w.windowOffsetX=10; w.windowOffsetY=10;
 w.pixmap=XCreatePixmap(x.display,w.handle,64,64,DefaultDepthOfScreen(x.screen));
 w.gc=XCreateGC(x.display,w.pixmap,0,NULL);
 x.image=XGetImage(x.display,RootWindowOfScreen(x.screen),0,0,128,128,AllPlanes,ZPixmap);
 freerdp_settings_set_bool(x.common.context.settings,FreeRDP_SoftwareGdi,TRUE);
 xf_UpdateWindowArea(&x,&w,128,128,4,4);
 xf_UpdateWindowArea(&x,&w,0,0,0,0);
 XPutPixel(x.image,73,73,0x123456);
 xf_UpdateWindowArea(&x,&w,0,0,64,64);
 XSync(x.display,False);
 XImage* painted=XGetImage(x.display,w.pixmap,63,63,1,1,AllPlanes,ZPixmap);
 if(!painted || XGetPixel(painted,0,0)!=0x123456){fprintf(stderr,"Last row/column not repainted\n");failed=1;}
 if(painted)XDestroyImage(painted);
 XDestroyImage(x.image);XFreeGC(x.display,w.gc);XFreePixmap(x.display,w.pixmap);
 /* Asynchronous property order: old Normal must not undo remote Minimize,
  * and old Iconic must not undo a remote Restore. A real later user action
  * remains accepted, and a missing WM acknowledgment cannot block forever. */
 w.rail_state=WINDOW_SHOW_MINIMIZED;w.rail_state_pending=TRUE;w.rail_state_pending_since=100;
 if(xf_rail_local_state_ready(&w,WINDOW_SHOW,101))failed=1;
 if(!xf_rail_local_state_ready(&w,WINDOW_SHOW_MINIMIZED,110)||w.rail_state_pending)failed=1;
 if(!xf_rail_local_state_ready(&w,WINDOW_SHOW,120))failed=1;
 w.rail_state=WINDOW_SHOW;w.rail_state_pending=TRUE;w.rail_state_pending_since=200;
 if(xf_rail_local_state_ready(&w,WINDOW_SHOW_MINIMIZED,201))failed=1;
 if(!xf_rail_local_state_ready(&w,WINDOW_SHOW,210)||w.rail_state_pending)failed=1;
 if(!xf_rail_local_state_ready(&w,WINDOW_SHOW_MINIMIZED,220))failed=1;
 w.rail_state_pending=TRUE;w.rail_state_pending_since=300;
 if(!xf_rail_local_state_ready(&w,WINDOW_SHOW_MINIMIZED,1800)||w.rail_state_pending)failed=1;
 if(xf_rail_local_state_ready(&w,WINDOW_HIDE,1801))failed=1;
 XDestroyWindow(x.display,w.handle); freerdp_settings_free(x.common.context.settings); XCloseDisplay(x.display);
 return failed;
}
