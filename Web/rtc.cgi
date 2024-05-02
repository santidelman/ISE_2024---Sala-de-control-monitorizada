t <html><head><title>RTC Control</title></head>
t </script></head>
i pg_header.inc
t <h2 align=center><br>RTC Module Control</h2>
t <p><font size="2">This page allows you to show you the hour of the <b>RTC</b>.
t  This Form uses a <b>POST</b> method to send data to a Web server.</font></p>
t <form action=rtc.cgi method=post name=cgi>
t <input type=hidden value="rtc" name=pg>
t <table border=0 width=99%><font size="3">
t <tr bgcolor=#aaccff>
t  <th width=40%>Item</th>
t  <th width=60%>Setting</th></tr>
# Here begin data setting which is formatted in HTTP_CGI.C module
t <tr><td><img src=pabb.gif>Hora</td>
c z 1 <td><input type=text name=rtc1 size=20 maxlength=20 value="%s"></td></tr>
t <tr><td><img src=pabb.gif>Fecha</td>
c z 2 <td><input type=text name=rtc2 size=20 maxlength=20 value="%s"></td></tr>
t <META HTTP-EQUIV="REFRESH" CONTENT="1">
i pg_footer.inc
. End of script must be closed with period.
