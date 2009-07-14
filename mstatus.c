#define PURPLE_PLUGINS

#include <glib.h>
#include <glib/gprintf.h>
#include <string.h>
#include <cmds.h>
#include <conversation.h>
#include <dbus/dbus-glib.h>

#include "plugin.h"
#include "version.h"

#define STRLEN 100

#define STATUS_OFF 0
#define STATUS_PAUSED 1
#define STATUS_NORMAL 2

#define INTERVAL 10000
#define DBUS_TIMEOUT 100

gboolean get_hash_str(GHashTable *table, const char *key, char *dest) {
	GValue* value = (GValue*) g_hash_table_lookup(table, key);
	if (value != NULL && G_VALUE_HOLDS_STRING(value)) {
		strncpy(dest, g_value_get_string(value), STRLEN-1);
                return TRUE;
	}
        return FALSE;
}

unsigned int get_hash_uint(GHashTable *table, const char *key) {
	GValue* value = (GValue*) g_hash_table_lookup(table, key);
	if (value != NULL && G_VALUE_HOLDS_UINT(value)) {
		return g_value_get_uint(value);
	}
	return 0;
}

static gboolean get_rhythmbox_info(struct TrackInfo* ti) {
	DBusGConnection *connection;
	DBusGProxy *player, *shell;
	GError *error = 0;
	connection = dbus_g_bus_get(DBUS_BUS_SESSION, &error);
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
	if (!dbus_g_proxy_call_with_timeout(shell, "getSongProperties", DBUS_TIMEOUT, &error,
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
	if (!get_hash_str(table, "rb:stream-song-title", ti->track)) {
		get_hash_str(table, "title", ti->track);
	}        
	get_hash_str(table, "artist", ti->artist);
	get_hash_str(table, "album", ti->album);
	ti->totalSecs = get_hash_uint(table, "duration");
	g_hash_table_destroy(table);

	return TRUE;
}

static PurpleCmdRet SetStatus(PurpleConversation *conv, const gchar *cmd, gchar **args, gchar *error, void *data) {
	GString *msgstr = NULL;
	msgstr = g_string_new("");
	g_string_append(msgstr, "21341234");
	switch(purple_conversation_get_type(conv)) {
		case PURPLE_CONV_TYPE_IM:
			purple_conv_im_send(PURPLE_CONV_IM(conv), msgstr->str);
			break;
		case PURPLE_CONV_TYPE_CHAT:
			purple_conv_chat_send(PURPLE_CONV_CHAT(conv), msgstr->str);
			break;
		default:
			g_string_free(msgstr, TRUE);
			return PURPLE_CMD_RET_FAILED;
	}
	g_string_free(msgstr, TRUE);
	return PURPLE_CMD_RET_OK;
}

static PurpleCmdId mstatus;

static gboolean LoadPlugin(PurplePlugin *plugin) {
	PurpleCmdFlag flags = PURPLE_CMD_FLAG_IM | PURPLE_CMD_FLAG_CHAT |	PURPLE_CMD_FLAG_ALLOW_WRONG_ARGS;
	mstatus = purple_cmd_register("m", "w", PURPLE_CMD_P_PLUGIN, flags, NULL,
							PURPLE_CMD_FUNC(SetStatus), NULL, NULL);
	return TRUE;
}

static gboolean UnloadPlugin(PurplePlugin *plugin) {
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
    "core-mStatus",
    "IRC mStatus",
    "1.0",
    "IRC Music status plugin",          
    "IRC Music status plugin",          
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
    
static void init_plugin(PurplePlugin *plugin) {                                  
}

PURPLE_INIT_PLUGIN(mStatus, init_plugin, info)






