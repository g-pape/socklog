#define LOG_PRI(p) ((p) & LOG_PRIMASK)
#define LOG_MAKEPRI(fac, pri) (((fac) <<3) | (pri))

#define INTERNAL_NOPRI 0x10 /* the "no priority" priority */
#define INTERNAL_MARK LOG_MAKEPRI(LOG_NFACILITIES, 0) /* mark "facility" */
typedef struct _code {
  char	*c_name;
  int	c_val;
} CODE;

#define LOG_AUTHPRIV (10 <<3)
#define LOG_FTP (11 <<3)

CODE prioritynames[] = {
  { "alert", LOG_ALERT },
  { "crit", LOG_CRIT },
  { "debug", LOG_DEBUG },
  { "emerg", LOG_EMERG },
  { "err", LOG_ERR },
  { "error", LOG_ERR }, /* DEPRECATED */
  { "info", LOG_INFO },
  { "none", INTERNAL_NOPRI }, /* INTERNAL */
  { "notice", LOG_NOTICE },
  { "panic", LOG_EMERG }, /* DEPRECATED */
  { "warn", LOG_WARNING }, /* DEPRECATED */
  { "warning", LOG_WARNING },
  { NULL, -1 }
};

#define LOG_FAC(p) (((p) & LOG_FACMASK) >>3) /* facility of pri */

CODE facilitynames[] = {
  { "auth", LOG_AUTH },
  { "authpriv", LOG_AUTHPRIV },
  { "cron", LOG_CRON },
  { "daemon", LOG_DAEMON },
  { "ftp", LOG_FTP },
  { "kern", LOG_KERN },
  { "lpr", LOG_LPR },
  { "mail", LOG_MAIL },
  { "mark", INTERNAL_MARK }, /* INTERNAL */
  { "news", LOG_NEWS },
  { "security", LOG_AUTH }, /* DEPRECATED */
  { "syslog", LOG_SYSLOG },
  { "user", LOG_USER },
  { "uucp", LOG_UUCP },
  { "local0", LOG_LOCAL0 },
  { "local1", LOG_LOCAL1 },
  { "local2", LOG_LOCAL2 },
  { "local3", LOG_LOCAL3 },
  { "local4", LOG_LOCAL4 },
  { "local5", LOG_LOCAL5 },
  { "local6", LOG_LOCAL6 },
  { "local7", LOG_LOCAL7 },
  { NULL, -1 }
};
