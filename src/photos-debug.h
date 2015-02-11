/*
 * Photos - access, organize and share your photos on GNOME
 * Copyright © 2015 Pranav Kant
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
 *   + gnome-terminal
 */

#ifndef ENABLE_DEBUG_H
#define ENABLE_DEBUG_H

#include <glib.h>

G_BEGIN_DECLS

typedef enum {
  PHOTOS_DEBUG_GEGL     = 1 << 0,
  PHOTOS_DEBUG_TRACKER  = 1 << 1,
  PHOTOS_DEBUG_DLNA     = 1 << 2,
} PhotosDebugFlags;

void _photos_debug_init(void);

extern PhotosDebugFlags _photos_debug_flags;
static inline gboolean _photos_debug_on (PhotosDebugFlags flags) G_GNUC_CONST G_GNUC_UNUSED;

static inline gboolean
_photos_debug_on (PhotosDebugFlags flags)
{
  return (_photos_debug_flags & flags) == flags;
}

#ifdef ENABLE_DEBUG
#define _PHOTOS_DEBUG_IF(flags) if (G_UNLIKELY (_photos_debug_on (flags)))
#else
#define _PHOTOS_DEBUG_IF(flags) if (0)
#endif

#if defined(__GNUC__) && G_HAVE_GNUC_VARARGS
#define _photos_debug_print(flags, fmt, ...) \
  G_STMT_START { _PHOTOS_DEBUG_IF(flags) g_printerr(fmt, ##__VA_ARGS__); } G_STMT_END
#else
#include <stdarg.h>
#include <glib/gstdio.h>
static void _photos_debug_print (guint flags, const char *fmt, ...)
{
  if (_photos_debug_on (flags)) {
    va_list  ap;
    va_start (ap, fmt);
    g_vfprintf (stderr, fmt, ap);
    va_end (ap);
  }
}
#endif

G_END_DECLS

#endif /* !ENABLE_DEBUG_H */
