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

#ifndef GPTree_h
#define GPTree_h

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _GPTreeNode	GPTreeNode;

/* Tree traverse flags */
typedef enum
{
  GPTREE_TRAVERSE_LEAVES     = 1 << 0,
  GPTREE_TRAVERSE_NON_LEAVES = 1 << 1,
  GPTREE_TRAVERSE_ALL        = GPTREE_TRAVERSE_LEAVES | GPTREE_TRAVERSE_NON_LEAVES,
  GPTREE_TRAVERSE_MASK       = 0x03,
  GPTREE_TRAVERSE_LEAFS      = GPTREE_TRAVERSE_LEAVES,
  GPTREE_TRAVERSE_NON_LEAFS  = GPTREE_TRAVERSE_NON_LEAVES
} GTraverseFlags;

/* Tree traverse orders */
typedef enum
{
  GPTREE_IN_ORDER,
  GPTREE_PRE_ORDER,
  GPTREE_POST_ORDER,
  GPTREE_LEVEL_ORDER
} GTraverseType;

typedef GPBool	(*GPTreeNodeTraverseFunc)	(GPTreeNode	       *node,
						 void*	data);
typedef void		(*GPTreeNodeForeachFunc)	(GPTreeNode	       *node,
						 void*	data);
typedef void*	(*GCopyFunc)            (const void*  src,
                                                 void*       data);

#define GPTreeNodeDef "gp.GPTreeNode (\
    gp.GPTreeNode.next (gp.GPTreeNode*) \
    gp.GPTreeNode.prev (gp.GPTreeNode*) \
    gp.GPTreeNode.parent (gp.GPTreeNode*) \
    gp.GPTreeNode.children (gp.GPTreeNode*)\
    gp.GPTreeNode.data (void*))"

/* N-way tree implementation
 */
struct _GPTreeNode
{
  GPTreeNode	  *next;
  GPTreeNode	  *prev;
  GPTreeNode	  *parent;
  GPTreeNode	  *children;
  void            *data;                 
};

#define	 GPTREE_NODE_IS_ROOT(node)	(((GPTreeNode*) (node))->parent == NULL && \
				 ((GPTreeNode*) (node))->prev == NULL && \
				 ((GPTreeNode*) (node))->next == NULL)
#define	 GPTREE_NODE_IS_LEAF(node)	(((GPTreeNode*) (node))->children == NULL)


GPTreeNode*	 GPTree_new		(void*	   data);
void	 GPTree_destroy		(GPTreeNode		  *root);
void	 GPTree_unlink		(GPTreeNode		  *node);
//GPTreeNode*   GPTree_copy_deep       (GPTreeNode            *node,
//				 GCopyFunc         copy_func,
//				 void*          data);
GPTreeNode*   GPTree_copy            (GPTreeNode            *node);
GPTreeNode*	 GPTree_insert		(GPTreeNode		  *parent,
				 int		   position,
				 GPTreeNode		  *node);
GPTreeNode*	 GPTree_insert_before	(GPTreeNode		  *parent,
				 GPTreeNode		  *sibling,
				 GPTreeNode		  *node);
GPTreeNode*   GPTree_insert_after    (GPTreeNode            *parent,
				 GPTreeNode            *sibling,
				 GPTreeNode            *node); 
GPTreeNode*	 GPTree_prepend		(GPTreeNode		  *parent,
				 GPTreeNode		  *node);
unsigned int	 GPTree_n_nodes		(GPTreeNode		  *root,
				 GTraverseFlags	   flags);
GPTreeNode*	 GPTree_get_root	(GPTreeNode		  *node);
GPBool GPTree_is_ancestor	(GPTreeNode		  *node,
				 GPTreeNode		  *descendant);
unsigned int	 GPTree_depth		(GPTreeNode		  *node);
GPTreeNode*	 GPTree_find		(GPTreeNode		  *root,
				 GTraverseType	   order,
				 GTraverseFlags	   flags,
				 void*	   data);

/* convenience macros */
#define GPTree_append(parent, node)				\
     GPTree_insert_before ((parent), NULL, (node))
#define	GPTree_insert_data(parent, position, data)		\
     GPTree_insert ((parent), (position), GPTree_new (data))
#define	GPTree_insert_data_before(parent, sibling, data)	\
     GPTree_insert_before ((parent), (sibling), GPTree_new (data))
#define	GPTree_prepend_data(parent, data)			\
     GPTree_prepend ((parent), GPTree_new (data))
#define	GPTree_append_data(parent, data)			\
     GPTree_insert_before ((parent), NULL, GPTree_new (data))

/* traversal function, assumes that `node' is root
 * (only traverses `node' and its subtree).
 * this function is just a high level interface to
 * low level traversal functions, optimized for speed.
 */
void	 GPTree_traverse	(GPTreeNode		  *root,
				 GTraverseType	   order,
				 GTraverseFlags	   flags,
				 int		   max_depth,
				 GPTreeNodeTraverseFunc func,
				 void*	   data);

/* return the maximum tree height starting with `node', this is an expensive
 * operation, since we need to visit all nodes. this could be shortened by
 * adding `unsigned int height' to struct _GPTreeNode, but then again, this is not very
 * often needed, and would make GPTree_insert() more time consuming.
 */
unsigned int	 GPTree_max_height	 (GPTreeNode *root);

void	 GPTree_children_foreach (GPTreeNode		  *node,
				  GTraverseFlags   flags,
				  GPTreeNodeForeachFunc func,
				  void*	   data);
void	 GPTree_reverse_children (GPTreeNode		  *node);
unsigned int	 GPTree_n_children	 (GPTreeNode		  *node);
GPTreeNode*	 GPTree_nth_child	 (GPTreeNode		  *node,
				  unsigned int		   n);
GPTreeNode*	 GPTree_last_child	 (GPTreeNode		  *node);
GPTreeNode*	 GPTree_find_child	 (GPTreeNode		  *node,
				  GTraverseFlags   flags,
				  void*	   data);
int	 GPTree_child_position	 (GPTreeNode		  *node,
				  GPTreeNode		  *child);
int	 GPTree_child_index	 (GPTreeNode		  *node,
				  void*	   data);

GPTreeNode*	 GPTree_first_sibling	 (GPTreeNode		  *node);
GPTreeNode*	 GPTree_last_sibling	 (GPTreeNode		  *node);

#define	 GPTree_prev_sibling(node)	((node) ? \
					 ((GPTreeNode*) (node))->prev : NULL)
#define	 GPTree_next_sibling(node)	((node) ? \
					 ((GPTreeNode*) (node))->next : NULL)
#define	 GPTree_first_child(node)	((node) ? \
					 ((GPTreeNode*) (node))->children : NULL)

#ifdef __cplusplus
}  /* end extern "C" */
#endif


#endif /* GPTree_h */
