/*
    edge.c -- edge tree management
    Copyright (C) 2000-2002 Guus Sliepen <guus@sliepen.eu.org>,
                  2000-2002 Ivo Timmermans <ivo@o2w.nl>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    $Id: edge.c,v 1.1.2.12 2002/09/04 13:48:51 guus Exp $
*/

#include "config.h"

#include <stdio.h>
#include <syslog.h>
#include <string.h>

#include <avl_tree.h>
#include <list.h>

#include "net.h"	/* Don't ask. */
#include "netutl.h"
#include "config.h"
#include "conf.h"
#include <utils.h>
#include "subnet.h"
#include "edge.h"
#include "node.h"

#include "xalloc.h"
#include "system.h"

avl_tree_t *edge_tree;        /* Tree with all known edges (replaces active_tree) */
avl_tree_t *edge_weight_tree; /* Tree with all edges, sorted on weight */

int edge_compare(edge_t *a, edge_t *b)
{
  int result;

  result = strcmp(a->from->name, b->from->name);

  if(result)
    return result;
  else
    return strcmp(a->to->name, b->to->name);
}

/* Evil edge_compare() from a parallel universe ;)

int edge_compare(edge_t *a, edge_t *b)
{
  int result;

  return (result = strcmp(a->from->name, b->from->name)) || (result = strcmp(a->to->name, b->to->name)), result;
}

*/

int edge_weight_compare(edge_t *a, edge_t *b)
{
  int result;

  result = a->weight - b->weight;

  if(result)
    return result;
  else
    return edge_compare(a, b);
}

void init_edges(void)
{
cp
  edge_tree = avl_alloc_tree((avl_compare_t)edge_compare, NULL);
  edge_weight_tree = avl_alloc_tree((avl_compare_t)edge_weight_compare, NULL);
cp
}

avl_tree_t *new_edge_tree(void)
{
cp
  return avl_alloc_tree((avl_compare_t)edge_compare, NULL);
cp
}

void free_edge_tree(avl_tree_t *edge_tree)
{
cp
  avl_delete_tree(edge_tree);
cp
}

void exit_edges(void)
{
cp
  avl_delete_tree(edge_tree);
cp
}

/* Creation and deletion of connection elements */

edge_t *new_edge(void)
{
  edge_t *e;
cp
  e = (edge_t *)xmalloc_and_zero(sizeof(*e));
cp
  return e;
}

void free_edge(edge_t *e)
{
cp
  free(e);
cp
}

void edge_add(edge_t *e)
{
cp
  avl_insert(edge_tree, e);
  avl_insert(edge_weight_tree, e);
  avl_insert(e->from->edge_tree, e);
cp
  e->reverse = lookup_edge(e->to, e->from);
  if(e->reverse)
    e->reverse->reverse = e;
cp
}

void edge_del(edge_t *e)
{
cp
  if(e->reverse)
    e->reverse->reverse = NULL;
cp
  avl_delete(edge_tree, e);
  avl_delete(edge_weight_tree, e);
  avl_delete(e->from->edge_tree, e);
cp
}

edge_t *lookup_edge(node_t *from, node_t *to)
{
  edge_t v;
cp
  v.from = from;
  v.to = to;

  return avl_search(edge_tree, &v);
}

void dump_edges(void)
{
  avl_node_t *node;
  edge_t *e;
  char *address;
cp
  syslog(LOG_DEBUG, _("Edges:"));

  for(node = edge_tree->head; node; node = node->next)
    {
      e = (edge_t *)node->data;
      address = sockaddr2hostname(&e->address);
      syslog(LOG_DEBUG, _(" %s to %s at %s options %lx weight %d"),
             e->from->name, e->to->name, address,
	     e->options, e->weight);
      free(address);
    }

  syslog(LOG_DEBUG, _("End of edges."));
cp
}