/*
  host.c - architecture-independent parts of HAL
  Part of Grbl

  Copyright (c) 2012 Radu - Eosif Mihailescu

  Grbl is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Grbl is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Grbl.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "host.h"

THostSettingStatus host_settings_store(const uint16_t signature,
    const void *settings, const size_t size) {
  uint8_t crc = 0, i, *j = (uint8_t *)settings;

  host_nvs_store_word((uint16_t *)0x00, signature);
  host_nvs_store_data((void *)(sizeof(uint16_t)), settings, size);
  for(i = 0; i < size; i++) crc = host_crc8(crc, j[i]);
  host_nvs_store_byte((uint8_t *)(size + sizeof(uint16_t)), crc);

  return HOST_SETTING_OK;
}

THostSettingStatus host_settings_fetch(const uint16_t signature,
    void *settings, const size_t size){
  uint8_t crc = 0, i, *j = (uint8_t *)settings;

  if(host_nvs_fetch_word((uint16_t *)0x00) != signature)
    return HOST_SETTING_NOSIG;
  host_nvs_fetch_data((void *)(sizeof(uint16_t)), settings, size);
  for(i = 0; i < size; i++) crc = host_crc8(crc, j[i]);
  if(host_nvs_fetch_byte((uint8_t *)(size + sizeof(uint16_t))) != crc)
    return HOST_SETTING_NOCRC;

  return HOST_SETTING_OK;
}

bool host_serialconsole_printstring(const char *s, bool block) {
  while(*s) if(!host_serialconsole_write(*s++, block)) return false;

  return true;
}

/* Print a constant string (a message). Depending on architecture and
 * configuration, this may mean said string is coming from a different address
 * space or via a possibly totally different access method than a usual char *.
 * The only specification is that the passed pointer can be transformed into an
 * usual string on a character-by-character basis by calling a HAL function
 * that will fetch it for us.
 */
bool host_serialconsole_printmessage(const char *s, bool block) {
  char c;

  while ((c = host_fetch_S(s++)))
    if(!host_serialconsole_write(c, block)) return false;

  return true;
}
