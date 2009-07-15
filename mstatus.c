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

void
trace(const char *str, ...)
{
	char buf[512]; /*Log buffer*/
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
		trace("Ошибка при открытии соединения с dbus: %s\n", error->message);
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
		trace("Ошибка при получении статуса rhythmbox dbus (%s). Возможно, плеер не запущен.", error->message);
		ti->status = STATUS_OFF;
		return TRUE;
	}
	
	char *uri;
	if (!dbus_g_proxy_call_with_timeout(player, "getPlayingUri", DBUS_TIMEOUT, &error,
				G_TYPE_INVALID,
				G_TYPE_STRING, &uri,
				G_TYPE_INVALID)) {
		trace("Ошибка при получении uri из rhythmbox dbus (%s)", error->message);
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
			trace("Ошибка при получении информации о песне из rhythmbox dbus (%s)", error->message);
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
		trace("Ошибка при получении оставшегося времени проигрывания из rhythmbox dbus (%s)", error->message);
	}
	return TRUE;
}

static PurpleCmdRet 
SetStatus(PurpleConversation *conv, const gchar *cmd, gchar **args, gchar *error, void *data)
{
	//char buff[512]; /*Status buffer*/
	//char bufs[512]; /*Status buffer*/
    gchar *buff;
    gchar *msgstr;

	struct TrackInfo ti;
	DBusGConnection *connection;
	DBusGProxy *player, *shell;
	//GString *msgstr = NULL;

	GRhInfo(&ti);

	//sprintf(buffer, "ACTION is now listening to %s — %s [rhythmbox]", ti.track, ti.artist);

	//buff = g_strconcat("\x01/me is now listening to ", ti.track, " — ", ti.artist, "[rhythmbox]\x01", NULL);
	//buff = g_strconcat("/me ACTION is now listening to ", "Lala", " — ", "lala", "[rhythmbox]", NULL);
    //buff = g_strdup("/help");
   // g_printf("%s", buff);

	//g_string_append(msgstr, "\x01");
	//g_string_append(msgstr, buff);
	//g_string_append(msgstr, "\x01");

	//printf(bufs, "ACTION is now listening to %s — %s [rhythmbox]", ti.track, ti.artist);

	switch(purple_conversation_get_type(conv)) {
    case PURPLE_CONV_TYPE_IM:
      //purple_conv_im_send(PURPLE_CONV_IM(conv), buff);
			buff = g_strconcat("/me is now listening to ", ti.track, " — ", ti.artist, " [rhythmbox]", NULL);
				g_printf("%s", buff);
      //purple_conv_im_send_with_flags(PURPLE_CONV_IM(conv), buff, PURPLE_MESSAGE_SEND | PURPLE_MESSAGE_SYSTEM );
      purple_conv_im_send(PURPLE_CONV_IM(conv), buff);
      break;
    case PURPLE_CONV_TYPE_CHAT:
      //purple_conv_chat_send(PURPLE_CONV_CHAT(conv), buff);
      purple_conv_chat_send_with_flags(PURPLE_CONV_CHAT(conv), buff, PURPLE_MESSAGE_SEND | PURPLE_MESSAGE_SYSTEM );
      break;
    default:
      g_free(buff);
      return PURPLE_CMD_RET_FAILED;
  }

    g_free(buff);
  //g_string_free(msgstr, TRUE);
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






