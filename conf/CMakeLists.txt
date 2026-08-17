# generate the configs with proper line endings
set(OUTPUT_CONFS ad.json anongame_infos.conf address_translation.conf 
  autoupdate.conf bnalias.conf bnban.conf bnetd_default_user.plain 
  bnissue.txt bnmaps.conf bnxpcalc.conf
  bnxplevel.conf channel.conf command_groups.conf realm.conf 
  sql_DB_layout.conf supportfile.conf topics.json 
  tournament.conf versioncheck.json icons.conf
  email_verification.conf cacert.pem)
foreach(CONF ${OUTPUT_CONFS})
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${CONF}.in ${CMAKE_CURRENT_BINARY_DIR}/${CONF} @ONLY)
endforeach(CONF)

if(WIN32)
	# generate the configs with proper line endings
	set(OUTPUT_CONFS d2gs.reg d2gs.script d2server.ini d2warden.ini ItemConfig.dat)
	foreach(CONF ${OUTPUT_CONFS})
	  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${CONF}.in ${CMAKE_CURRENT_BINARY_DIR}/${CONF} @ONLY NEWLINE_STYLE WIN32)
	endforeach(CONF)
endif(WIN32)

# generate the config files replacing the path variables
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/bnetd.conf.in ${CMAKE_CURRENT_BINARY_DIR}/bnetd.conf)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/d2cs.conf.in ${CMAKE_CURRENT_BINARY_DIR}/d2cs.conf)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/d2dbs.conf.in ${CMAKE_CURRENT_BINARY_DIR}/d2dbs.conf)

if(WITH_BNETD)
  set(BNETD_CONFS bnetd.conf ad.json anongame_infos.conf address_translation.conf 
  autoupdate.conf bnalias.conf bnban.conf 
  bnetd_default_user.plain bnissue.txt bnmaps.conf  
  bnxpcalc.conf bnxplevel.conf channel.conf command_groups.conf  
  realm.conf sql_DB_layout.conf supportfile.conf topics.json 
  tournament.conf versioncheck.json icons.conf
  email_verification.conf cacert.pem)
endif(WITH_BNETD)

if(WITH_D2CS)
  set(D2CS_CONFS d2cs.conf anongame_infos.conf)
endif(WITH_D2CS)

if(WITH_D2DBS)
  set(D2DBS_CONFS d2dbs.conf)
endif(WITH_D2DBS)

if(WITH_D2GS)
  set(D2GS_CONFS d2gs.reg d2gs.script d2server.ini d2warden.ini ItemConfig.dat)
endif(WITH_D2GS)

foreach(CONF_FILE ${BNETD_CONFS} ${D2CS_CONFS} ${D2DBS_CONFS})
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${CONF_FILE} DESTINATION ${SYSCONFDIR})
endforeach(CONF_FILE)

foreach(CONF_FILE ${D2GS_CONFS})
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${CONF_FILE} DESTINATION ${SBIN_INSTALL_DIR})
endforeach(CONF_FILE)

add_subdirectory(i18n)