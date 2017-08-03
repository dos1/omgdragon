/*! \file empty.c
 *  \brief Empty gamestate.
 */
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

#include "../common.h"
#include <math.h>
#include <libsuperderpy.h>

struct GamestateResources {
		// This struct is for every resource allocated and used by your gamestate.
		// It gets created on load and then gets passed around to all other function calls.
		ALLEGRO_FONT *font;
		ALLEGRO_BITMAP *bg;
		int blink_counter;
		struct Character *dragon, *enemy, *explosion;
		bool forward;
		bool left;
		bool right;
		bool killed;

		int id;
		struct {
				float x;
				float y;
				float rot;
				bool killed;
				bool roaring;
				bool active;
				struct Character *explosion;
		} players[256];
};

int Gamestate_ProgressCount = 1; // number of loading steps as reported by Gamestate_Load

void Gamestate_Logic(struct Game *game, struct GamestateResources* data) {
	// Called 60 times per second. Here you should do all your game logic.
	data->blink_counter++;
	if (data->blink_counter >= 60) {
		data->blink_counter = 0;
	}

	if (data->blink_counter%3==0) {
		char buf[256];
		snprintf(buf, 255, "U%d;%d;%d;%d", data->id, (int)(data->dragon->x*1000), (int)(data->dragon->y*1000), (int)(data->dragon->angle*1000));
		if (data->id!=-1) {
			WebSocketSend(game, buf);
		}
	}

	for (int i=0; i<256; i++) {
		AnimateCharacter(game, data->players[i].explosion, 3);
	}
	AnimateCharacter(game, data->explosion, 3);

	AnimateCharacter(game, data->dragon, 1);


	if (data->forward) {
		MoveCharacterF(game, data->dragon, -0.0025 * cos(data->dragon->angle), -0.0025 * sin(data->dragon->angle) * 1.7777, 0);
	}

	if (data->left) {
		MoveCharacterF(game, data->dragon, 0, 0, -0.1);
	}
	if (data->right) {
		MoveCharacterF(game, data->dragon, 0, 0, 0.1);
	}

	if (data->dragon->x < -0.2) {
		SetCharacterPositionF(game, data->dragon, 1, 0.5, 0);
	}
}

void Gamestate_Draw(struct Game *game, struct GamestateResources* data) {
	// Called as soon as possible, but no sooner than next Gamestate_Logic call.
	// Draw everything to the screen here.
	al_draw_bitmap(data->bg, 0, 0, 0);

	for (int i=0; i<256; i++) {
		if (data->players[i].active) {
			SetCharacterPositionF(game, data->enemy, data->players[i].x, data->players[i].y, data->players[i].rot);

			SetCharacterPositionF(game, data->players[i].explosion, data->players[i].x - 0.12 * cos(data->players[i].rot + 0.105), data->players[i].y - 0.15 * sin(data->players[i].rot + 0.105) * 1.77 - 0.06, data->players[i].rot + 0.105);
			DrawCharacter(game, data->players[i].explosion, al_map_rgb(255,255,255), 0);
			DrawCharacter(game, data->enemy, data->players[i].killed ? al_map_rgb(0,0,0) : al_map_rgb(255,255,255), 0);

			//MoveCharacterF(game, data->dragon, -0.0025 * cos(data->dragon->angle), -0.0025 * sin(data->dragon->angle) * 1.7777, 0);

		}
	}

	SetCharacterPositionF(game, data->explosion, data->dragon->x - 0.12 * cos(data->dragon->angle + 0.105), data->dragon->y - 0.15 * sin(data->dragon->angle + 0.105) * 1.77 - 0.06, data->dragon->angle + 0.105);
	DrawCharacter(game, data->explosion, al_map_rgb(255,255,255), 0);
	DrawCharacter(game, data->dragon,data->killed ? al_map_rgb(0,0,0) : al_map_rgb(255,255,255), 0);
}

void Gamestate_ProcessEvent(struct Game *game, struct GamestateResources* data, ALLEGRO_EVENT *ev) {
	// Called for each event in Allegro event queue.
	// Here you can handle user input, expiring timers etc.
	if ((ev->type==ALLEGRO_EVENT_KEY_DOWN) && (ev->keyboard.keycode == ALLEGRO_KEY_ESCAPE)) {
		UnloadCurrentGamestate(game); // mark this gamestate to be stopped and unloaded
		// When there are no active gamestates, the engine will quit.
	}

	if ((ev->type==ALLEGRO_EVENT_KEY_DOWN) && (ev->keyboard.keycode == ALLEGRO_KEY_UP)) {
		data->forward = true;
		SelectSpritesheet(game, data->dragon, "walk");
	}
	if ((ev->type==ALLEGRO_EVENT_KEY_DOWN) && (ev->keyboard.keycode == ALLEGRO_KEY_LEFT)) {
		data->left = true;
	}
	if ((ev->type==ALLEGRO_EVENT_KEY_DOWN) && (ev->keyboard.keycode == ALLEGRO_KEY_RIGHT)) {
		data->right = true;
	}

	if ((ev->type==ALLEGRO_EVENT_KEY_DOWN) && (ev->keyboard.keycode == ALLEGRO_KEY_SPACE)) {
		char buf[256];
		SelectSpritesheet(game, data->dragon, "roar");
		snprintf(buf, 255, "F%d", data->id);
		if (data->id!=-1) {
			WebSocketSend(game, buf);
		}

		SelectSpritesheet(game, data->explosion, "explode");
	}

	if ((ev->type==ALLEGRO_EVENT_KEY_UP) && (ev->keyboard.keycode == ALLEGRO_KEY_UP)) {
		data->forward = false;
		SelectSpritesheet(game, data->dragon, "stand");
	}
	if ((ev->type==ALLEGRO_EVENT_KEY_UP) && (ev->keyboard.keycode == ALLEGRO_KEY_LEFT)) {
		data->left = false;
	}
	if ((ev->type==ALLEGRO_EVENT_KEY_UP) && (ev->keyboard.keycode == ALLEGRO_KEY_RIGHT)) {
		data->right = false;
	}

	if (ev->type == OMGDRAGON_EVENT_UPDATE) {
		data->players[ev->user.data1].active = true;
//		data->players[ev->user.data1].killed = false;
		data->players[ev->user.data1].x = (int)(ev->user.data2) / 1000.0;
		data->players[ev->user.data1].y = (int)(ev->user.data3) / 1000.0;
		data->players[ev->user.data1].rot = (int)(ev->user.data4) / 1000.0;
//		data->players[ev->user.data1].roaring = false;
		PrintConsole(game, "got update id = %d, x = %f, y = %f, rot = %f",
		             ev->user.data1,
		             data->players[ev->user.data1].x,
		    data->players[ev->user.data1].y,
		    data->players[ev->user.data1].rot);
	}
	if (ev->type == OMGDRAGON_EVENT_JOIN) {
		data->players[ev->user.data1].active = true;
		data->players[ev->user.data1].roaring = false;
		data->players[ev->user.data1].killed = false;
		data->players[ev->user.data1].x = -1;
		data->players[ev->user.data1].y = -1;
		data->players[ev->user.data1].rot = 0;
	}
	if (ev->type == OMGDRAGON_EVENT_LEAVE) {
		data->players[ev->user.data1].active = false;
	}
	if (ev->type == OMGDRAGON_EVENT_FIRE) {
		data->players[ev->user.data1].roaring = true;
		SelectSpritesheet(game, data->players[ev->user.data1].explosion, "explode");
	}
	if (ev->type == OMGDRAGON_EVENT_KILL) {
		data->players[ev->user.data2].killed = true;
	}
	if (ev->type == OMGDRAGON_EVENT_RESPAWN) {
		data->players[ev->user.data2].killed = false;
		data->players[ev->user.data1].roaring = false;
		data->players[ev->user.data1].killed = false;
		data->players[ev->user.data1].x = -1;
		data->players[ev->user.data1].y = -1;
		data->players[ev->user.data1].rot = 0;
	}
	if (ev->type == OMGDRAGON_EVENT_ID) {
		data->id = ev->user.data1;
		PrintConsole(game, "got ID: %d", data->id);
	}

}

void* Gamestate_Load(struct Game *game, void (*progress)(struct Game*)) {
	// Called once, when the gamestate library is being loaded.
	// Good place for allocating memory, loading bitmaps etc.
	struct GamestateResources *data = malloc(sizeof(struct GamestateResources));
	data->font = al_create_builtin_font();
	progress(game); // report that we progressed with the loading, so the engine can draw a progress bar

	data->dragon = CreateCharacter(game, "dragon");
	RegisterSpritesheet(game, data->dragon, "walk");
	RegisterSpritesheet(game, data->dragon, "roar");
	RegisterSpritesheet(game, data->dragon, "stand");
	LoadSpritesheets(game, data->dragon);

	data->bg = al_load_bitmap(GetDataFilePath(game, "bg.png"));

	data->enemy = CreateCharacter(game, "dragon");
	data->enemy->shared = true;
	data->enemy->spritesheets = data->dragon->spritesheets;

	data->explosion = CreateCharacter(game, "explosion");
	RegisterSpritesheet(game, data->explosion, "blank");
	RegisterSpritesheet(game, data->explosion, "explode");
	LoadSpritesheets(game, data->explosion);

	for (int i=0; i<256; i++) {
		data->players[i].explosion = CreateCharacter(game, "explosion");
		data->players[i].explosion->shared = true;
		data->players[i].explosion->spritesheets = data->explosion->spritesheets;
	}

	return data;
}

void Gamestate_Unload(struct Game *game, struct GamestateResources* data) {
	// Called when the gamestate library is being unloaded.
	// Good place for freeing all allocated memory and resources.
	al_destroy_font(data->font);
	al_destroy_bitmap(data->bg);
	DestroyCharacter(game, data->dragon);
	DestroyCharacter(game, data->enemy);
	for (int i=0; i<256; i++) {
		DestroyCharacter(game, data->players[i].explosion);
	}
	free(data);
}

void Gamestate_Start(struct Game *game, struct GamestateResources* data) {
	// Called when this gamestate gets control. Good place for initializing state,
	// playing music etc.
	data->blink_counter = 0;
	SelectSpritesheet(game, data->dragon, "stand");
	SelectSpritesheet(game, data->enemy, "stand");
	SetCharacterPositionF(game, data->dragon, (rand()/(float)RAND_MAX) * 0.8, (rand()/(float)RAND_MAX) * 0.8, 0);
	data->forward = false;
	data->left = false;
	data->right = false;
	data->killed = false;
	data->id = -1;

	for (int i=0; i<256; i++) {
		data->players[i].active = false;
		SelectSpritesheet(game, data->players[i].explosion, "blank");
	}
	SelectSpritesheet(game, data->explosion, "blank");

	WebSocketConnect(game);
}

void Gamestate_Stop(struct Game *game, struct GamestateResources* data) {
	// Called when gamestate gets stopped. Stop timers, music etc. here.
	WebSocketDisconnect(game);
}

void Gamestate_Pause(struct Game *game, struct GamestateResources* data) {
	// Called when gamestate gets paused (so only Draw is being called, no Logic not ProcessEvent)
	// Pause your timers here.
}

void Gamestate_Resume(struct Game *game, struct GamestateResources* data) {
	// Called when gamestate gets resumed. Resume your timers here.
}

// Ignore this for now.
// TODO: Check, comment, refine and/or remove:
void Gamestate_Reload(struct Game *game, struct GamestateResources* data) {}
