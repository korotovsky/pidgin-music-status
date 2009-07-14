#define PURPLE_PLUGINS

#include <glib.h>

//#include <ctype.h>
//#include <stdlib.h>
#include <string.h>
//#include <time.h>
#include <cmds.h>
#include <conversation.h>
//#include "notify.h"
#include "plugin.h"
#include "version.h"

static PurpleCmdRet putstatus(PurpleConversation *conv, const gchar *cmd, gchar **args, gchar *error, void *data) {
	GString *msgstr = NULL;
	msgstr = g_string_new("");
	g_string_append(msgstr, "test");
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

static gboolean plugin_load(PurplePlugin *plugin) {
	//const gchar *mstatus_help;
	PurpleCmdFlag flags = PURPLE_CMD_FLAG_IM | PURPLE_CMD_FLAG_CHAT |	PURPLE_CMD_FLAG_ALLOW_WRONG_ARGS;
	//mstatus_help = _("blablabla");
	mstatus = purple_cmd_register("m", "w", PURPLE_CMD_P_PLUGIN, flags, NULL,
							//PURPLE_CMD_FUNC(putstatus), mstatus_help, NULL);
							PURPLE_CMD_FUNC(putstatus), "12345", NULL);
	return TRUE;
}

static gboolean plugin_unload(PurplePlugin *plugin) {
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
    plugin_load,                   
    plugin_unload,                          
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
/*

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <cmds.h>
#include <conversation.h>
#include <debug.h>
#include <plugin.h>

#define PURPLE_MAJOR_VERSION 1
#define PURPLE_MINOR_VERSION 1

static PurpleCmdId mstatus;

static PurpleCmdRet putstatus(PurpleConversation *conv, const gchar *cmd, gchar **args, gchar *error, void *data) {
	GString *msgstr = NULL;
	msgstr = g_string_new("");
	g_string_append(msgstr, "test");
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

static gboolean plugin_load(PurplePlugin *plugin) {
	//const gchar *mstatus_help;
	PurpleCmdFlag flags = PURPLE_CMD_FLAG_IM | PURPLE_CMD_FLAG_CHAT |	PURPLE_CMD_FLAG_ALLOW_WRONG_ARGS;
	//mstatus_help = _("blablabla");
	mstatus = purple_cmd_register("m", "w", PURPLE_CMD_P_PLUGIN, flags, NULL,
							//PURPLE_CMD_FUNC(putstatus), mstatus_help, NULL);
							PURPLE_CMD_FUNC(putstatus), "12345", NULL);
	return TRUE;
}

static gboolean plugin_unload(PurplePlugin *plugin) {
	purple_cmd_unregister(mstatus);
	return TRUE;
}

static PurplePluginInfo mstatus_info = {
	PURPLE_PLUGIN_MAGIC,
	PURPLE_MAJOR_VERSION,
	PURPLE_MINOR_VERSION,
	PURPLE_PLUGIN_STANDARD,
	NULL,
	0,
	NULL,
	PURPLE_PRIORITY_DEFAULT,

	"core-hello_world",
	"Hello World!",
	"1.1",

	"Hello World Plugin",          
	"Hello World Plugin",          
	"My Name <email@helloworld.tld>",                          
	"http://helloworld.tld",     
    
	plugin_load,                   
	NULL,                          
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

PURPLE_INIT_PLUGIN(mstatus, init_plugin, mstatus_info)*/
