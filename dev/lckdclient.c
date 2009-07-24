/*
 * lckdclient.c
 * Rudimentary command line interface to the Lockdown protocol
 *
 * Copyright (c) 2008 Jonathan Beck All Rights Reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <readline/readline.h>
#include <readline/history.h>

#include <libiphone/libiphone.h>
#include <libiphone/lockdown.h>

int main(int argc, char *argv[])
{
	lockdownd_client_t client = NULL;
	iphone_device_t phone = NULL;

	iphone_set_debug_level(1);

	if (IPHONE_E_SUCCESS != iphone_get_device(&phone)) {
		printf("No iPhone found, is it plugged in?\n");
		return -1;
	}

	if (IPHONE_E_SUCCESS != lockdownd_new_client(phone, &client)) {
		iphone_free_device(phone);
		return -1;
	}

	char *uid = NULL;
	if (IPHONE_E_SUCCESS == lockdownd_get_device_uid(client, &uid)) {
		printf("DeviceUniqueID : %s\n", uid);
		free(uid);
	}

	using_history();
	int loop = TRUE;
	while (loop) {
		char *cmd = readline("> ");
		if (cmd) {

			gchar **args = g_strsplit(cmd, " ", 0);

			int len = 0;
			if (args) {
				while (*(args + len)) {
					g_strstrip(*(args + len));
					len++;
				}
			}

			if (len > 0) {
				add_history(cmd);
				if (!strcmp(*args, "quit"))
					loop = FALSE;

				if (!strcmp(*args, "get") && len >= 2) {
					plist_t value = NULL;
					if (IPHONE_E_SUCCESS == lockdownd_get_value(client, len == 3 ? *(args + 1):NULL,  len == 3 ? *(args + 2):*(args + 1), &value))
					{
						char *xml = NULL;
						uint32_t length;
						plist_to_xml(value, &xml, &length);
						printf("Success : value = %s\n", xml);
						free(xml);
					}
					else
						printf("Error\n");

					if (value)
						plist_free(value);
				}

				if (!strcmp(*args, "start") && len == 2) {
					int port = 0;
					lockdownd_start_service(client, *(args + 1), &port);
					printf("%i\n", port);
				}
			}
			g_strfreev(args);
		}
		free(cmd);
		cmd = NULL;
	}
	clear_history();
	lockdownd_free_client(client);
	iphone_free_device(phone);

	return 0;
}
