/*
  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _TRAYICON_H
#define _TRAYICON_H

#include <windows.h>
#include <stdbool.h>

typedef void (*callback_functionPtr) ();

bool trayicon_init(HICON icon, char tooltip[]);
bool trayicon_change_icon(HICON newicon);
void trayicon_remove();

void trayicon_add_item(char *text, callback_functionPtr functionPtr);

#endif
