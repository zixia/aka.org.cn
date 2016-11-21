int vga_waitevent(int which, fd_set * in, fd_set * out, fd_set * except,
		  struct timeval *timeout)
{
    fd_set infdset;
    int fd, retval;

    if (!in) {
	in = &infdset;
	FD_ZERO(in);
    }
    fd = __svgalib_mouse_fd;	/* __svgalib_mouse_fd might change on
				   vc switch!! */
    if ((which & VGA_MOUSEEVENT) && (fd >= 0))
	FD_SET(fd, in);
    if (which & VGA_KEYEVENT) {
	fd = __svgalib_kbd_fd;
	if (fd >= 0) {		/* we are in raw mode */
	    FD_SET(fd, in);
	} else {
	    FD_SET(__svgalib_tty_fd, in);
	}
    }
    if (select(FD_SETSIZE, in, out, except, timeout) < 0)
	return -1;
    retval = 0;
    fd = __svgalib_mouse_fd;
    if ((which & VGA_MOUSEEVENT) && (fd >= 0)) {
	if (FD_ISSET(fd, in)) {
	    retval |= VGA_MOUSEEVENT;
	    FD_CLR(fd, in);
	    mouse_update();
	}
    }
    if (which & VGA_KEYEVENT) {
	fd = __svgalib_kbd_fd;
	if (fd >= 0) {		/* we are in raw mode */
	    if (FD_ISSET(fd, in)) {
		FD_CLR(fd, in);
		retval |= VGA_KEYEVENT;
		keyboard_update();
	    }
	} else if (FD_ISSET(__svgalib_tty_fd, in)) {
	    FD_CLR(__svgalib_tty_fd, in);
	    retval |= VGA_KEYEVENT;
	}
    }
    return retval;
}

