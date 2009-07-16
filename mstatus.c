#define PURPLE_PLUGINS

#include <glib.h>
#include <glib/gprintf.h>
#include <string.h>
#include <cmds.h>
#include <conversation.h>
#include <dbus/dbus-glib.h>

#include "plugin.h"
#include "version.h"
#include <assert.h>

#define PLUGIN_ID "core-mStatus"
#define PREF_LOG TRUE
#define STRLEN 100

#define STATUS_OFF 0
#define STATUS_PAUSED 1
#define STATUS_NORMAL 2

#define INTERVAL 10000
#define DBUS_TIMEOUT 100

#define STRLEN 100

struct TrackInfo
{
	char track[STRLEN];
	char artist[STRLEN];
	char album[STRLEN];
	int status;
	int totalSecs;
	int currentSecs;
};

gboolean GHashStr(GHashTable *table, const char *key, char *dest) {
	GValue* value = (GValue*) g_hash_table_lookup(table, key);
	if (value != NULL && G_VALUE_HOLDS_STRING(value)) {
		strncpy(dest, g_value_get_string(value), STRLEN-1);
		return TRUE;
	}
		return FALSE;
}

unsigned int
GHashUnit(GHashTable *table, const char *key)
{
	GValue* value = (GValue*) g_hash_table_lookup(table, key);
	if (value != NULL && G_VALUE_HOLDS_UINT(value)) {
		return g_value_get_uint(value);
	}
	return 0;
}

gboolean
GRhInfo(struct TrackInfo* ti)
{
	DBusGConnection *connection;
	DBusGProxy *player, *shell;
	GError *error = 0;

	connection = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
	if (connection == NULL) {
		g_error_free (error);
		return FALSE;
	}

	shell = dbus_g_proxy_new_for_name(connection,
			"org.gnome.Rhythmbox",
			"/org/gnome/Rhythmbox/Shell",
			"org.gnome.Rhythmbox.Shell");
	player = dbus_g_proxy_new_for_name(connection,
			"org.gnome.Rhythmbox",
			"/org/gnome/Rhythmbox/Player",
			"org.gnome.Rhythmbox.Player");

	gboolean playing;
	if (!dbus_g_proxy_call_with_timeout(player, "getPlaying", DBUS_TIMEOUT, &error,
				G_TYPE_INVALID,
				G_TYPE_BOOLEAN, &playing,
				G_TYPE_INVALID)) {
		ti->status = STATUS_OFF;
		return TRUE;
	}
	
	char *uri;
	if (!dbus_g_proxy_call_with_timeout(player, "getPlayingUri", DBUS_TIMEOUT, &error,
				G_TYPE_INVALID,
				G_TYPE_STRING, &uri,
				G_TYPE_INVALID)) {
		return FALSE;
	}

	GHashTable *table;
	if(!dbus_g_proxy_call_with_timeout(shell, "getSongProperties", DBUS_TIMEOUT, &error,
			G_TYPE_STRING, uri,
			G_TYPE_INVALID, 
			dbus_g_type_get_map("GHashTable", G_TYPE_STRING, G_TYPE_VALUE),	&table,
			G_TYPE_INVALID)) {
		if (!playing) {
			ti->status = STATUS_OFF;
			return TRUE;
		} else {
			return FALSE;
		}
	}
	g_free(uri);

	if (playing)
		ti->status = STATUS_NORMAL;
	else
		ti->status = STATUS_PAUSED;

	if (!GHashStr(table, "rb:stream-song-title", ti->track)) {
		GHashStr(table, "title", ti->track);
	}       
 
	GHashStr(table, "artist", ti->artist);
	GHashStr(table, "album", ti->album);
	ti->totalSecs = GHashUnit(table, "duration");
	g_hash_table_destroy(table);
	if (!dbus_g_proxy_call_with_timeout(player, "getElapsed", DBUS_TIMEOUT, &error,
				G_TYPE_INVALID,
				G_TYPE_UINT, &ti->currentSecs,
				G_TYPE_INVALID)) {
	}
	return TRUE;
}

static PurpleCmdRet 
SetStatus(PurpleConversation *conv, PurplePlugin *plugin,
					const gchar *cmd, gchar **args, gchar *error, void *data)
{
	PurpleAccount *account = NULL;
	PurpleConnection *gc = NULL;
	DBusGConnection *connection;

	gchar *buffer = NULL;

	struct TrackInfo ti;

	GRhInfo(&ti);

	/* Verify for irc protocol */
	gc = purple_conversation_get_gc(conv);
	account = purple_connection_get_account(gc);
	if(strcmp("prpl-irc", purple_account_get_protocol_id(account)))
		return;

	/* format string */
	switch(ti.status) {
		case STATUS_NORMAL: {
			buffer = g_strconcat("\001", "ACTION is now listening to ", ti.track, " — ", ti.artist, " [rhythmbox]", "\001", NULL);
				g_printf("%s", buffer);
			break;
		} 
		case STATUS_PAUSED: {
			buffer = g_strconcat("\001", "ACTION is now paused [rhythmbox]", "\001", NULL);
				g_printf("%s", buffer);
			break;
		}
		case STATUS_OFF: {
			buffer = g_strconcat("\001", "ACTION is now not running [rhythmbox]", "\001", NULL);
				g_printf("%s", buffer);
			break;
		}
	}

	/* debug */
	/*purple_notify_message(plugin, PURPLE_NOTIFY_MSG_INFO, "Debug!",
												buffer, NULL, NULL, NULL);*/

	/* Send */
	purple_conv_chat_send(PURPLE_CONV_CHAT(conv), buffer);

	/* clean mem */
	g_free(buffer);

  return PURPLE_CMD_RET_OK;
}

static PurpleCmdId mstatus;

static gboolean 
LoadPlugin(PurplePlugin *plugin)
{
	PurpleCmdFlag flags = PURPLE_CMD_FLAG_IM | PURPLE_CMD_FLAG_CHAT |	PURPLE_CMD_FLAG_ALLOW_WRONG_ARGS;
	mstatus = purple_cmd_register("m", "w", PURPLE_CMD_P_PLUGIN, flags, NULL,
							PURPLE_CMD_FUNC(SetStatus), NULL, NULL);
	return TRUE;
}

static gboolean 
UnloadPlugin(PurplePlugin *plugin) 
{
	purple_cmd_unregister(mstatus);
	return TRUE;
}

static PurplePluginInfo info = {
    PURPLE_PLUGIN_MAGIC,
    PURPLE_MAJOR_VERSION,
    PURPLE_MINOR_VERSION,
    PURPLE_PLUGIN_STANDARD,
    NULL,
    0,
    NULL,
    PURPLE_PRIORITY_DEFAULT,
    PLUGIN_ID,
    "IRC mStatus",
    "1.0",
    "IRC Music status plugin",          
    "IRC Music status plugin. Shows that you're \
				listening to in the form of the command \"/me is now listening to <track> — <artist> [rhythmbox]\" \
				just type \"/m\"",          
    "Dmitry <admin@itmages.ru>",                          
    "http://code.google.com/p/mStatus/",     
    LoadPlugin,                   
    UnloadPlugin,                          
    NULL,                                                  
    NULL,                          
    NULL,                          
    NULL,                        
    NULL,                   
    NULL,                          
    NULL,                          
    NULL,                          
    NULL                           
};                               
    
static void 
init_plugin(PurplePlugin *plugin) 
{                                  
}

PURPLE_INIT_PLUGIN(PLUGIN_ID, init_plugin, info)
