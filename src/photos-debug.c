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

#include <config.h>

#include <glib.h>

#include "photos-debug.h"

PhotosDebugFlags _photos_debug_flags;

void
_photos_debug_init(void)
{
#ifdef ENABLE_DEBUG
  const GDebugKey keys[] = {
    { "gegl",    PHOTOS_DEBUG_GEGL    },
    { "tracker", PHOTOS_DEBUG_TRACKER },
    { "dlna",    PHOTOS_DEBUG_DLNA    },
  };

  _photos_debug_flags = g_parse_debug_string (g_getenv ("GNOME_PHOTOS_DEBUG"),
                                              keys, G_N_ELEMENTS (keys));
#endif /* ENABLE_DEBUG */
}
