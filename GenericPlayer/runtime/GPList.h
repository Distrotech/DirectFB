/* GLIB - Library of useful routines for C programming
 * Copyright (C) 1995-1997  Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/*
 * Modified by the GLib Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GLib Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GLib at ftp://ftp.gtk.org/pub/gtk/. 
 */

#ifndef GPList_h
#define GPList_h

#ifdef __cplusplus
extern "C" {
#endif


#define GPListDef "gp.GPList( \
    gp.GPlist.next(gp.GPList*)\
    gp.GPlist.prev(gp.GPList*)\
    gp.GPlist.data(void*))"

#define GPSListDef "gp.GPSList( \
    gp.GPSlist.next(gp.GPSList*)\
    gp.GPlist.data(void*))"

typedef struct _GPList GPList;

struct _GPList
{
  GPList *next;
  GPList *prev;
  void* data;
};

typedef struct _GPSList GPSList;

struct _GPSList
{
  GPList *next;
  void* data;
};


typedef void (*GPFunc) (void* data, void* user_data);

typedef int (*GPCompareFunc) (const void*  a, const void*  b);
typedef int (*GPCompareDataFunc)(const void*  a, const void*  b, void* user_data);


/* Doubly linked lists
 */
GPList*   GPList_alloc                   (void) ;
void     GPList_free                    (GPList            *list);
void     GPList_free_1                  (GPList            *list);
#define  GPList_free1                   GPList_free_1
GPList*   GPList_append                  (GPList            *list,
					 void*          data) ;
GPList*   GPList_prepend                 (GPList            *list,
					 void*          data) ;
GPList*   GPList_insert                  (GPList            *list,
					 void*          data,
					 int              position) ;
GPList*   GPList_insert_sorted           (GPList            *list,
					 void*          data,
					 GPCompareFunc      func) ;
GPList*   GPList_insert_sorted_with_data (GPList            *list,
					 void*          data,
					 GPCompareDataFunc  func,
					 void*          user_data) ;
GPList*   GPList_insert_before           (GPList            *list,
					 GPList            *sibling,
					 void*          data) ;
GPList*   GPList_concat                  (GPList            *list1,
					 GPList            *list2) ;
GPList*   GPList_remove                  (GPList            *list,
					 const void*     data) ;
GPList*   GPList_remove_all              (GPList            *list,
					 const void*     data) ;
GPList*   GPList_remove_link             (GPList            *list,
					 GPList            *llink) ;
GPList*   GPList_delete_link             (GPList            *list,
					 GPList            *link_) ;
GPList*   GPList_reverse                 (GPList            *list) ;
GPList*   GPList_copy                    (GPList            *list) ;
GPList*   GPList_nth                     (GPList            *list,
					 unsigned int             n);
GPList*   GPList_nth_prev                (GPList            *list,
					 unsigned int             n);
GPList*   GPList_find                    (GPList            *list,
					 const void*     data);
GPList*   GPList_find_custom             (GPList            *list,
					 const void*     data,
					 GPCompareFunc      func);
int     GPList_position                (GPList            *list,
					 GPList            *llink);
int     GPList_index                   (GPList            *list,
					 const void*     data);
GPList*   GPList_last                    (GPList            *list);
GPList*   GPList_first                   (GPList            *list);
unsigned int    GPList_length                  (GPList            *list);
void     GPList_foreach                 (GPList            *list,
					 GPFunc             func,
					 void*          user_data);
GPList*   GPList_sort                    (GPList            *list,
					 GPCompareFunc      compare_func) ;
GPList*   GPList_sort_with_data          (GPList            *list,
					 GPCompareDataFunc  compare_func,
					 void*          user_data)  ;
void* GPList_nth_data                (GPList            *list,
					 unsigned int             n);


#define GPList_previous(list)	        ((list) ? (((GPList *)(list))->prev) : NULL)
#define GPList_next(list)	        ((list) ? (((GPList *)(list))->next) : NULL)

#ifdef __cplusplus
}  /* end extern "C" */
#endif

#endif /* GPList_h */

