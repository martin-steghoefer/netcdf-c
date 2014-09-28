#ifndef NCCOMPRESS_H
#define NCCOMPRESS_H

#include <hdf5.h>

/* It should be possible to overlay this
on the params[] to extract the values.
This should matc the union comment in netcdf.h.
*/
typedef union {
    int params[NC_COMPRESSION_MAX_PARAMS];
    unsigned int level; /* e.g zip, bzip2 */
    struct {
        unsigned int options_mask;
	unsigned int bits_per_pixel;
        unsigned int pixels_per_block;
	unsigned int pixels_per_scanline;
    } szip;
    struct {
	int isdouble; /* 0=> double precision 1=> single precision */
        int ny;
        int nz;
        int minbits;
        int maxbits;
        int maxprec;
        int minexp;
    } zfpzip;
} nc_compression_t;

/*
Turn on specified compression for a variable's plist
*/
extern int nccompress_set(const char* algorithm, hid_t plistid, int*);

extern int nccompress_inq_parameters(H5Z_filter_t filter, hid_t, size_t, unsigned int*,char*,int*);

/* 
Register all compression filters with the library
*/
extern int nccompress_register_all(void);

#endif /*NCCOMPRESS_H*/
