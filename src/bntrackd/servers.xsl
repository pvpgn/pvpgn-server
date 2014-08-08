<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method="html" indent="yes" encoding="UTF-8" doctype-public="-//W3C/DTD XHTML 1.0 Transitional//EN" doctype-system="http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd" standalone="yes" media-type="text/html"/>
<xsl:template match="/">
<html>
<head><title>PvPGN XML Server List</title>
<link href="main.css" rel="stylesheet" type="text/css" />
</head>
<body>
<p>
<table border='0' cellspacing='0' cellpadding='3' width='256'>
  <tr><th><a href='xml_servers.tgz'>Download(bntrackd.c,XSL,PHP)</a></th><th><a href='servers.xsl'>servers.xsl</a></th></tr>
</table>
</p>
<table border='0' cellspacing='0' cellpadding='3' width='512'>
<tr><th align='left'>Address</th><th align='left'>Location</th><th align='left'>Description</th><th align='left'>URL</th><th align='left'>Uptime</th><th align='left'>Contact</th><th align='left'>Software</th><th>Users</th><th align='left'>Games</th><th align='left'>Channels</th></tr>
<xsl:for-each select="server_list/server">
<xsl:sort select="users" order="descending"/>
<tr>
<td><a href='bnetd://{address}:{port}/'><xsl:value-of select="address"/>:<xsl:value-of select="port"/></a></td>
<td><xsl:value-of select="location"/></td>
<td><xsl:value-of select="description"/></td>
<td nowrap="nowrap"><a href='{url}'><xsl:value-of select="url"/></a></td>
<td><xsl:value-of select="uptime"/></td>
<td><a href='mailto://{contact_email}'><xsl:value-of select="contact_name"/></a>&#160;<xsl:value-of select="contact_email"/></td>
<td><xsl:value-of select="software"/>&#160;<xsl:value-of select="version"/>&#160;<xsl:value-of select="platform"/></td>
<td><xsl:value-of select="users"/></td>
<td><xsl:value-of select="games"/></td>
<td><xsl:value-of select="channels"/></td></tr>
</xsl:for-each>
</table>
</body>
</html>
</xsl:template>
</xsl:stylesheet>
