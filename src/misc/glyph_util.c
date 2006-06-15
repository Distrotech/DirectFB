/*
 * (c) Copyright 2004-2006 Mitsubishi Electric Corp.
 *
 * All rights reserved.
 *
 * Written by Toshiyuki Takahashi,
 *            Koichi Hiramatsu,
 *            Seishi Takahashi,
 *            Atsushi Hori
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "misc/glyph_util.h"

static int
trn_scaling_enlarge[16]={
	0, /* left=0, right=0 -> out=0 */
	0, /* left=0, right=1 -> out=0 */
	1, /* left=0, right=2 -> out=1 */
	2, /* left=0, right=3 -> out=2 */
	0, /* left=1, right=0 -> out=0 */
	1, /* left=1, right=1 -> out=1 */
	1, /* left=1, right=2 -> out=1 */
	2, /* left=1, right=3 -> out=2 */
	1, /* left=2, right=0 -> out=1 */
	1, /* left=2, right=1 -> out=1 */
	2, /* left=2, right=2 -> out=2 */
	3, /* left=2, right=3 -> out=3 */
	2, /* left=3, right=0 -> out=2 */
	2, /* left=3, right=1 -> out=2 */
	3, /* left=3, right=2 -> out=3 */
	3  /* left=3, right=3 -> out=3 */
};

static int
trn_scaling_reduction[16]={
	0, /* left=0, right=0 -> out=0 */
	1, /* left=0, right=1 -> out=1 */
	1, /* left=0, right=2 -> out=1 */
	2, /* left=0, right=3 -> out=2 */
	1, /* left=1, right=0 -> out=1 */
	1, /* left=1, right=1 -> out=1 */
	2, /* left=1, right=2 -> out=2 */
	2, /* left=1, right=3 -> out=2 */
	1, /* left=2, right=0 -> out=1 */
	2, /* left=2, right=1 -> out=2 */
	2, /* left=2, right=2 -> out=2 */
	3, /* left=2, right=3 -> out=3 */
	2, /* left=3, right=0 -> out=2 */
	2, /* left=3, right=1 -> out=2 */
	3, /* left=3, right=2 -> out=3 */
	3  /* left=3, right=3 -> out=3 */
};

static int
trn_bold_comp_ref0[16]={
    0, /* left=0, right=0 -> out=0 */
    1, /* left=0, right=1 -> out=1 */
    2, /* left=0, right=2 -> out=2 */
    3, /* left=0, right=3 -> out=3 */
    1, /* left=1, right=0 -> out=1 */
    1, /* left=1, right=1 -> out=1 */
    2, /* left=1, right=2 -> out=2 */
    3, /* left=1, right=3 -> out=3 */
    2, /* left=2, right=0 -> out=2 */
    2, /* left=2, right=1 -> out=1 */
    2, /* left=2, right=2 -> out=2 */
    3, /* left=2, right=3 -> out=3 */
    3, /* left=3, right=0 -> out=3 */
    3, /* left=3, right=1 -> out=1 */
    3, /* left=3, right=2 -> out=2 */
    3  /* left=3, right=3 -> out=3 */
};


#define		H_ENLARGE_FILTER(d,s,w,x) 													\
			{																			\
		    	if(w -x >= 5)	{														\
                	*d = (*s & 0xC0) |													\
							(trn_scaling_enlarge[(*s>>4)]<<4) | 						\
							((*s & 0x30)>>2) |											\
							(trn_scaling_enlarge[((*s & 0x3C)>>2)]); 					\
					d++;																\
                	*d = ((*s & 0x0C)<<4) | 											\
							(trn_scaling_enlarge[(*s & 0x0F)]<<4) | 					\
							((*s & 0x03)<<2) |											\
							(trn_scaling_enlarge[((*s & 0x03)<<2) | (*(s+1)>>6)]); 		\
					d++;																\
		    	}																		\
                else if (w - x >= 2) {													\
					int	adr;															\
					if(w - x == 2)														\
						adr = (*s & 0x30) >> 2;											\
					else																\
						adr = (*s & 0x3C) >> 2;											\
                	*d++ = (*s & 0xC0) |												\
							(trn_scaling_enlarge[(*s>>4)]<<4) | 						\
							((*s & 0x30)>>2) |											\
							trn_scaling_enlarge[adr]; 									\
                    if (w - x == 3) {													\
                		*d = ((*s & 0x0C)<<4) | 										\
							(trn_scaling_enlarge[(*s & 0x0C)]<<4) | 					\
							(*d & 0x0F) ;												\
						d++;															\
					}																	\
                    else if (w - x == 4) {												\
                		*d++ = ((*s & 0x0C)<<4) | 										\
							(trn_scaling_enlarge[(*s & 0x0C)]<<4) | 					\
							((*s & 0x03)<<2) |											\
							(trn_scaling_enlarge[(*s & 0x03)<<2]); 						\
					}																	\
		        }																		\
                if (w - x == 1) {														\
                	*d = (*s & 0xC0) | 													\
							(trn_scaling_enlarge[(*s & 0xC0)>>4]<<4) |					\
							(*d & 0x0F) ;												\
					d++;																\
                }																		\
            }																			\


#define		H_REDUCT_FILTER(d,s,w,x) 													\
			{																			\
                if (w - x >= 8) {														\
					*d = (trn_scaling_reduction[(*s & 0xF0)>>4]<<6) | 					\
					     (trn_scaling_reduction[(*s & 0x0F)]<<4) ; 						\
                	s++;																\
					*d |=(trn_scaling_reduction[(*s & 0xF0)>>4]<<2) | 					\
				     (trn_scaling_reduction[(*s & 0x0F)]) ;							\
                	s++;																\
		        }																		\
                else if (w - x >= 6) {													\
					*d = (trn_scaling_reduction[(*s & 0xF0)>>4]<<6) | 					\
					     (trn_scaling_reduction[(*s & 0x0F)]<<4) ; 						\
                	s++;																\
					*d |=((trn_scaling_reduction[(*s & 0xF0)>>4]<<2) | 					\
               	     	  (*d & 0x03)) ;												\
                	s++;																\
                }																		\
                else if (w - x >= 4) {													\
					*d = (trn_scaling_reduction[(*s & 0xF0)>>4]<<6) | 					\
					     (trn_scaling_reduction[(*s & 0x0F)]<<4) | 						\
               	         (*d & 0x0F) ;													\
                	s++;																\
                }																		\
                else if (w - x >= 2) {													\
					*d = (trn_scaling_reduction[(*s & 0xF0)>>4]<<6) | 					\
               	         (*d & 0x3F) ;													\
                	s++;																\
                }																		\
            }																			\



/**********************************************************************
 *
 * A2 (four-level greyscale) Glyph Utilities
 *
 **********************************************************************
 */
/*
	dfb_copy_glyph_a2: simple copy method
			sbuf   : buffer for source glyph
			spitch : byte per line of source glyph
			dbuf   : buffer for destination glyph
			dpitch : byte per line of destination glyph
			w 	   : number of pixels per line
			h 	   : number of lines per glyph
*/
void
dfb_copy_glyph_a2(__u8* sbuf, int spitch,
                  __u8* dbuf, int dpitch,
                  int w, int h)
{
     int x, y;
     __u8 *s;
     __u8 *d;
     __u8 mask;
     int pad_bits;

     for (y = 0; y < h; y++) {
          s = sbuf + spitch * y;
          d = dbuf + dpitch * y;
          for (x = 0; x < w; x += 4) {
               if (w - x >= 4) {
                    *d++ = *s++;
               }
               else {
                    pad_bits = (4 - (w - x)) * 2;
                    mask = (0xFF >> pad_bits) << pad_bits;
                    *d = (*s & mask) | (*d & ~mask);
               }
          }
     }
}


/*
	dfb_scaling_copy_glyph_a2: scaling method  using averaging filter
							scaling ratio can be set to 1, 2, 1/2 
							for each H or V direction.
			sbuf   : buffer for source glyph
			spitch : byte per line of source glyph
			dbuf   : buffer for destination glyph
			dpitch : byte per line of destination glyph
			w 	   : number of pixels per line
			h 	   : number of lines per glyph
			horizontal: horizontal ratio (0: non-scale, <0: 1/2, >0: *2) 
			vertical  : vertical   ratio (0: non-scale, <0: 1/2, >0: *2) 
*/
void
dfb_scaling_copy_glyph_a2(__u8* sbuf, int spitch,
                          __u8* dbuf, int dpitch,
                          int w, int h,
                          int horizontal, /* 0: non-scale, <0: 1/2, >0: *2 */
                          int vertical    /* 0: non-scale, <0: 1/2, >0: *2 */
                          )
{
     int x, y;
     __u8 *s;
     __u8 zero = 0;
     __u8 *d, *d1, *d2, *d3;
     __u8 mask;
     int pad_bits;
	int  w2;

     for (y = 0; y < h; y++) {
     	if (vertical > 0) {
	    	s = sbuf + spitch * y;
            d = dbuf + dpitch * y * 2;
		}
     	else if (vertical < 0) {
               /* too lazy, but it won't be used */
               s = sbuf + spitch * y;
               d = dbuf + dpitch * y/2;
               y++;
     	}
     	else {
               s = sbuf + spitch * y;
               d = dbuf + dpitch * y;
     	}
      	if (horizontal > 0) {
            for (x = 0; x < w; x += 4, s++) {
				H_ENLARGE_FILTER(d,s,w,x) 
			}

        }
        else if (horizontal < 0)	{
            for (x = 0; x < w; x += 8, d++) {

				H_REDUCT_FILTER(d,s,w,x) 
            }
        }
        else	{  /* if horizontal = 0 */
               for (x = 0; x < w; x += 4) {
                    if (w - x >= 4) {
                         *d++ = *s++;
                    }
                    else {
                         pad_bits = (4 - (w - x)) * 2;
                         mask = (0xFF >> pad_bits) << pad_bits;
                         *d = (*s & mask) | (*d & ~mask);
                    }
               }
        }
    }    /* end of for-loop of "y" */
     

	if(horizontal > 0)				w2 = w * 2;
	else if(horizontal < 0)			w2 = w >> 1;
	else /*if horizontal == 0 */	w2 = w;

    for (y = 0; y < h; y++) {
     	if (vertical > 0) {
            d1 = dbuf + dpitch * y * 2;
            d2 = dbuf + dpitch * (y * 2 + 1) ;
	    	if (y == h-1)
	 			d3 = &zero;
	    	else
            	d3 = dbuf + dpitch * (y * 2 + 2);

        	for (x = 0; x < w2 ; x += 4) {
            	*d2 = 
						(trn_scaling_enlarge[((*d1 & 0xC0)>>4) | (*d3 >>6)] << 6) | 
						(trn_scaling_enlarge[((*d1 & 0x30)>>2) | ((*d3 & 0x30)>>4)] << 4) |
						(trn_scaling_enlarge[(*d1 & 0x0C) | ((*d3 & 0x0C)>>2)] << 2) |
						 trn_scaling_enlarge[((*d1 & 0x03)<<2) | (*d3 & 0x03)]  ;
				d1++;
				d2++;
				if(y != h-1)	d3++;
        	}
    	}
     	else if (vertical < 0) {
            s = sbuf + spitch * (y + 1);
            d = dbuf + dpitch * y/2;
            y++;

			if(horizontal > 0)	{
			  __u8 *workbuf, tmpbuf[3];

              for (x = 0; x < w ; x += 4, s++) {
			    tmpbuf[0] = 0;
			    tmpbuf[1] = 0;
				workbuf = tmpbuf;

				H_ENLARGE_FILTER(workbuf,s,w,x)

				*d = (trn_scaling_reduction[((tmpbuf[0] & 0xC0) >> 4) | ((*d & 0xC0)>>6)]<<6) | 
					 (trn_scaling_reduction[((tmpbuf[0] & 0x30) >> 2) | ((*d & 0x30)>>4)]<<4) | 
					 (trn_scaling_reduction[(tmpbuf[0] & 0x0C)        | ((*d & 0x0C)>>2)]<<2) | 
					 (trn_scaling_reduction[((tmpbuf[0] & 0x03) << 2) | (*d & 0x03)]) ; 
				d++;
				*d = (trn_scaling_reduction[((tmpbuf[1] & 0xC0) >> 4) | ((*d & 0xC0)>>6)]<<6) | 
					 (trn_scaling_reduction[((tmpbuf[1] & 0x30) >> 2) | ((*d & 0x30)>>4)]<<4) | 
					 (trn_scaling_reduction[(tmpbuf[1] & 0x0C)        | ((*d & 0x0C)>>2)]<<2) | 
					 (trn_scaling_reduction[((tmpbuf[1] & 0x03) << 2) | (*d & 0x03)]) ; 
				d++;
			  }
			}
			else if(horizontal < 0)	{
              for (x = 0; x < w ; x += 8, d++) {
				__u8 *s_tmp, tmp = 0;

				s_tmp = &tmp;
				H_REDUCT_FILTER(s_tmp,s,w,x) 
 
				*d = (trn_scaling_reduction[((*s_tmp & 0xC0)>>4) | ((*d & 0xC0)>>6)]<<6) | 
				     (trn_scaling_reduction[((*s_tmp & 0x30)>>2) | ((*d & 0x30)>>4)]<<4) | 
				     (trn_scaling_reduction[ (*s_tmp & 0x0C)     | ((*d & 0x0C)>>2)]<<2) | 
				     (trn_scaling_reduction[((*s_tmp & 0x03)<<2) | (*d & 0x03)]) ; 
     	      }
     	    }
			else /* if(horizontal == 0) */	{
              for (x = 0; x < w2 ; x += 4) {
				*d = (trn_scaling_reduction[((*s & 0xC0)>>4) | ((*d & 0xC0)>>6)]<<6) | 
				     (trn_scaling_reduction[((*s & 0x30)>>2) | ((*d & 0x30)>>4)]<<4) | 
				     (trn_scaling_reduction[(*s & 0x0C) | ((*d & 0x0C)>>2)]<<2) | 
				     (trn_scaling_reduction[((*s & 0x03)<<2) | (*d & 0x03)]) ; 
                s++;
                d++;
              }
		    }
     	}
    }  /* end of for-loop of "y" */
}


/*
	dfb_draw_glyph_a2: (LUT8) colorized copy method
			sbuf   : buffer for source glyph
			spitch : byte per line of source glyph
			dbuf   : buffer for destination glyph
			dpitch : byte per line of destination glyph
			w 	   : number of pixels per line
			h 	   : number of lines per glyph
               palette[]: four entry array of color indexes for LUT8
                          minus for transparent
*/
void
dfb_draw_glyph_a2(__u8* sbuf, int spitch,
                  __u8* dbuf, int dpitch,
                  int w, int h, int palette[])
{
     int x, y, k;
     __u8 *s;
     __u8 *d;

     for (y = 0; y < h; y++) {
          s = sbuf + spitch * y;
          d = dbuf + dpitch * y;
          for (x = 0; x < w; ) {
               for (k = 0; (k < 4) && (x < w); k++, x++) {
                    __u8 tmp = (*s >> (6 - 2*k)) & 0x03;
                    if (palette[tmp] < 0) { /* transparent */
                         d++;
                    }
                    else {
                         *d++ = palette[tmp];
                    }
               }
               s++;
          }
     }
}




/*
	bold_copy_glyph: bolding with copying method
			sbuf   : buffer for source glyph
			spitch : byte per line of source glyph
			dbuf   : buffer for destination glyph
			dpitch : byte per line of destination glyph
			w 	   : number of pixels per line
			h 	   : number of lines per glyph
			fatdots: the number of fat dots which is the parameter of this algorithm
					(0: 1 dot fat, else : 2 dots fat)
*/

static void
bold_copy_glyph_a2(__u8* sbuf, int spitch,
                   __u8* dbuf, int dpitch,
                   int	 w,    int h,
                   int   fatdots	/* if 0: 1dot fat, else: 2dots fat */
                   )
{
    int x, y;
    __u8 *s, s_tmp;
    __u8 *d, d_tmp;
    __u8 mask;
    int pad_bits;

	fatdots = 0;

    if(fatdots == 0)	{
	   for (y = 0; y < h; y++) {
           s = sbuf + spitch * y;
           d = dbuf + dpitch * y;
           for (x = 0; x < w; x += 4, s++) {

		     if(x == 0) 	s_tmp = 0;
		     else			s_tmp = *(s-1);
           
		     if (w - x >= 4)
                 *d++ = (trn_bold_comp_ref0[((s_tmp & 0x03)<<2) + ((*s & 0xc0)>>6)]<<6) + 
					    (trn_bold_comp_ref0[((*s & 0xf0)>>4)]<<4) + 
					    (trn_bold_comp_ref0[((*s & 0x3c)>>2)]<<2) +
					    trn_bold_comp_ref0[(*s & 0x0f)] ;
             else {
                 d_tmp= (trn_bold_comp_ref0[((s_tmp & 0x03)<<2) + ((*s & 0xc0)>>6)]<<6) + 
					    (trn_bold_comp_ref0[((*s & 0xf0)>>4)]<<4) + 
					    (trn_bold_comp_ref0[((*s & 0x3c)>>2)]<<2) +
					    trn_bold_comp_ref0[(*s & 0x0f)] ;
                 pad_bits = (4 - (w - x)) * 2;
                 mask = (0xFF >> pad_bits) << pad_bits;
                 *d = (d_tmp & mask) | (*d & ~mask);
             }
           }
	   }
    }
}



/*
	dfb_bold_glyph_a2: bolding function
			sbuf   : buffer for source glyph
			spitch : byte per line of source glyph
			dbuf   : buffer for destination glyph
			dpitch : byte per line of destination glyph
			w 	   : number of pixels per line
			h 	   : number of lines per glyph
*/
void
dfb_bold_glyph_a2(__u8* sbuf, int spitch,
                  __u8* dbuf, int dpitch,
                  int	 w  , int h)
{
	bold_copy_glyph_a2(sbuf, spitch, dbuf, dpitch, w, h, 0);
}


/*
	dfb_frame_glyph_a2: framing function
				if center pixel is colored, destination pixel is set to "3".
				if there are some colored pixel in neighbour 8 pixels,
					destination pixel is set to "3".
					 the sum of the weights must be beyond 1.
			sbuf   : buffer for source glyph
			spitch : byte per line of source glyph
			dbuf   : buffer for destination glyph
			dpitch : byte per line of destination glyph
			w 	   : number of pixels per line
			h 	   : number of lines per glyph
*/

void
dfb_frame_glyph_a2(__u8* sbuf, int spitch,
                   __u8* dbuf, int dpitch,
                   int	 w,    int h)
{
    int x, y;
    __u8 *s1, *s2, *s3;
	__u8 zero[3]={0, 0, 0};	/* dummy "0" buffer for pre-line and next-line */
    __u8 *d, d_tmp;
    __u8 mask;
	__u8 s1_1, s1_2, s2_1, s2_2, s3_1, s3_2, dx;
    int pad_bits;

	for (y = 0; y < h; y++) {
		/* if y=0, dummy "0" buffer is set to preline buffer s1 */ 
		s1 = (y == 0)   ? &zero[1] : sbuf + spitch * (y-1);	
		/* if y=h-1, dummy "0" buffer is set to nextline buffer s3 */ 
		s3 = (y == h-1) ? &zero[1] : sbuf + spitch * (y+1);	
		s2 = sbuf + spitch * y;
		
		d  = dbuf + dpitch * y;

		for (x = 0; x < w; x += 4, d++)	{
			/* pre-line     continuous 3 byte : s1_1 , *s1, s1_2 */
			/* current-line continuous 3 byte : s2_1 , *s2, s2_2 */
			/* next-line    continuous 3 byte : s3_1 , *s3, s3_2 */
			s1_1 = (x==0)   ? 0 : *(s1-1);
			s1_2 = (w-x<=4) ? 0 : *(s1+1);
			s2_1 = (x==0)   ? 0 : *(s2-1);
			s2_2 = (w-x<=4) ? 0 : *(s2+1);
			s3_1 = (x==0)   ? 0 : *(s3-1);
			s3_2 = (w-x<=4) ? 0 : *(s3+1);

			/* if center pixel is colored, *d is set to "0" */
			/* if there are some colored pixel in neighbour 8 pixel,
				*d is set to "3"  */
			if((*s2&0xc0) != 0)
				d_tmp = 0;
			else	{
				dx =  (s1_1&0x03) | (*s1>>6) | ((*s1&0x30)>>4) |
						(s2_1&0x03) |            ((*s2&0x30)>>4) |
						(s3_1&0x03) | (*s3>>6) | ((*s3&0x30)>>4) ;
				if(dx != 0)		d_tmp = 0xc0;
				else			d_tmp = 0;
			}
			if((*s2&0x30) == 0)	{
				dx =  (*s1>>6) | ((*s1&0x30)>>4) | ((*s1&0x0c)>>2) |
						(*s2>>6) |                   ((*s2&0x0c)>>2) |
						(*s3>>6) | ((*s3&0x30)>>4) | ((*s3&0x0c)>>2) ;
				if(dx != 0)
					d_tmp |= 0x30;
			}
			if((*s2&0x0c) == 0)	{
				dx =  ((*s1&0x30)>>4) | ((*s1&0x0c)>>2) | (*s1&0x03) |
						((*s2&0x30)>>4) |                   (*s2&0x03) |
						((*s3&0x30)>>4) | ((*s3&0x0c)>>2) | (*s3&0x03) ;
				if(dx != 0)
					d_tmp |= 0x0c;
			}
			if((*s2&0x03) == 0)	{
				dx =  ((*s1&0x0c)>>2) | (*s1&0x03) |(s1_2>>6) |
						((*s2&0x0c)>>2) |             (s2_2>>6) |
						((*s3&0x0c)>>2) | (*s3&0x03) |(s3_2>>6) ;
				if(dx != 0)
					d_tmp |= 0x03;
			}

			if(w - x >= 4)	{
				*d = d_tmp;
			}
			else	{
                pad_bits = (4 - (w - x)) * 2;
                mask = (0xFF >> pad_bits) << pad_bits;
                *d = (d_tmp & mask) | (*d & ~mask);
			}

			s2++;
			/* dummy "0" buffer is selected in case of y=0, so that "s1" buffer shouldn't be incremented */
			if(y > 0)	s1++;	
			/* dummy "0" buffer is selected in case of y=h-1, so that "s3" buffer shouldn't be incremented */
			if(y < h-1)	s3++;

		}	/* for x */
	}		/* for y */
}



#define DRAW_PIXEL(d,color) { \
               if (color < 0) \
                    d++; \
               else \
                    *d++ = color; \
} \

#define DRAW_FRAME_PIXEL(cond, d,color,fr,bg) { \
               color = (cond) ? fr : bg; \
               DRAW_PIXEL(d,color); \
} \

/*
  edge_line_glyph_a2: framing function for upper & bottom edge line
           if there are some colored pixel in 3 pixels of upper or button line,
           destination pixel is set to "4", and translate with CLUT.
			s   : buffer for referece line of source glyph
			d   : buffer for destination glyph
			w   : number of pixels per line of source
               background_color: LUT8 color index for background
               frame_color: LUT8 color index for frame
*/
static void
edge_line_glyph_a2(__u8* s,
                   __u8* d,
                   int w, int background_color, int frame_color)
{
     int	x, c, pad_bits, mask;
     __u8	dx, s_1, s_2;
     int color;
     
     for (x = 0, c=0; ; x += 4, s++)	{
          /* if y==0 -> s=next-line  ->  continuous 3 byte : s_1 , *s, s_2 */
          s_1 = (x==0)   ? 0 : *(s-1);
          s_2 = (w-x<=4) ? 0 : *(s+1);
          
          /* mask processing for line edge */
          if( w-x < 4)	{
               pad_bits = (4 - (w - x)) * 2;
               mask = (0xFF >> pad_bits) << pad_bits;
               *s = *s & mask;
          }

          if(x == 0)
               DRAW_FRAME_PIXEL(*s&0xc0,d,color,frame_color,background_color); /* edge pel */
          
          dx =  (s_1&0x03) | (*s>>6) | ((*s&0x30)>>4) ;
          DRAW_FRAME_PIXEL(dx,d,color,frame_color,background_color); /* 1st pel */

          c++;
          if(c >= w)	{
               DRAW_FRAME_PIXEL(*s&0xc0,d,color,frame_color,background_color); /* edge pel */
               break;
          }
          
          dx =  (*s>>6) | ((*s&0x30)>>4) | ((*s&0x0c)>>2) ;
          DRAW_FRAME_PIXEL(dx,d,color,frame_color,background_color); /* 2nd pel */
          c++;
          if(c >= w)	{
               DRAW_FRAME_PIXEL(*s&0x30,d,color,frame_color,background_color); /* edge pel */
               break;
          }
          
          dx =  ((*s&0x30)>>4) | ((*s&0x0c)>>2) | (*s&0x03) ;
          DRAW_FRAME_PIXEL(dx,d,color,frame_color,background_color); /* 3rd pel */
          c++;
          if(c >= w)	{
               DRAW_FRAME_PIXEL(*s&0x0c,d,color,frame_color,background_color); /* edge pel */
               break;
          }
          
          dx =  ((*s&0x0c)>>2) | (*s&0x03) |(s_2>>6) ;
          DRAW_FRAME_PIXEL(dx,d,color,frame_color,background_color); /* 4th pel */
          c++;
          if(c >= w)	{
               DRAW_FRAME_PIXEL(*s&0x03,d,color,frame_color,background_color); /* edge pel */
               break;
          }
     }
}


/*
	dfb_draw_framed_glyph_a2: (LUT8) colorized copy method with adding frame
          if center pixel is colored, destination pixel is drawn with the palette.
          if there are some colored pixel in neighbour 8 pixels,
          destination pixel is drawn as frame_color.
			sbuf   : buffer for source glyph (2bpp)
			spitch : byte per line of source glyph
			dbuf   : buffer for destination glyph (8bpp)
			dpitch : byte per line of destination glyph
			w 	   : number of pixels per line of source
			h 	   : number of lines per glyph of source
			( destination w is w+2, destination h is h+2)
               palette[]: four entry array of color indexes for LUT8
                          minus for transparent
               frame_color: LUT8 color index for frame
                          minus for transparent
*/
void
dfb_draw_framed_glyph_a2(__u8* sbuf, int spitch,
                         __u8* dbuf, int dpitch,
                         int w, int h, int palette[], int frame_color)
{
     int x, y;
     __u8 *s1, *s2, *s3;
	__u8 zero[3]={0, 0, 0};	/* dummy "0" buffer for pre-line and next-line */
     __u8 *d;
     __u8 mask;
     __u8 s1_0, s1_1, s1_2, s2_0, s2_1, s2_2, s3_0, s3_1, s3_2, dx;
     int pad_bits, c;
     int color;
     
     if (frame_color < 0) {
          dfb_draw_glyph_a2(sbuf, spitch, dbuf, dpitch, w, h, palette);
          return;
     }

     for (y = 0; y < h; y++) {

          /* if y=0, dummy "0" buffer is set to preline buffer s1 */ 
          s1 = (y == 0)   ? &zero[1] : sbuf + spitch * (y-1);	

		/* if y=h-1, dummy "0" buffer is set to nextline buffer s3 */ 
          s3 = (y == h-1) ? &zero[1] : sbuf + spitch * (y+1);	

          s2 = sbuf + spitch * y;

          if(y == 0)	{
               edge_line_glyph_a2(s2, dbuf, w, palette[0], frame_color);
          }
          
          d  = dbuf + dpitch * (y+1);

          for (x=0, c=0; ; x += 4)	{
               /* pre-line     continuous 3 byte : s1_1 , s1_0, s1_2 */
               /* current-line continuous 3 byte : s2_1 , s2_0, s2_2 */
               /* next-line    continuous 3 byte : s3_1 , s3_0, s3_2 */
               s1_1 = (x==0)   ? 0 : *(s1-1);
               s1_2 = (w-x<=4) ? 0 : *(s1+1);
               s2_1 = (x==0)   ? 0 : *(s2-1);
               s2_2 = (w-x<=4) ? 0 : *(s2+1);
               s3_1 = (x==0)   ? 0 : *(s3-1);
               s3_2 = (w-x<=4) ? 0 : *(s3+1);

               /* if center pixel is colored, *d is set to the same color
                * if there are some colored pixel in neighbour 8 pixel,
                * *d is set to "4"
                */

               /* mask processing for line edge */
               if( w-x < 4)	{
                    pad_bits = (4 - (w - x)) * 2;
                    mask = (0xFF >> pad_bits) << pad_bits;
                    s1_0 = *s1 & mask;
                    s2_0 = *s2 & mask;
                    s3_0 = *s3 & mask;
               }
               else	{
                    s1_0 = *s1;
                    s2_0 = *s2;
                    s3_0 = *s3;
               }

               if (x==0)	{
                    dx = (s1_0>>6) |
                         (s2_0>>6) |
                         (s3_0>>6) ;
                    DRAW_FRAME_PIXEL(dx,d,color,frame_color,palette[0]); /* edge pel (line head) */
			}

			/* 1st  pel */
               if(s2_0>>6)
                    DRAW_PIXEL(d,palette[s2_0>>6])
               else	{
                    dx = (s1_1&0x03) | (s1_0>>6) | ((s1_0&0x30)>>4) |
                         (s2_1&0x03) |             ((s2_0&0x30)>>4) |
                         (s3_1&0x03) | (s3_0>>6) | ((s3_0&0x30)>>4) ;
                    DRAW_FRAME_PIXEL(dx,d,color,frame_color,palette[0]);
			}
               c++;
               if(c >= w)	{
                    dx = (s1_0>>6) |
                         (s2_0>>6) |
                         (s3_0>>6) ;
                    DRAW_FRAME_PIXEL(dx,d,color,frame_color,palette[0]); /* edge pel (line end) */
				break;
			}

			/* 2nd  pel */
               if(s2_0&0x30)
                    DRAW_PIXEL(d,palette[(s2_0&0x30)>>4])
               else	{
                    dx = (s1_0>>6) | ((s1_0&0x30)>>4) | ((s1_0&0x0c)>>2) |
                         (s2_0>>6) |                    ((s2_0&0x0c)>>2) |
                         (s3_0>>6) | ((s3_0&0x30)>>4) | ((s3_0&0x0c)>>2) ;
                    DRAW_FRAME_PIXEL(dx,d,color,frame_color,palette[0]);
			}
			c++;
			if(c >= w)	{
                    dx = (s1_0&0x30) |
                         (s2_0&0x30) |
                         (s3_0&0x30) ;
                    DRAW_FRAME_PIXEL(dx,d,color,frame_color,palette[0]); /* edge pel (line end) */
				break;
			}

			/* 3rd  pel */
               if(s2_0&0x0c)
                    DRAW_PIXEL(d,palette[(s2_0&0x0c)>>2])
               else	{
                    dx = ((s1_0&0x30)>>4) | ((s1_0&0x0c)>>2) | (s1_0&0x03) |
                         ((s2_0&0x30)>>4) |                    (s2_0&0x03) |
                         ((s3_0&0x30)>>4) | ((s3_0&0x0c)>>2) | (s3_0&0x03) ;
                    DRAW_FRAME_PIXEL(dx,d,color,frame_color,palette[0]);
			}
			c++;
			if(c >= w)	{
                    dx = (s1_0&0x0c) |
                         (s2_0&0x0c) |
                         (s3_0&0x0c) ;
                    DRAW_FRAME_PIXEL(dx,d,color,frame_color,palette[0]); /* edge pel (line end) */
                    break;
               }

               /* 4th  pel */
               if(s2_0&0x03)
                    DRAW_PIXEL(d,palette[s2_0&0x03])
               else	{
                    dx = ((s1_0&0x0c)>>2) | (s1_0&0x03) |(s1_2>>6) |
                         ((s2_0&0x0c)>>2) |              (s2_2>>6) |
                         ((s3_0&0x0c)>>2) | (s3_0&0x03) |(s3_2>>6) ;
                    DRAW_FRAME_PIXEL(dx,d,color,frame_color,palette[0]);
               }
               c++;
               if(c >= w)	{
                    dx = (s1_0&0x03) |
                         (s2_0&0x03) |
                         (s3_0&0x03) ;
                    DRAW_FRAME_PIXEL(dx,d,color,frame_color,palette[0]); /* edge pel (line end) */
				break;
			}

               s2++;
               /* dummy "0" buffer is selected in case of y=0, so that "s1" buffer shouldn't be incremented */
               if (y > 0)
                    s1++;	
               /* dummy "0" buffer is selected in case of y=h-1, so that "s3" buffer shouldn't be incremented */
               if (y < h-1)
                    s3++;

          }	/* for x */

          if(y == h-1)	{
               s2 = sbuf + spitch * y;
               d  = dbuf + dpitch * (y+2);
               edge_line_glyph_a2(s2, d, w, palette[0], frame_color);
          }
	
     }		/* for y */
}

/* end of glyph_util.c */

