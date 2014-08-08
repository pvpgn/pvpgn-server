if COND_BNETD
bnetd_confs = bnetd.conf ad.conf anongame_infos.conf address_translation.conf \
	autoupdate.conf bnalias.conf bnban.conf bnetd_default_user.cdb \
	bnetd_default_user.plain bnissue.txt bnmaps.conf \
	bnmotd.txt bnxpcalc.conf bnxplevel.conf channel.conf \
	command_groups.conf realm.conf sql_DB_layout.conf \
	supportfile.conf topics.conf tournament.conf versioncheck.conf \
	sql_DB_layout2.conf
bnetd_extras = bnetd.conf.in bnetd.conf.win32 ad.conf.in anongame_infos.conf.in \
	address_translation.conf.in autoupdate.conf.in bnalias.conf.in \
	bnban.conf.in bnetd_default_user.plain.in bnissue.txt.in \
	bnmaps.conf.in bnxpcalc.conf.in bnxplevel.conf.in \
	channel.conf.in command_groups.conf.in realm.conf.in \
	sql_DB_layout.conf.in supportfile.conf.in topics.conf.in \
	tournament.conf.in versioncheck.conf.in sql_DB_layout2.conf.in
else
bnetd_confs =
bnetd_extras =
endif

if COND_D2CS
d2cs_confs = d2cs.conf address_translation.conf d2server.ini
d2cs_extras = d2cs.conf.in d2cs.conf.win32 address_translation.conf.in
else
d2cs_confs =
d2cs_extras =
endif

if COND_D2DBS
d2dbs_confs = d2dbs.conf
d2dbs_extras = d2dbs.conf.in d2dbs.conf.win32
else
d2dbs_confs =
d2dbs_extras =
endif

dist_sysconf_DATA = $(bnetd_confs) $(d2cs_confs) $(d2dbs_confs)

EXTRA_DIST = $(bnetd_extras) $(d2cs_extras) $(d2dbs_extras)

%.conf : %.conf.in $(top_builddir)/config.status
	$(CP) $< $@

%.txt : %.txt.in $(top_builddir)/config.status
	$(CP) $< $@

bnetd_default_user.plain : bnetd_default_user.plain.in $(top_builddir)/config.status
	$(CP) $< $@

bnetd.conf:bnetd.conf.in $(top_builddir)/config.status
	$(SED) -e "s,\$${LOCALSTATEDIR}/,$(localstatedir)/,g" -e "s,\$${SYSCONFDIR}/,$(sysconfdir)/,g" $(top_srcdir)/conf/bnetd.conf.in > $@

d2cs.conf:d2cs.conf.in $(top_builddir)/config.status
	$(SED) -e "s,\$${LOCALSTATEDIR}/,$(localstatedir)/,g" -e "s,\$${SYSCONFDIR}/,$(sysconfdir)/,g" $(top_srcdir)/conf/d2cs.conf.in > $@

d2dbs.conf:d2dbs.conf.in $(top_builddir)/config.status
	$(SED) -e "s,\$${LOCALSTATEDIR}/,$(localstatedir)/,g" -e "s,\$${SYSCONFDIR}/,$(sysconfdir)/,g" $(top_srcdir)/conf/d2dbs.conf.in > $@

CONFIG_CLEAN_FILES = bnetd.conf d2cs.conf d2dbs.conf ad.conf \
	anongame_infos.conf address_translation.conf autoupdate.conf \
	bnalias.conf bnban.conf bnetd_default_user.plain \
	bnissue.txt bnmaps.conf bnxpcalc.conf bnxplevel.conf \
	channel.conf command_groups.conf realm.conf \
	sql_DB_layout.conf supportfile.conf topics.conf tournament.conf \
	versioncheck.conf sql_DB_layout2.conf 
