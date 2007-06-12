/* GLIB - Library of useful routines for C programming
 * Copyright (C) 1995-1997  Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * GPTreeNode: N-way tree implementation.
 * Copyright (C) 1998 Tim Janik
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

#include "GPRuntime.h"
#include "GPTree.h"
#include <stdlib.h>

/* --- functions --- */
GPTreeNode*
GPTree_new (void* data)
{
  GPTreeNode *node = calloc(1,sizeof(GPTreeNode));
  if(!node)
    return NULL;
  node->data = data;
  return node;
}

static void
GPTree_free (GPTreeNode *node)
{
  while (node)
    {
      GPTreeNode *next = node->next;
      if (node->children)
        GPTree_free (node->children);
        free (node);
      node = next;
    }
}

void
GPTree_destroy (GPTreeNode *root)
{
  
  if (!GPTREE_NODE_IS_ROOT (root))
    GPTree_unlink (root);
  
  GPTree_free (root);
}

void
GPTree_unlink (GPTreeNode *node)
{
  if (node->prev)
    node->prev->next = node->next;
  else if (node->parent)
    node->parent->children = node->next;
  node->parent = NULL;
  if (node->next)
    {
      node->next->prev = node->prev;
      node->next = NULL;
    }
  node->prev = NULL;
}

/**
 * GPTree_copy_deep:
 * @node: a #GPTreeNode
 * @copy_func: the function which is called to copy the data inside each node,
 *   or %NULL to use the original data.
 * @data: data to pass to @copy_func
 * 
 * Recursively copies a #GPTreeNode and its data.
 * 
 * Return value: a new #GPTreeNode containing copies of the data in @node.
 *
 * Since: 2.4
 **/
GPTreeNode*
GPTree_copy_deep (GPTreeNode     *node, 
		  GCopyFunc  copy_func,
		  void*   data)
{
  GPTreeNode *new_node = NULL;

  if (copy_func == NULL)
	return GPTree_copy (node);

  if (node)
    {
      GPTreeNode *child, *new_child;
      
      new_node = GPTree_new (copy_func (node->data, data));
      
      for (child = GPTree_last_child (node); child; child = child->prev) 
	{
	  new_child = GPTree_copy_deep (child, copy_func, data);
	  GPTree_prepend (new_node, new_child);
	}
    }
  
  return new_node;
}

GPTreeNode*
GPTree_copy (GPTreeNode *node)
{
  GPTreeNode *new_node = NULL;
  
  if (node)
    {
      GPTreeNode *child;
      
      new_node = GPTree_new (node->data);
      
      for (child = GPTree_last_child (node); child; child = child->prev)
	GPTree_prepend (new_node, GPTree_copy (child));
    }
  
  return new_node;
}

GPTreeNode*
GPTree_insert (GPTreeNode *parent,
	       int   position,
	       GPTreeNode *node)
{
  if (position > 0)
    return GPTree_insert_before (parent,
				 GPTree_nth_child (parent, position),
				 node);
  else if (position == 0)
    return GPTree_prepend (parent, node);
  else /* if (position < 0) */
    return GPTree_append (parent, node);
}

GPTreeNode*
GPTree_insert_before (GPTreeNode *parent,
		      GPTreeNode *sibling,
		      GPTreeNode *node)
{
  node->parent = parent;
  
  if (sibling)
    {
      if (sibling->prev)
	{
	  node->prev = sibling->prev;
	  node->prev->next = node;
	  node->next = sibling;
	  sibling->prev = node;
	}
      else
	{
	  node->parent->children = node;
	  node->next = sibling;
	  sibling->prev = node;
	}
    }
  else
    {
      if (parent->children)
	{
	  sibling = parent->children;
	  while (sibling->next)
	    sibling = sibling->next;
	  node->prev = sibling;
	  sibling->next = node;
	}
      else
	node->parent->children = node;
    }
  return node;
}

GPTreeNode*
GPTree_insert_after (GPTreeNode *parent,
		     GPTreeNode *sibling,
		     GPTreeNode *node)
{
  node->parent = parent;

  if (sibling)
    {
      if (sibling->next)
	{
	  sibling->next->prev = node;
	}
      node->next = sibling->next;
      node->prev = sibling;
      sibling->next = node;
    }
  else
    {
      if (parent->children)
	{
	  node->next = parent->children;
	  parent->children->prev = node;
	}
      parent->children = node;
    }

  return node;
}

GPTreeNode*
GPTree_prepend (GPTreeNode *parent,
		GPTreeNode *node)
{
  return GPTree_insert_before (parent, parent->children, node);
}

GPTreeNode*
GPTree_get_root (GPTreeNode *node)
{
  while (node->parent)
    node = node->parent;
  
  return node;
}

GPBool
GPTree_is_ancestor (GPTreeNode *node,
		    GPTreeNode *descendant)
{
  while (descendant)
    {
      if (descendant->parent == node)
	return TRUE;
      
      descendant = descendant->parent;
    }
  
  return FALSE;
}

/* returns 1 for root, 2 for first level children,
 * 3 for children's children...
 */
unsigned int
GPTree_depth (GPTreeNode *node)
{
  register unsigned int depth = 0;
  
  while (node)
    {
      depth++;
      node = node->parent;
    }
  
  return depth;
}

void
GPTree_reverse_children (GPTreeNode *node)
{
  GPTreeNode *child;
  GPTreeNode *last;
  
  child = node->children;
  last = NULL;
  while (child)
    {
      last = child;
      child = last->next;
      last->next = last->prev;
      last->prev = child;
    }
  node->children = last;
}

unsigned int
GPTree_max_height (GPTreeNode *root)
{
  register GPTreeNode *child;
  register unsigned int max_height = 0;
  
  if (!root)
    return 0;
  
  child = root->children;
  while (child)
    {
      register unsigned int tmp_height;
      
      tmp_height = GPTree_max_height (child);
      if (tmp_height > max_height)
	max_height = tmp_height;
      child = child->next;
    }
  
  return max_height + 1;
}

static GPBool
GPTree_traverse_pre_order (GPTreeNode	    *node,
			   GTraverseFlags    flags,
			   GPTreeNodeTraverseFunc func,
			   void*	     data)
{
  if (node->children)
    {
      GPTreeNode *child;
      
      if ((flags & GPTREE_TRAVERSE_NON_LEAFS) &&
	  func (node, data))
	return TRUE;
      
      child = node->children;
      while (child)
	{
	  register GPTreeNode *current;
	  
	  current = child;
	  child = current->next;
	  if (GPTree_traverse_pre_order (current, flags, func, data))
	    return TRUE;
	}
    }
  else if ((flags & GPTREE_TRAVERSE_LEAFS) &&
	   func (node, data))
    return TRUE;
  
  return FALSE;
}

static GPBool
GPTree_depth_traverse_pre_order (GPTreeNode		  *node,
				 GTraverseFlags	   flags,
				 unsigned int		   depth,
				 GPTreeNodeTraverseFunc func,
				 void*	   data)
{
  if (node->children)
    {
      GPTreeNode *child;
      
      if ((flags & GPTREE_TRAVERSE_NON_LEAFS) &&
	  func (node, data))
	return TRUE;
      
      depth--;
      if (!depth)
	return FALSE;
      
      child = node->children;
      while (child)
	{
	  register GPTreeNode *current;
	  
	  current = child;
	  child = current->next;
	  if (GPTree_depth_traverse_pre_order (current, flags, depth, func, data))
	    return TRUE;
	}
    }
  else if ((flags & GPTREE_TRAVERSE_LEAFS) &&
	   func (node, data))
    return TRUE;
  
  return FALSE;
}

static GPBool
GPTree_traverse_post_order (GPTreeNode	     *node,
			    GTraverseFlags    flags,
			    GPTreeNodeTraverseFunc func,
			    void*	      data)
{
  if (node->children)
    {
      GPTreeNode *child;
      
      child = node->children;
      while (child)
	{
	  register GPTreeNode *current;
	  
	  current = child;
	  child = current->next;
	  if (GPTree_traverse_post_order (current, flags, func, data))
	    return TRUE;
	}
      
      if ((flags & GPTREE_TRAVERSE_NON_LEAFS) &&
	  func (node, data))
	return TRUE;
      
    }
  else if ((flags & GPTREE_TRAVERSE_LEAFS) &&
	   func (node, data))
    return TRUE;
  
  return FALSE;
}

static GPBool
GPTree_depth_traverse_post_order (GPTreeNode		   *node,
				  GTraverseFlags    flags,
				  unsigned int		    depth,
				  GPTreeNodeTraverseFunc func,
				  void*	    data)
{
  if (node->children)
    {
      depth--;
      if (depth)
	{
	  GPTreeNode *child;
	  
	  child = node->children;
	  while (child)
	    {
	      register GPTreeNode *current;
	      
	      current = child;
	      child = current->next;
	      if (GPTree_depth_traverse_post_order (current, flags, depth, func, data))
		return TRUE;
	    }
	}
      
      if ((flags & GPTREE_TRAVERSE_NON_LEAFS) &&
	  func (node, data))
	return TRUE;
      
    }
  else if ((flags & GPTREE_TRAVERSE_LEAFS) &&
	   func (node, data))
    return TRUE;
  
  return FALSE;
}

static GPBool
GPTree_traverse_in_order (GPTreeNode		   *node,
			  GTraverseFlags    flags,
			  GPTreeNodeTraverseFunc func,
			  void*	    data)
{
  if (node->children)
    {
      GPTreeNode *child;
      register GPTreeNode *current;
      
      child = node->children;
      current = child;
      child = current->next;
      
      if (GPTree_traverse_in_order (current, flags, func, data))
	return TRUE;
      
      if ((flags & GPTREE_TRAVERSE_NON_LEAFS) &&
	  func (node, data))
	return TRUE;
      
      while (child)
	{
	  current = child;
	  child = current->next;
	  if (GPTree_traverse_in_order (current, flags, func, data))
	    return TRUE;
	}
    }
  else if ((flags & GPTREE_TRAVERSE_LEAFS) &&
	   func (node, data))
    return TRUE;
  
  return FALSE;
}

static GPBool
GPTree_depth_traverse_in_order (GPTreeNode		 *node,
				GTraverseFlags	  flags,
				unsigned int		  depth,
				GPTreeNodeTraverseFunc func,
				void*	  data)
{
  if (node->children)
    {
      depth--;
      if (depth)
	{
	  GPTreeNode *child;
	  register GPTreeNode *current;
	  
	  child = node->children;
	  current = child;
	  child = current->next;
	  
	  if (GPTree_depth_traverse_in_order (current, flags, depth, func, data))
	    return TRUE;
	  
	  if ((flags & GPTREE_TRAVERSE_NON_LEAFS) &&
	      func (node, data))
	    return TRUE;
	  
	  while (child)
	    {
	      current = child;
	      child = current->next;
	      if (GPTree_depth_traverse_in_order (current, flags, depth, func, data))
		return TRUE;
	    }
	}
      else if ((flags & GPTREE_TRAVERSE_NON_LEAFS) &&
	       func (node, data))
	return TRUE;
    }
  else if ((flags & GPTREE_TRAVERSE_LEAFS) &&
	   func (node, data))
    return TRUE;
  
  return FALSE;
}

static GPBool
GPTree_traverse_level (GPTreeNode		 *node,
		       GTraverseFlags	  flags,
		       unsigned int		  level,
		       GPTreeNodeTraverseFunc func,
		       void*	  data,
		       GPBool         *more_levels)
{
  if (level == 0) 
    {
      if (node->children)
	{
	  *more_levels = TRUE;
	  return (flags & GPTREE_TRAVERSE_NON_LEAFS) && func (node, data);
	}
      else
	{
	  return (flags & GPTREE_TRAVERSE_LEAFS) && func (node, data);
	}
    }
  else 
    {
      node = node->children;
      
      while (node)
	{
	  if (GPTree_traverse_level (node, flags, level - 1, func, data, more_levels))
	    return TRUE;

	  node = node->next;
	}
    }

  return FALSE;
}

static GPBool
GPTree_depth_traverse_level (GPTreeNode		 *node,
			     GTraverseFlags	  flags,
			     unsigned int		  depth,
			     GPTreeNodeTraverseFunc func,
			     void*	  data)
{
  unsigned int level;
  GPBool more_levels;

  level = 0;  
  while (level != depth) 
    {
      more_levels = FALSE;
      if (GPTree_traverse_level (node, flags, level, func, data, &more_levels))
	return TRUE;
      if (!more_levels)
	break;
      level++;
    }
  return FALSE;
}

void
GPTree_traverse (GPTreeNode		  *root,
		 GTraverseType	   order,
		 GTraverseFlags	   flags,
		 int		   depth,
		 GPTreeNodeTraverseFunc func,
		 void*	   data)
{
  switch (order)
    {
    case GPTREE_PRE_ORDER:
      if (depth < 0)
	GPTree_traverse_pre_order (root, flags, func, data);
      else
	GPTree_depth_traverse_pre_order (root, flags, depth, func, data);
      break;
    case GPTREE_POST_ORDER:
      if (depth < 0)
	GPTree_traverse_post_order (root, flags, func, data);
      else
	GPTree_depth_traverse_post_order (root, flags, depth, func, data);
      break;
    case GPTREE_IN_ORDER:
      if (depth < 0)
	GPTree_traverse_in_order (root, flags, func, data);
      else
	GPTree_depth_traverse_in_order (root, flags, depth, func, data);
      break;
    case GPTREE_LEVEL_ORDER:
      GPTree_depth_traverse_level (root, flags, depth, func, data);
      break;
    }
}

static GPBool
GPTree_find_func (GPTreeNode	  *node,
		  void* data)
{
  register void* *d = data;
  
  if (*d != node->data)
    return FALSE;
  
  *(++d) = node;
}

GPTreeNode*
GPTree_find (GPTreeNode	       *root,
	     GTraverseType	order,
	     GTraverseFlags	flags,
	     void*		data)
{
  void* d[2];
  d[0] = data;
  d[1] = NULL;
  
  GPTree_traverse (root, order, flags, -1, GPTree_find_func, d);
  
  return d[1];
}

static void
GPTree_count_func (GPTreeNode	 *node,
		   GTraverseFlags flags,
		   unsigned int	 *n)
{
  if (node->children)
    {
      GPTreeNode *child;
      
      if (flags & GPTREE_TRAVERSE_NON_LEAFS)
	(*n)++;
      
      child = node->children;
      while (child)
	{
	  GPTree_count_func (child, flags, n);
	  child = child->next;
	}
    }
  else if (flags & GPTREE_TRAVERSE_LEAFS)
    (*n)++;
}

unsigned int
GPTree_n_nodes (GPTreeNode	      *root,
		GTraverseFlags flags)
{
  unsigned int n = 0;
  GPTree_count_func (root, flags, &n);
  
  return n;
}

GPTreeNode*
GPTree_last_child (GPTreeNode *node)
{
  node = node->children;
  if (node)
    while (node->next)
      node = node->next;
  
  return node;
}

GPTreeNode*
GPTree_nth_child (GPTreeNode *node,
		  unsigned int	 n)
{
  node = node->children;
  if (node)
    while ((n-- > 0) && node)
      node = node->next;
  
  return node;
}

unsigned int
GPTree_n_children (GPTreeNode *node)
{
  unsigned int n = 0;
  node = node->children;
  while (node)
    {
      n++;
      node = node->next;
    }
  
  return n;
}

GPTreeNode*
GPTree_find_child (GPTreeNode	 *node,
		   GTraverseFlags flags,
		   void*	  data)
{
  node = node->children;
  while (node)
    {
      if (node->data == data)
	{
	  if (GPTREE_NODE_IS_LEAF (node))
	    {
	      if (flags & GPTREE_TRAVERSE_LEAFS)
		return node;
	    }
	  else
	    {
	      if (flags & GPTREE_TRAVERSE_NON_LEAFS)
		return node;
	    }
	}
      node = node->next;
    }
}

int
GPTree_child_position (GPTreeNode *node,
		       GPTreeNode *child)
{
  register unsigned int n = 0;
  
  node = node->children;
  while (node)
    {
      if (node == child)
	return n;
      n++;
      node = node->next;
    }
  
  return -1;
}

int
GPTree_child_index (GPTreeNode   *node,
		    void* data)
{
  register unsigned int n = 0;
  
  node = node->children;
  while (node)
    {
      if (node->data == data)
	return n;
      n++;
      node = node->next;
    }
  return -1;
}

GPTreeNode*
GPTree_first_sibling (GPTreeNode *node)
{
  if (node->parent)
    return node->parent->children;
  
  while (node->prev)
    node = node->prev;
  
  return node;
}

GPTreeNode*
GPTree_last_sibling (GPTreeNode *node)
{
  while (node->next)
    node = node->next;
  
  return node;
}

void
GPTree_children_foreach (GPTreeNode		 *node,
			 GTraverseFlags	  flags,
			 GPTreeNodeForeachFunc func,
			 void*	  data)
{
  
  node = node->children;
  while (node)
    {
      register GPTreeNode *current;
      
      current = node;
      node = current->next;
      if (GPTREE_NODE_IS_LEAF (current))
	{
	  if (flags & GPTREE_TRAVERSE_LEAFS)
	    func (current, data);
	}
      else
	{
	  if (flags & GPTREE_TRAVERSE_NON_LEAFS)
	    func (current, data);
	}
    }
}

