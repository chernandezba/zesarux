/*
    ZEsarUX  ZX Second-Emulator And Released for UniX
    Copyright (C) 2013 Cesar Hernandez Bano

    This file is part of ZEsarUX.

    ZEsarUX is free software: you can redistribute it and/or modify
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

#ifndef STATS_H
#define STATS_H

extern char stats_uuid[];

extern void generate_stats_uuid(void);

extern z80_bit stats_enabled;
extern z80_bit stats_asked;
extern void send_stats_server(void);
extern int stats_get_current_total_minutes_use(void);

#define REMOTE_ZESARUX_SERVER "51.83.33.13"
#define STATS_URL_UPDATE_STABLE_VERSION "/check_updates/stable.txt"
#define STATS_URL_UPDATE_SNAPSHOT_VERSION "/check_updates/snapshot.txt"
#define STATS_URL_YESTERDAY_USERS "/zesarux_yesterday_users.txt"

#define MAX_UPDATE_VERSION_STRING 1024
#define MAX_YESTERDAY_USERS_STRING 99

extern char stats_last_remote_version[];
extern z80_bit stats_check_updates_enabled;
extern z80_bit stats_check_yesterday_users_enabled;

extern void stats_check_updates(void);
extern void stats_ask_if_enable(void);
extern void stats_enable(void);
extern void stats_disable(void);

extern void stats_check_yesterday_users(void);

extern int stats_total_speccy_browser_queries;
extern int stats_total_zx81_browser_queries;
extern char stats_last_yesterday_users[];

extern int stats_frames_total;
extern int stats_frames_total_drawn;
extern int stats_frames_total_dropped;

#endif
