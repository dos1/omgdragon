/*
 * Copyright (c) Sebastian Krzyszkowiak <dos@dosowisko.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define LIBSUPERDERPY_DATA_TYPE struct CommonResources
#include <libsuperderpy.h>
#include <libwebsockets.h>

struct CommonResources {
		// Fill in with common data accessible from all gamestates.
		bool ws;
		struct lws *ws_socket;
		struct lws_context *ws_context;
		bool ws_connected;
		char* ws_buffer;
};

typedef enum {
	WEBSOCKET_EVENT_INCOMING_MESSAGE = 2048,
	WEBSOCKET_EVENT_CONNECTING,
	WEBSOCKET_EVENT_CONNECTED,
	WEBSOCKET_EVENT_DISCONNECTED,
} WEBSOCKET_EVENT_TYPE;

typedef enum {
	OMGDRAGON_EVENT_UPDATE = 1024,
	OMGDRAGON_EVENT_FIRE,
	OMGDRAGON_EVENT_KILL,
	OMGDRAGON_EVENT_SCORE,
	OMGDRAGON_EVENT_JOIN,
	OMGDRAGON_EVENT_LEAVE,
	OMGDRAGON_EVENT_ID,
	OMGDRAGON_EVENT_RESPAWN
} OMGDRAGON_EVENT_TYPE;


struct CommonResources* CreateGameData(struct Game *game);
void DestroyGameData(struct Game *game, struct CommonResources *data);
void WebSocketConnect(struct Game *game);
void WebSocketDisconnect(struct Game *game);
void WebSocketSend(struct Game *game, char* msg);
bool GlobalEventHandler(struct Game *game, ALLEGRO_EVENT *event);
