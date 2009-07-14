/*
 * Copyleft
 */

#include "../common/pp_internal.h"

/* libc */
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Purple */
#include <cmds.h>
#include <conversation.h>
#include <debug.h>
#include <plugin.h>

static PurpleCmdId bash, qdb;

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
	const gchar *mstatus_help;
	PurpleCmdFlag flags = PURPLE_CMD_FLAG_IM | PURPLE_CMD_FLAG_CHAT |	PURPLE_CMD_FLAG_ALLOW_WRONG_ARGS;
	mstatus_help = _("blablabla");
	mstatus = purple_cmd_register("m", "w", PURPLE_CMD_P_PLUGIN, flags, NULL,
							PURPLE_CMD_FUNC(putstatus), bash_help, NULL);
	return TRUE;
}

static gboolean plugin_unload(PurplePlugin *plugin) {
	purple_cmd_unregister(mstatus);
	return TRUE;
}

static PurplePluginInfo bash_info =
{
	PURPLE_PLUGIN_MAGIC, /* magic, my ass */
	PURPLE_MAJOR_VERSION,
	PURPLE_MINOR_VERSION,
	PURPLE_PLUGIN_STANDARD,
	NULL,
	0,
	NULL,
	PURPLE_PRIORITY_DEFAULT,
	"wtf???",
	NULL,
	PP_VERSION,
	NULL,
	NULL,
	"Dmitry <rmpic30@gmail.com>",
	PP_WEBSITE,
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
#ifdef ENABLE_NLS
	bindtextdomain(GETTEXT_PACKAGE, PP_LOCALEDIR);
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
#endif /* ENABLE_NLS */
	mstatus_info.name = _("mstatus");
	mstatus_info.summary =
					_("12345");
	mstatus_info.description =
					_("1234567890 1234567890 /m command.");

	return;
}

PURPLE_INIT_PLUGIN(bash, init_plugin, mstatus_info)
