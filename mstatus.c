#define PURPLE_PLUGINS

#include <glib.h>
#include <glib/gprintf.h>
#include <string.h>
#include <cmds.h>
#include <conversation.h>
#include <dbus/dbus-glib.h>
#include "notify.h"
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

/* Trace a debugging message. Writes to log file as well as purple
 * debug sink.
 */
void
trace(const char *str, ...)
{
	char buf[512];
	va_list ap;
	va_start(ap, str);
	vsnprintf(buf, 512, str, ap);
	va_end(ap);

	FILE *log = fopen("/tmp/mstatus.log", "a");
	assert(log);
	time_t t;
	time(&t);
	fprintf(log, "%s: %s\n", ctime(&t), buf);
	fclose(log);

	purple_debug_info(PLUGIN_ID, "%s\n", buf);
}

gboolean get_hash_str(GHashTable *table, const char *key, char *dest) {
	GValue* value = (GValue*) g_hash_table_lookup(table, key);
	if (value != NULL && G_VALUE_HOLDS_STRING(value)) {
		strncpy(dest, g_value_get_string(value), STRLEN-1);
                return TRUE;
	}
        return FALSE;
}

unsigned int
get_hash_uint(GHashTable *table, const char *key)
{
	GValue* value = (GValue*) g_hash_table_lookup(table, key);
	if (value != NULL && G_VALUE_HOLDS_UINT(value)) {
		return g_value_get_uint(value);
	}
	return 0;
}

gboolean
get_rhythmbox_info(struct TrackInfo* ti)
{
	DBusGConnection *connection;
	DBusGProxy *player, *shell;
	GError *error = 0;

	connection = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
	if (connection == NULL) {
		trace("Failed to open connection to dbus: %s\n", error->message);
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
		trace("Failed to get playing state from rhythmbox dbus (%s). Assuming player is off", error->message);
		ti->status = STATUS_OFF;
		return TRUE;
	}
	
	char *uri;
	if (!dbus_g_proxy_call_with_timeout(player, "getPlayingUri", DBUS_TIMEOUT, &error,
				G_TYPE_INVALID,
				G_TYPE_STRING, &uri,
				G_TYPE_INVALID)) {
		trace("Failed to get song uri from rhythmbox dbus (%s)", error->message);
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
			trace("Failed to get song info from rhythmbox dbus (%s)", error->message);
			return FALSE;
		}
	}
	g_free(uri);

	if (playing)
		ti->status = STATUS_NORMAL;
	else
		ti->status = STATUS_PAUSED;

	if (!get_hash_str(table, "rb:stream-song-title", ti->track))
	{
		get_hash_str(table, "title", ti->track);

	}       
 
	get_hash_str(table, "artist", ti->artist);
	get_hash_str(table, "album", ti->album);
	ti->totalSecs = get_hash_uint(table, "duration");
	g_hash_table_destroy(table);
	if (!dbus_g_proxy_call_with_timeout(player, "getElapsed", DBUS_TIMEOUT, &error,
				G_TYPE_INVALID,
				G_TYPE_UINT, &ti->currentSecs,
				G_TYPE_INVALID)) {
		trace("Failed to get elapsed time from rhythmbox dbus (%s)", error->message);
	}
	return TRUE;
}

static PurpleCmdRet 
SetStatus(PurpleConversation *conv, const gchar *cmd, gchar **args, gchar *error, void *data)
{
	char buffer[1024];
	struct TrackInfo ti;
	DBusGConnection *connection;
	DBusGProxy *player, *shell;
	GString *msgstr = NULL;

	get_rhythmbox_info(&ti);

	msgstr = g_string_new("");
	sprintf(buffer, "/me ::: %s ::: %s ::: %s :::", ti.track, ti.artist, ti.album);
	g_string_append(msgstr, buffer);
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

static gboolean 
LoadPlugin(PurplePlugin *plugin)
{
	PurpleCmdFlag flags = PURPLE_CMD_FLAG_IM | PURPLE_CMD_FLAG_CHAT |	PURPLE_CMD_FLAG_ALLOW_WRONG_ARGS;
	mstatus = purple_cmd_register("m", "w", PURPLE_CMD_P_PLUGIN, flags, NULL,
							PURPLE_CMD_FUNC(SetStatus), NULL, NULL);
	trace("Start logging");
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
    
static void 
init_plugin(PurplePlugin *plugin) 
{                                  
}

PURPLE_INIT_PLUGIN(PLUGIN_ID, init_plugin, info)






