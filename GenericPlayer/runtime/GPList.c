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

/* 
 * MT safe
 */

#include "GPList.h"
#include <stdlib.h>


GPList* GPList_alloc (void)
{
  return calloc(1,sizeof(GPList));
}

void
GPList_free (GPList *list)
{
  GPList* next = list;
  while(next) {
    GPList* tmp =next;
    next = next->next;
    /*FIXME: data see glib*/
    free(tmp);
  }
}

GPList*
GPList_append (GPList	*list,
	       void*	 data)
{
  GPList *new_list;
  GPList *last;
  
  new_list = GPList_alloc ();
  new_list->data = data;
  new_list->next = NULL;
  
  if (list)
    {
      last = GPList_last (list);
      last->next = new_list;
      new_list->prev = last;

      return list;
    }
  else
    {
      new_list->prev = NULL;
      return new_list;
    }
}

GPList*
GPList_prepend (GPList	 *list,
		void*  data)
{
  GPList *new_list;
  
  new_list = GPList_alloc ();
  new_list->data = data;
  new_list->next = list;
  
  if (list)
    {
      new_list->prev = list->prev;
      if (list->prev)
	list->prev->next = new_list;
      list->prev = new_list;
    }
  else
    new_list->prev = NULL;
  
  return new_list;
}

GPList*
GPList_insert (GPList	*list,
	       void*	 data,
	       int	 position)
{
  GPList *new_list;
  GPList *tmp_list;
  
  if (position < 0)
    return GPList_append (list, data);
  else if (position == 0)
    return GPList_prepend (list, data);
  
  tmp_list = GPList_nth (list, position);
  if (!tmp_list)
    return GPList_append (list, data);
  
  new_list = GPList_alloc ();
  new_list->data = data;
  new_list->prev = tmp_list->prev;
  if (tmp_list->prev)
    tmp_list->prev->next = new_list;
  new_list->next = tmp_list;
  tmp_list->prev = new_list;
  
  if (tmp_list == list)
    return new_list;
  else
    return list;
}

GPList*
GPList_insert_before (GPList   *list,
		      GPList   *sibling,
		      void* data)
{
  if (!list)
    {
      list = GPList_alloc ();
      list->data = data;
      return list;
    }
  else if (sibling)
    {
      GPList *node;

      node = GPList_alloc ();
      node->data = data;
      node->prev = sibling->prev;
      node->next = sibling;
      sibling->prev = node;
      if (node->prev)
	{
	  node->prev->next = node;
	  return list;
	}
      else
	{
	  return node;
	}
    }
  else
    {
      GPList *last;

      last = list;
      while (last->next)
	last = last->next;

      last->next = GPList_alloc ();
      last->next->data = data;
      last->next->prev = last;
      last->next->next = NULL;

      return list;
    }
}

GPList *
GPList_concat (GPList *list1, GPList *list2)
{
  GPList *tmp_list;
  
  if (list2)
    {
      tmp_list = GPList_last (list1);
      if (tmp_list)
	tmp_list->next = list2;
      else
	list1 = list2;
      list2->prev = tmp_list;
    }
  
  return list1;
}

GPList*
GPList_remove (GPList	     *list,
	       const void*  data)
{
  GPList *tmp;
  
  tmp = list;
  while (tmp)
    {
      if (tmp->data != data)
	tmp = tmp->next;
      else
	{
	  if (tmp->prev)
	    tmp->prev->next = tmp->next;
	  if (tmp->next)
	    tmp->next->prev = tmp->prev;
	  
	  if (list == tmp)
	    list = list->next;
	  
	     free (tmp);
	  
	  break;
	}
    }
  return list;
}

GPList*
GPList_remove_all (GPList	*list,
		   const void* data)
{
  GPList *tmp = list;

  while (tmp)
    {
      if (tmp->data != data)
	tmp = tmp->next;
      else
	{
	  GPList *next = tmp->next;

	  if (tmp->prev)
	    tmp->prev->next = next;
	  else
	    list = next;
	  if (next)
	    next->prev = tmp->prev;

	  free (tmp);
	  tmp = next;
	}
    }
  return list;
}

static inline GPList*
_GPList_remove_link (GPList *list,
		     GPList *link)
{
  if (link)
    {
      if (link->prev)
	link->prev->next = link->next;
      if (link->next)
	link->next->prev = link->prev;
      
      if (link == list)
	list = list->next;
      
      link->next = NULL;
      link->prev = NULL;
    }
  
  return list;
}

GPList*
GPList_remove_link (GPList *list,
		    GPList *link)
{
  return _GPList_remove_link (list, link);
}

GPList*
GPList_delete_link (GPList *list,
		    GPList *link)
{
  list = _GPList_remove_link (list, link);
  free (link);

  return list;
}

GPList*
GPList_copy (GPList *list)
{
  GPList *new_list = NULL;

  if (list)
    {
      GPList *last;

      new_list = GPList_alloc ();
      new_list->data = list->data;
      new_list->prev = NULL;
      last = new_list;
      list = list->next;
      while (list)
	{
	  last->next = GPList_alloc ();
	  last->next->prev = last;
	  last = last->next;
	  last->data = list->data;
	  list = list->next;
	}
      last->next = NULL;
    }

  return new_list;
}

GPList*
GPList_reverse (GPList *list)
{
  GPList *last;
  
  last = NULL;
  while (list)
    {
      last = list;
      list = last->next;
      last->next = last->prev;
      last->prev = list;
    }
  
  return last;
}

GPList*
GPList_nth (GPList *list,
	    unsigned int  n)
{
  while ((n-- > 0) && list)
    list = list->next;
  
  return list;
}

GPList*
GPList_nth_prev (GPList *list,
		 unsigned int  n)
{
  while ((n-- > 0) && list)
    list = list->prev;
  
  return list;
}

void*
GPList_nth_data (GPList     *list,
		 unsigned int      n)
{
  while ((n-- > 0) && list)
    list = list->next;
  
  return list ? list->data : NULL;
}

GPList*
GPList_find (GPList         *list,
	     const void*  data)
{
  while (list)
    {
      if (list->data == data)
	break;
      list = list->next;
    }
  
  return list;
}

GPList*
GPList_find_custom (GPList         *list,
		    const void*  data,
		    GPCompareFunc   func)
{

  while (list)
    {
      if (! func (list->data, data))
	return list;
      list = list->next;
    }

  return NULL;
}


int
GPList_position (GPList *list,
		 GPList *link)
{
  int i;

  i = 0;
  while (list)
    {
      if (list == link)
	return i;
      i++;
      list = list->next;
    }

  return -1;
}

int
GPList_index (GPList         *list,
	      const void*  data)
{
  int i;

  i = 0;
  while (list)
    {
      if (list->data == data)
	return i;
      i++;
      list = list->next;
    }

  return -1;
}

GPList*
GPList_last (GPList *list)
{
  if (list)
    {
      while (list->next)
	list = list->next;
    }
  
  return list;
}

GPList*
GPList_first (GPList *list)
{
  if (list)
    {
      while (list->prev)
	list = list->prev;
    }
  
  return list;
}

unsigned int
GPList_length (GPList *list)
{
  unsigned int length;
  
  length = 0;
  while (list)
    {
      length++;
      list = list->next;
    }
  
  return length;
}

void
GPList_foreach (GPList	 *list,
		GPFunc	  func,
		void*  user_data)
{
  while (list)
    {
      GPList *next = list->next;
      (*func) (list->data, user_data);
      list = next;
    }
}

static GPList*
GPList_insert_sorted_real (GPList    *list,
			   void*  data,
			   GPCompareFunc     func,
			   void*  user_data)
{
  GPList *tmp_list = list;
  GPList *new_list;
  int cmp;

  
  if (!list) 
    {
      new_list = GPList_alloc ();
      new_list->data = data;
      return new_list;
    }
  
  cmp = ((GPCompareDataFunc) func) (data, tmp_list->data, user_data);

  while ((tmp_list->next) && (cmp > 0))
    {
      tmp_list = tmp_list->next;

      cmp = ((GPCompareDataFunc) func) (data, tmp_list->data, user_data);
    }

  new_list = GPList_alloc ();
  new_list->data = data;

  if ((!tmp_list->next) && (cmp > 0))
    {
      tmp_list->next = new_list;
      new_list->prev = tmp_list;
      return list;
    }
   
  if (tmp_list->prev)
    {
      tmp_list->prev->next = new_list;
      new_list->prev = tmp_list->prev;
    }
  new_list->next = tmp_list;
  tmp_list->prev = new_list;
 
  if (tmp_list == list)
    return new_list;
  else
    return list;
}

GPList* GPList_insert_sorted (GPList* list, void* data, GPCompareFunc func)
{
  return GPList_insert_sorted_real (list, data, (GPCompareFunc) func, NULL);
}

GPList*
GPList_insert_sorted_with_data (GPList            *list,
				void*          data,
				GPCompareDataFunc  func,
				void*          user_data)
{
  return GPList_insert_sorted_real (list, data, (GPCompareFunc) func, user_data);
}

static GPList *
GPList_sort_merge (GPList     *l1, 
		   GPList     *l2,
		   GPCompareFunc     compare_func,
		   void*  user_data)
{
  GPList list, *l, *lprev;
  int cmp;

  l = &list; 
  lprev = NULL;

  while (l1 && l2)
    {
      cmp = ((GPCompareDataFunc) compare_func) (l1->data, l2->data, user_data);

      if (cmp <= 0)
        {
	  l->next = l1;
	  l1 = l1->next;
        } 
      else 
	{
	  l->next = l2;
	  l2 = l2->next;
        }
      l = l->next;
      l->prev = lprev; 
      lprev = l;
    }
  l->next = l1 ? l1 : l2;
  l->next->prev = l;

  return list.next;
}

static GPList* 
GPList_sort_real (GPList    *list,
		  GPCompareFunc     compare_func,
		  void*  user_data)
{
  GPList *l1, *l2;
  
  if (!list) 
    return NULL;
  if (!list->next) 
    return list;
  
  l1 = list; 
  l2 = list->next;

  while ((l2 = l2->next) != NULL)
    {
      if ((l2 = l2->next) == NULL) 
	break;
      l1 = l1->next;
    }
  l2 = l1->next; 
  l1->next = NULL; 

  return GPList_sort_merge (GPList_sort_real (list, compare_func, user_data),
			    GPList_sort_real (l2, compare_func, user_data),
			    compare_func,
			    user_data);
}

GPList *
GPList_sort (GPList        *list,
	     GPCompareFunc  compare_func)
{
  return GPList_sort_real (list, (GPCompareFunc) compare_func, NULL);
			    
}

GPList *
GPList_sort_with_data (GPList            *list,
		       GPCompareDataFunc  compare_func,
		       void*          user_data)
{
  return GPList_sort_real (list, (GPCompareFunc) compare_func, user_data);
}

