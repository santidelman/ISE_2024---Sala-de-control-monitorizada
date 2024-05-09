t <html><head><title>Test Ambiental</title>
t <script language=JavaScript type="text/javascript" src="xml_http.js"></script>
t <script language=JavaScript type="text/javascript">
t var mq = new periodicObj("ad.cgx", 500);
t var periodicUpdate = false; 
t function updateClock() {
t     var now = new Date(); 
t     var hours = now.getHours().toString().padStart(2, '0');
t     var minutes = now.getMinutes().toString().padStart(2, '0');
t     var seconds = now.getSeconds().toString().padStart(2, '0');
t     var formatted = hours + ":" + minutes + ":" + seconds;
t     document.getElementById('rtc1').value = formatted; // Actualiza el campo de texto de la hora
t     if (periodicUpdate) {
t         updateMQ(); 
t     }
t }
t function updateMQ() {
t     updateMultiple(mq, function() {
t         var mqElement = document.getElementById("ad_value");
t         var mqVal = mqElement.textContent;
t         var numVal = parseInt(mqVal, 16);
t         if (!isNaN(numVal)) {
t             var coLevel = (numVal * 50) / 4096; // Ajusta esta fórmula según la calibración del sensor
t             mqElement.textContent = coLevel.toFixed(2) + ' ppm'; // Mostrar ppm solo si numVal es un número
t         } else {
t             mqElement.textContent = "Datos no disponibles"; // Mostrar mensaje de error si el número no es válido
t         }
t     })
t }
t function periodicUpdateAd() {
t     periodicUpdate = document.getElementById("adChkBox").checked;
t     if (periodicUpdate) {
t         setInterval(updateMQ, 1000); // Actualizar cada segundo
t     } else {
t         clearInterval(updateMQ); // Detener la actualización
t     }
t }
t window.onload = function() {
t     setInterval(updateClock, 1000); // Ejecuta la función updateClock cada segundo
t }
t </script>
i pg_header.inc
t     <tr style="background-image:url(fondo.png);"><th>
t         <div style="width:900px;">
t             <div style="height: 10px;"></div>
t             <table style="font-family:Verdana; margin:10px;" align="right">
t                 <tr>
t                     <td>Hora</td>
#c z 1                 <td><input type=text name=rtc1 id=rtc1 size=20 maxlength=20 value="%s"></td>
t                 </tr>
t                 <tr>
t                     <td>Fecha</td>
#c z 2                 <td><input type=text name=rtc2 size=20 maxlength=20 value="%s"></td>
t                 </tr>
t             </table>
t             <div style="height: 50px;"></div>
t             <p style="font-size: 30px;"><b>Lectura de Condiciones Ambientales</b></p>
t                 <p align=center>
t                     <input type=button value="Refresh" onclick="updateMultiple(c, updateMQ)">
t                     Periodic:<input type="checkbox" id="adChkBox" onclick="periodicUpdateAd()">
t                 </p>
t             <center><table style="font-family:Verdana; margin:10px;">
t                 <tr padding="5">
t                     <td width="60" valign="top"><img src="temp.png" width="50" height="50"></td>
t                     <th style="width:200px; text-align:left;">Temperatura:</th>
t                     <th style="width:250px; text-align:center;">**</th>
t                     <th style="width:200px; text-align:right;">
t                         <input type=button
t                                value="Actualizar"
t                                style="background-color:#E9E9E9;"
t                                onclick="updateTemp();">
t                     </th>
t                 </tr>
t                 <tr padding="5">
t                     <td width="60" valign="top"><img src="humedad.png" width="50" height="50"></td>
t                     <th style="width:200px; text-align:left;">Humedad:</th>
t                     <th style="width:250px; text-align:center;">**</th>
t                     <th style="width:200px; text-align:right;">
t                         <input type=button
t                                value="Actualizar"
t                                style="background-color:#E9E9E9;"
t                                onclick="updateHum();">
t                     </th>
t                 </tr>
t                 <tr padding="5">
t                     <td width="60" valign="top"><img src="co2.png" width="50" height="50"></td>
t                     <th style="width:200px; text-align:left;">Monoxido de Carbono</th>
t                     <th style="width:300px; text-align:center;">
c g 1                       <span id="ad_value" value="0x%03X"></span>
t                     </th>
t                     <th style="width:200px; text-align:right;">
t                         <input type=button
t                                value="Actualizar"
t                                style="background-color:#E9E9E9;"
t                                onclick="updateMQ();">
t                     </th>
t                 </tr>
t             </table></center>
i pg_footer.inc
. End of script must be closed with period.
