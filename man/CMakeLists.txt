install(FILES bnbot.1 bnchat.1 bnetd.1 bnetd.conf.5 bnftp.1 bni2tga.1 
    bnibuild.1 bniextract.1 bnilist.1 bnpass.1 bnpcap.1 bnproxy.1 bnstat.1 
    bntrackd.1 tgainfo.1
    DESTINATION ${MAN_INSTALL_DIR}/man1)

install(FILES bnetd.conf.5 bntext.5
    DESTINATION ${MAN_INSTALL_DIR}/man5)