/*
 * iobuf.h
 *
 * Defines the structures used to track abstract kernel-space io buffers.
 *
 */

#ifndef __LINUX_IOBUF_H
#define __LINUX_IOBUF_H

#include <linux/mm.h>
#include <linux/init.h>

/*
 * The kiobuf structure describes a physical set of pages reserved
 * locked for IO.  The reference counts on each page will have been
 * incremented, and the flags field will indicate whether or not we have
 * pre-locked all of the pages for IO.
 *
 * kiobufs may be passed in arrays to form a kiovec, but we must
 * preserve the property that no page is present more than once over the
 * entire iovec.
 */

#define KIO_MAX_ATOMIC_IO	64 /* in kb */
#define KIO_MAX_ATOMIC_BYTES	(64 * 1024)
#define KIO_STATIC_PAGES	(KIO_MAX_ATOMIC_IO / (PAGE_SIZE >> 10))
#define KIO_MAX_SECTORS		(KIO_MAX_ATOMIC_IO * 2)

struct kiobuf 
{
	int		nr_pages;	/* Pages actually referenced */
	int		array_len;	/* Space in the allocated lists */
	int		offset;		/* Offset to start of valid data */
	int		length;		/* Number of valid bytes of data */

	/* Keep separate track of the physical addresses and page
	 * structs involved.  If we do IO to a memory-mapped device
	 * region, there won't necessarily be page structs defined for
	 * every address. */

	unsigned long *	pagelist;
	struct page **	maplist;
	unsigned long *	bouncelist;

	unsigned int	locked : 1;	/* If set, pages has been locked */
	unsigned int	bounced : 1;	/* If set, bounce pages are set up */
	
	/* Always embed enough struct pages for 64k of IO */
	unsigned long	page_array[KIO_STATIC_PAGES];
	struct page *	map_array[KIO_STATIC_PAGES];
	unsigned long	bounce_array[KIO_STATIC_PAGES];
};


/* mm/memory.c */

int	map_user_kiobuf(int rw, struct kiobuf *, unsigned long va, size_t len);
void	unmap_kiobuf(struct kiobuf *iobuf);

/* fs/iobuf.c */

void __init kiobuf_init(void);
int	alloc_kiovec(int nr, struct kiobuf **);
void	free_kiovec(int nr, struct kiobuf **);
int	expand_kiobuf(struct kiobuf *, int);
int	setup_kiobuf_bounce_pages(struct kiobuf *, int gfp_mask);
void	clear_kiobuf_bounce_pages(struct kiobuf *);
void	kiobuf_copy_bounce(struct kiobuf *, int direction, int max);

/* Direction codes for kiobuf_copy_bounce: */
enum {
	COPY_TO_BOUNCE,
	COPY_FROM_BOUNCE
};

/* fs/buffer.c */

int	brw_kiovec(int rw, int nr, struct kiobuf *iovec[], 
		   kdev_t dev, unsigned long b[], int size, int bmap);

#endif /* __LINUX_IOBUF_H */
