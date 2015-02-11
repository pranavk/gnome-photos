/*
 * Photos - access, organize and share your photos on GNOME
 * Copyright © 2012, 2013 Red Hat, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

/* Based on code from:
 *   + Documents
 */


#include "config.h"

#include <glib.h>
#include <tracker-sparql.h>

#include "photos-debug.h"
#include "photos-tracker-queue.h"


struct _PhotosTrackerQueuePrivate
{
  GQueue *queue;
  TrackerSparqlConnection *connection;
  gboolean running;
};

static void photos_tracker_queue_initable_iface_init (GInitableIface *iface);


G_DEFINE_TYPE_WITH_CODE (PhotosTrackerQueue, photos_tracker_queue, G_TYPE_OBJECT,
                         G_ADD_PRIVATE (PhotosTrackerQueue)
                         G_IMPLEMENT_INTERFACE (G_TYPE_INITABLE, photos_tracker_queue_initable_iface_init));


typedef enum
{
  PHOTOS_TRACKER_QUERY_SELECT,
  PHOTOS_TRACKER_QUERY_UPDATE,
  PHOTOS_TRACKER_QUERY_UPDATE_BLANK
} PhotosTrackerQueryType;

typedef struct _PhotosTrackerQueueData PhotosTrackerQueueData;

struct _PhotosTrackerQueueData
{
  GAsyncReadyCallback callback;
  GCancellable *cancellable;
  GDestroyNotify destroy_data;
  PhotosTrackerQueryType query_type;
  gchar *sparql;
  gpointer user_data;
};


static void photos_tracker_queue_check (PhotosTrackerQueue *self);


static void
photos_tracker_queue_data_free (PhotosTrackerQueueData *data)
{
  g_clear_object (&data->cancellable);
  g_free (data->sparql);

  if (data->destroy_data != NULL)
    (*data->destroy_data) (data->user_data);

  g_slice_free (PhotosTrackerQueueData, data);
}


static PhotosTrackerQueueData *
photos_tracker_queue_data_new (const gchar *sparql,
                               PhotosTrackerQueryType query_type,
                               GCancellable *cancellable,
                               GAsyncReadyCallback callback,
                               gpointer user_data,
                               GDestroyNotify destroy_data)
{
  PhotosTrackerQueueData *data;

  data = g_slice_new0 (PhotosTrackerQueueData);
  data->sparql = g_strdup (sparql);
  data->query_type = query_type;
  data->cancellable = cancellable;
  data->callback = callback;
  data->user_data = user_data;
  data->destroy_data = destroy_data;

  return data;
}


static void
photos_tracker_queue_collector (GObject *source_object, GAsyncResult *res, gpointer user_data)
{
  PhotosTrackerQueue *self = PHOTOS_TRACKER_QUEUE (user_data);
  PhotosTrackerQueuePrivate *priv = self->priv;
  PhotosTrackerQueueData *data;

  data = g_queue_pop_head (priv->queue);
  if (data->callback != NULL)
    (*data->callback) (source_object, res, data->user_data);
  priv->running = FALSE;
  photos_tracker_queue_data_free (data);

  photos_tracker_queue_check (self);
  g_object_unref (self);
}


static void
photos_tracker_queue_check (PhotosTrackerQueue *self)
{
  PhotosTrackerQueuePrivate *priv = self->priv;
  PhotosTrackerQueueData *data;

  if (priv->running)
    return;

  if (priv->queue->length == 0)
    return;

  data = g_queue_peek_head (priv->queue);
  priv->running = TRUE;

  _photos_debug_print (PHOTOS_DEBUG_TRACKER, "%s", data->sparql);

  switch (data->query_type)
    {
    case PHOTOS_TRACKER_QUERY_SELECT:
      tracker_sparql_connection_query_async (priv->connection,
                                             data->sparql,
                                             data->cancellable,
                                             photos_tracker_queue_collector,
                                             g_object_ref (self));
      break;

    case PHOTOS_TRACKER_QUERY_UPDATE:
      tracker_sparql_connection_update_async (priv->connection,
                                              data->sparql,
                                              G_PRIORITY_DEFAULT,
                                              data->cancellable,
                                              photos_tracker_queue_collector,
                                              g_object_ref (self));
      break;

    case PHOTOS_TRACKER_QUERY_UPDATE_BLANK:
      tracker_sparql_connection_update_blank_async (priv->connection,
                                                    data->sparql,
                                                    G_PRIORITY_DEFAULT,
                                                    data->cancellable,
                                                    photos_tracker_queue_collector,
                                                    g_object_ref (self));
      break;
    }
}


static GObject *
photos_tracker_queue_constructor (GType type, guint n_construct_params, GObjectConstructParam *construct_params)
{
  static GObject *self = NULL;

  if (self == NULL)
    {
      self = G_OBJECT_CLASS (photos_tracker_queue_parent_class)->constructor (type,
                                                                              n_construct_params,
                                                                              construct_params);
      g_object_add_weak_pointer (self, (gpointer) &self);
      return self;
    }

  return g_object_ref (self);
}


static void
photos_tracker_queue_dispose (GObject *object)
{
  PhotosTrackerQueue *self = PHOTOS_TRACKER_QUEUE (object);

  g_clear_object (&self->priv->connection);

  G_OBJECT_CLASS (photos_tracker_queue_parent_class)->dispose (object);
}


static void
photos_tracker_queue_finalize (GObject *object)
{
  PhotosTrackerQueue *self = PHOTOS_TRACKER_QUEUE (object);

  g_queue_free (self->priv->queue);

  G_OBJECT_CLASS (photos_tracker_queue_parent_class)->finalize (object);
}


static void
photos_tracker_queue_init (PhotosTrackerQueue *self)
{
  PhotosTrackerQueuePrivate *priv = self->priv;

  self->priv = photos_tracker_queue_get_instance_private (self);
  priv = self->priv;

  priv->queue = g_queue_new ();
}


static void
photos_tracker_queue_class_init (PhotosTrackerQueueClass *class)
{
  GObjectClass *object_class = G_OBJECT_CLASS (class);

  object_class->constructor = photos_tracker_queue_constructor;
  object_class->dispose = photos_tracker_queue_dispose;
  object_class->finalize = photos_tracker_queue_finalize;
}


static gboolean
photos_tracker_queue_initable_init (GInitable *initable, GCancellable *cancellable, GError **error)
{
  PhotosTrackerQueue *self = PHOTOS_TRACKER_QUEUE (initable);
  PhotosTrackerQueuePrivate *priv = self->priv;
  gboolean ret_val = TRUE;

  if (G_LIKELY (priv->connection != NULL))
    goto out;

  priv->connection = tracker_sparql_connection_get (cancellable, error);
  if (G_UNLIKELY (priv->connection == NULL))
    ret_val = FALSE;

 out:
  return ret_val;
}


static void
photos_tracker_queue_initable_iface_init (GInitableIface *iface)
{
  iface->init = photos_tracker_queue_initable_init;
}


PhotosTrackerQueue *
photos_tracker_queue_dup_singleton (GCancellable *cancellable, GError **error)
{
  return g_initable_new (PHOTOS_TYPE_TRACKER_QUEUE, cancellable, error, NULL);
}


void
photos_tracker_queue_select (PhotosTrackerQueue *self,
                             const gchar *sparql,
                             GCancellable *cancellable,
                             GAsyncReadyCallback callback,
                             gpointer user_data,
                             GDestroyNotify destroy_data)
{
  PhotosTrackerQueueData *data;

  if (cancellable != NULL)
    g_object_ref (cancellable);

  data = photos_tracker_queue_data_new (sparql,
                                        PHOTOS_TRACKER_QUERY_SELECT,
                                        cancellable,
                                        callback,
                                        user_data,
                                        destroy_data);

  g_queue_push_tail (self->priv->queue, data);
  photos_tracker_queue_check (self);
}


void
photos_tracker_queue_update (PhotosTrackerQueue *self,
                             const gchar *sparql,
                             GCancellable *cancellable,
                             GAsyncReadyCallback callback,
                             gpointer user_data,
                             GDestroyNotify destroy_data)
{
  PhotosTrackerQueueData *data;

  if (cancellable != NULL)
    g_object_ref (cancellable);

  data = photos_tracker_queue_data_new (sparql,
                                        PHOTOS_TRACKER_QUERY_UPDATE,
                                        cancellable,
                                        callback,
                                        user_data,
                                        destroy_data);

  g_queue_push_tail (self->priv->queue, data);
  photos_tracker_queue_check (self);
}


void
photos_tracker_queue_update_blank (PhotosTrackerQueue *self,
                                   const gchar *sparql,
                                   GCancellable *cancellable,
                                   GAsyncReadyCallback callback,
                                   gpointer user_data,
                                   GDestroyNotify destroy_data)
{
  PhotosTrackerQueueData *data;

  if (cancellable != NULL)
    g_object_ref (cancellable);

  data = photos_tracker_queue_data_new (sparql,
                                        PHOTOS_TRACKER_QUERY_UPDATE_BLANK,
                                        cancellable,
                                        callback,
                                        user_data,
                                        destroy_data);

  g_queue_push_tail (self->priv->queue, data);
  photos_tracker_queue_check (self);
}
