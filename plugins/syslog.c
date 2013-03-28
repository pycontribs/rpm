#include <syslog.h>

#include "plugin.h"

rpmPluginHook PLUGIN_HOOKS = (PLUGINHOOK_INIT|
			      PLUGINHOOK_CLEANUP|
			      PLUGINHOOK_TSM_PRE|
			      PLUGINHOOK_TSM_POST|
			      PLUGINHOOK_PSM_POST|
			      PLUGINHOOK_SCRIPTLET_POST);

static int logging = 0;
static unsigned int scriptfail = 0;
static unsigned int pkgfail = 0;

rpmRC PLUGINHOOK_INIT_FUNC(rpmts ts, const char * name, const char * opts)
{
    /* XXX make this configurable? */
    const char * log_ident = "[RPM]";

    openlog(log_ident, (LOG_PID), LOG_USER);
    return RPMRC_OK;
}

rpmRC PLUGINHOOK_CLEANUP_FUNC(void)
{
    closelog();
    return RPMRC_OK;
}

rpmRC PLUGINHOOK_TSM_PRE_FUNC(rpmts ts)
{
    /* Reset counters */
    scriptfail = 0;
    pkgfail = 0;

    /* Assume we are logging but... */
    logging = 1;

    /* ...don't log test transactions */
    if (rpmtsFlags(ts) & (RPMTRANS_FLAG_TEST|RPMTRANS_FLAG_BUILD_PROBS))
	logging = 0;

    /* ...don't log chroot transactions */
    if (!rstreq(rpmtsRootDir(ts), "/"))
	logging = 0;

    if (logging) {
	syslog(LOG_NOTICE, "Transaction ID %x started", rpmtsGetTid(ts));
    }

    return RPMRC_OK;
}

rpmRC PLUGINHOOK_TSM_POST_FUNC(rpmts ts, int res)
{
    if (logging) {
	if (pkgfail || scriptfail) {
	    syslog(LOG_WARNING, "%u elements failed, %u scripts failed",
		   pkgfail, scriptfail);
	}
	syslog(LOG_NOTICE, "Transaction ID %x finished: %d",
		rpmtsGetTid(ts), res);
    }

    logging = 0;
    return RPMRC_OK;
}

rpmRC PLUGINHOOK_PSM_POST_FUNC(rpmte te, int res)
{
    if (logging) {
	int lvl = LOG_NOTICE;
	const char *op = (rpmteType(te) == TR_ADDED) ? "install" : "erase";
	const char *outcome = "success";
	/* XXX: Permit configurable header queryformat? */
	const char *pkg = rpmteNEVRA(te);

	if (res != RPMRC_OK) {
	    lvl = LOG_WARNING;
	    outcome = "failure";
	    pkgfail++;
	}

	syslog(lvl, "%s %s: %s", op, pkg, outcome);
    }
    return RPMRC_OK;
}

rpmRC PLUGINHOOK_SCRIPTLET_POST_FUNC(const char *s_name, int type, int res)
{
    if (logging && res) {
	syslog(LOG_WARNING, "scriptlet %s failure: %d\n", s_name, res);
	scriptfail++;
    }
    return RPMRC_OK;
}