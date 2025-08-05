const char ota_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <title>FIRMWARE UPDATE</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    :root {
      --primary-color: #4CAF50;
      --primary-hover: #45a049;
      --background-color: #f5f5f5;
      --text-color: #333;
      --input-border-color: #ddd;
      --card-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
      --transition: all 0.3s ease;
    }

    * {
      margin: 0;
      padding: 0;
      box-sizing: border-box;
    }

    body {
      background-color: var(--background-color);
      color: var(--text-color);
      font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Oxygen, Ubuntu, Cantarell, sans-serif;
      line-height: 1.6;
      min-height: 100vh;
      display: flex;
      flex-direction: column;
      align-items: center;
    }

    header {
      background-color: var(--primary-color);
      color: white;
      width: 100%;
      padding: 1.5rem 0;
      text-align: center;
      box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
      margin-bottom: 2rem;
    }

    header h1 {
      font-size: 1.8rem;
      font-weight: 600;
    }

    main {
      width: 90%;
      max-width: 500px;
      margin: 0 auto;
      padding: 2rem;
      background: white;
      border-radius: 10px;
      box-shadow: var(--card-shadow);
    }

    .input-group {
      margin-bottom: 1.5rem;
    }

    label {
      display: block;
      margin-bottom: 0.5rem;
      font-weight: 500;
      color: var(--text-color);
    }

    input[type="file"] {
      width: 100%;
      padding: 0.8rem;
      border: 2px solid var(--input-border-color);
      border-radius: 6px;
      font-size: 1rem;
      transition: var(--transition);
      margin-bottom: 1.5rem;
    }

    input[type="submit"] {
      width: 100%;
      padding: 1rem;
      background-color: var(--primary-color);
      color: white;
      border: none;
      border-radius: 6px;
      font-size: 1rem;
      font-weight: 600;
      cursor: pointer;
      transition: var(--transition);
      margin-bottom: 1.5rem;
    }

    input[type="submit"]:hover {
      background-color: var(--primary-hover);
      transform: translateY(-1px);
    }

    .status {
      margin-top: 1rem;
      padding: 1.5rem;
      background-color: #f8f9fa;
      border-radius: 6px;
      box-shadow: var(--card-shadow);
    }

    .status p {
      margin: 0.5rem 0;
      color: var(--text-color);
      font-size: 0.95rem;
    }

    .progress-bar {
      width: 100%;
      height: 20px;
      background-color: #f0f0f0;
      border-radius: 10px;
      overflow: hidden;
      margin: 1rem 0;
    }

    .progress {
      width: 0%;
      height: 100%;
      background-color: var(--primary-color);
      transition: width 0.3s ease;
    }

    .buttons {
      margin-top: 1rem;
      text-align: center;
    }

    .button {
      display: inline-block;
      padding: 0.8rem 1.5rem;
      background-color: var(--primary-color);
      color: white;
      text-decoration: none;
      border-radius: 6px;
      transition: var(--transition);
      font-weight: 500;
    }

    .button:hover {
      background-color: var(--primary-hover);
      transform: translateY(-1px);
    }

    @media (max-width: 600px) {
      main {
        width: 95%;
        padding: 1.5rem;
      }

      header {
        padding: 1rem 0;
      }

      header h1 {
        font-size: 1.5rem;
      }
    }
  </style>
</head>
<body>
  <header>
    <h1> X-POE OTA Update</h1>
  </header>
  <main>
    <form id="uploadForm" method="POST" action="/update" enctype="multipart/form-data">
      <div class="input-group">
        <label>Select Firmware:</label>
        <input type="file" name="update" accept=".bin">
      </div>
      <input type="submit" value="Update Firmware">
    </form>
    <div class="status">
      <p>Update Progress:</p>
      <div class="progress-bar">
        <div class="progress" id="progress"></div>
      </div>
      <p>Progress: <span id="progress-text">0%</span></p>
    </div>
    <div class="buttons">
      <a href="/" class="button">HOME</a>
    </div>
  </main>
  <script>
    const form = document.getElementById('uploadForm');
    const progressBar = document.getElementById('progress');
    const progressText = document.getElementById('progress-text');
    const statusText = document.getElementById('status');

    form.onsubmit = function (e) {
      e.preventDefault();
      const formData = new FormData(form);
      const xhr = new XMLHttpRequest();
      xhr.open('POST', '/update', true);

      xhr.upload.onprogress = function (event) {
        if (event.lengthComputable) {
          const percentComplete = (event.loaded / event.total) * 100;
          progressBar.style.width = percentComplete + '%';
          progressText.textContent = Math.round(percentComplete) + '%';
        }
      };

      xhr.onload = function () {
        if (xhr.status === 200) {
          alert('OTA Update Success! Restarting...');
          setTimeout(() => {
            location.reload();
          }, 3000);
        } else {
          alert('OTA Update Failed!');
        }
      };

      xhr.send(formData);
    };
  </script>
</body>
</html>
)rawliteral";

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>PROCOM</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            text-align: center;
            margin: 50px;
            background-color: #f4f4f9;
        }
        h1 {
            color: #333;
        }
        button {
            background-color: #4CAF50;
            color: white;
            padding: 15px 32px;
            text-align: center;
            font-size: 18px;
            cursor: pointer;
            border: none;
            margin: 10px;
            border-radius: 8px;
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
            width: 350px;
        }
        button:hover {
            background-color: #45a049;
        }
        form {
            margin-top: 20px;
        }
        input[type="text"], input[type="password"], input[type="number"] {
            padding: 8px;
            margin: 10px 0;
            width: 300px;
            font-size: 16px;
            border-radius: 5px;
            border: 1px solid #ddd;
        }
        input[type="submit"] {
            background-color: #4CAF50;
            color: white;
            padding: 10px 20px;
            font-size: 16px;
            cursor: pointer;
            border: none;
            border-radius: 5px;
        }
        input[type="submit"]:hover {
            background-color: #45a049;
        }
    </style>
</head>
<body>
<h1>PROCOM X-POE TOUCH PANEL 2 MODULE 4K V1.9</h1>
<button onclick="location.href='/wifi'">WiFi Configuration</button><br>
<button onclick="location.href='/xpoe'">X-POE Configuration</button><br>
<button onclick="location.href='/handleConfig'">X-POE SLAVE Config</button><br>
<button onclick="location.href='/touchcfg'">Touch Panel Configuration</button><br>
<button onclick="location.href='/ota'">OTA Update</button><br>
<button onclick="location.href='/restart'">RESTART</button><br><br>
<button onclick="logoutButton()">Logout</button>

<script>
function logoutButton() {
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/logout", true);
  xhr.send();
  setTimeout(function(){ window.open("/logged-out","_self"); }, 1000);
}
</script>
</body>
</html>
)rawliteral";

const char logout_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
</head>
<body>
  <p>Logged out or <a href="/">return to homepage</a>.</p>
  <p><strong>Note:</strong> close all web browser tabs to complete the logout process.</p>
</body>
</html>
)rawliteral";

// HTML and CSS content
const char header_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>PROCOM X</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            text-align: center;
            margin: 50px;
            background-color: #f4f4f9;
        }
        h1 {
            color: #333;
        }
        button {
            background-color: #4CAF50;
            color: white;
            padding: 15px 32px;
            text-align: center;
            font-size: 18px;
            cursor: pointer;
            border: none;
            margin: 10px;
            border-radius: 8px;
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
            width: 350px;
        }
        button:hover {
            background-color: #45a049;
        }
        form {
            margin-top: 20px;
        }
        input[type="text"], input[type="password"], input[type="number"] {
            padding: 8px;
            margin: 10px 0;
            width: 300px;
            font-size: 16px;
            border-radius: 5px;
            border: 1px solid #ddd;
        }
        input[type="submit"] {
            background-color: #4CAF50;
            color: white;
            padding: 10px 20px;
            font-size: 16px;
            cursor: pointer;
            border: none;
            border-radius: 5px;
        }
        input[type="submit"]:hover {
            background-color: #45a049;
        }
    </style>
</head>
<body>
)rawliteral";

const char footer_html[] PROGMEM = R"rawliteral(
    <br><br>
    <button onclick="location.href='/'" style="padding: 10px 20px; font-size: 16px; cursor: pointer;">Home</button>
</body>
</html>
)rawliteral";

// X-POE Configuration page HTML content
String getXPOEConfigPage()
{
  String page = String(header_html);
  page += "<h1>X-POE Configuration Page</h1>";
  page += "<form method='POST' action='/xpoesave'>";
  page += "MQTT Server IP: <input type='text' name='mqttServer' value='" + mqtt_server + "'><br><br>";
  page += "MQTT Port: <input type='number' name='mqttPort' value='" + String(mqtt_port) + "'><br><br>";
  page += "MQTT User: <input type='text' name='mqttUser' value='" + mqtt_user + "'><br><br>";
  page += "MQTT Password: <input type='password' name='mqttPassword' value='" + mqtt_password + "'><br><br>";
  page += "MQTT MacID: <input type='text' name='mqttMacID' value='" + mqtt_macid + "'><br><br>";
  page += "HTTP USER: <input type='text' name='httpUser' value='" + http_user + "'><br><br>";
  page += "HTTP PASS: <input type='text' name='httpPass' value='" + http_pass + "'><br><br>";
  page += "DIM Steps: <input type='number' name='dimSteps' value='" + String(dimSteps) + "'><br><br>";
  page += "DIM delay(500-2000ms): <input type='number' name='dimDelay' value='" + String(dimTickerMillis) + "'><br><br>";

  page += "<input type='submit' value='Save'>";
  page += "</form>";
  page += String(footer_html);
  return page;
}

// Wi-Fi Configuration page HTML content
String getWiFiConfigPage()
{
  String page = String(header_html);
  page += "<h1>WiFi Configuration Page</h1>";
  page += "<form method='POST' action='/wifisave'>";
  page += "SSID: <input type='text' name='wifiSSID' value='" + String(wifiSSID) + "'><br><br>";
  page += "Password: <input type='password' name='wifiPassword' value='" + String(wifiPassword) + "'><br><br>";

  page += "Use Static IP: <input type='checkbox' name='useStaticIP' " + String(useStaticIP ? "checked" : "") + "><br>";
  page += "Static IP: <input type='text' name='staticIP' value='" + String(staticIP) + "'><br>";
  page += "Gateway: <input type='text' name='gateway' value='" + String(gateway) + "'><br>";
  page += "Subnet: <input type='text' name='subnet' value='" + String(subnet) + "'><br>";

  page += "<input type='submit' value='Save'>";
  page += "</form>";
  page += String(footer_html);
  return page;
}

void handleRoot()
{
  if (!server.authenticate(http_username, http_password))
  {
    return server.requestAuthentication();
  }
  String html = String(index_html);
  // html.replace("%BUTTONPLACEHOLDER%", processor("BUTTONPLACEHOLDER"));
  // html.replace("%STATE%", processor("STATE"));
  server.send(200, "text/html", html);
}

void handleOtaPage()
{
  server.send(200, "text/html", ota_html);
  otaEnabled = true;
  Serial.println("Enabled OTA, Sensor Read Stopped");
  otaEnableTime = millis();
}

void handleUpdate() {
    HTTPUpload &upload = server.upload();

    if (upload.status == UPLOAD_FILE_START) {
        Serial.printf("Update: %s\n", upload.filename.c_str());
        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
            Update.printError(Serial);
        }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
            Update.printError(Serial);
        }
    } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) {
            Serial.printf("Update Success: %u bytes\n", upload.totalSize);
        } else {
            Update.printError(Serial);
        }
        otaEnabled = false;
    }else if (upload.status == UPLOAD_FILE_ABORTED)
    {
      Update.abort();
      Serial.println("Update Aborted");
      otaEnabled = false;
      server.send(500, "text/plain", "Update Aborted");
    }
    delay(100);
}

void handleUpdateFinished() {
    server.send(200, "text/plain", Update.hasError() ? "FAIL" : "OK");
    delay(500);
    ESP.restart();
}

void server_ota()
{
  server.on("/ota", HTTP_GET, handleOtaPage);
  server.on("/update", HTTP_POST, handleUpdateFinished, handleUpdate);
}

void handleLogout()
{
  server.send(401);
}

void handleLoggedOut()
{
  server.send(200, "text/html", logout_html);
}

void handleWiFiConfig()
{
  if (!server.authenticate(http_username, http_password))
  {
    return server.requestAuthentication();
  }
  String page = getWiFiConfigPage();
  server.send(200, "text/html", page);
}

void handleWiFiSubmit()
{
  if (!server.authenticate(http_username, http_password))
  {
    return server.requestAuthentication();
  }
  if (server.method() == HTTP_POST)
  {
    if (server.hasArg("wifiSSID"))
      wifiSSID = server.arg("wifiSSID");
    if (server.hasArg("wifiPassword"))
      wifiPassword = server.arg("wifiPassword");

    if (server.hasArg("useStaticIP"))
    {
      useStaticIP = true;
    }
    else
    {
      useStaticIP = false;
    }
    if (server.hasArg("staticIP"))
      staticIP = server.arg("staticIP");
    if (server.hasArg("gateway"))
      gateway = server.arg("gateway");
    if (server.hasArg("subnet"))
      subnet = server.arg("subnet");

    Serial.println(useStaticIP);
    Serial.println(staticIP);
    Serial.println(subnet);

    preferences.begin("wifi", false);
    preferences.putString("wifiSSID", wifiSSID);
    preferences.putString("wifiPassword", wifiPassword);

    preferences.putBool("useStaticIP", useStaticIP);
    preferences.putString("staticIP", staticIP);
    preferences.putString("gateway", gateway);
    preferences.putString("subnet", subnet);
    preferences.end();

    server.send(200, "text/html", "<h1>WiFi Configuration Saved!</h1><a href='/wifi'>Back</a>");
    // WiFi.disconnect();
    // WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());
  }
}

void handleXPOEConfig()
{
  if (!server.authenticate(http_username, http_password))
  {
    return server.requestAuthentication();
  }
  String page = getXPOEConfigPage();
  server.send(200, "text/html", page);
}

void handleXPOESubmit()
{
  if (!server.authenticate(http_username, http_password))
  {
    return server.requestAuthentication();
  }
  if (server.method() == HTTP_POST)
  {
    if (server.hasArg("mqttServer"))
      mqtt_server = server.arg("mqttServer");
    if (server.hasArg("mqttPort"))
      mqtt_port = server.arg("mqttPort").toInt();
    if (server.hasArg("mqttUser"))
      mqtt_user = server.arg("mqttUser");
    if (server.hasArg("mqttPassword"))
      mqtt_password = server.arg("mqttPassword");
    if (server.hasArg("mqttMacID"))
      mqtt_macid = server.arg("mqttMacID");

    if (server.hasArg("httpUser"))
      http_user = server.arg("httpUser");
    if (server.hasArg("httpPass"))
      http_pass = server.arg("httpPass");

    if (server.hasArg("dimSteps"))
      dimSteps = server.arg("dimSteps").toInt();
    if (server.hasArg("dimDelay"))
      dimTickerMillis = server.arg("dimDelay").toInt();

    preferences.begin("mqtt", false);
    preferences.putString("mqttServer", mqtt_server);
    preferences.putUInt("mqttPort", mqtt_port);
    preferences.putString("mqttUser", mqtt_user);
    preferences.putString("mqttPassword", mqtt_password);
    preferences.putString("mqttMacID", mqtt_macid);

    preferences.putString("httpUser", http_user);
    preferences.putString("httpPass", http_pass);

    preferences.putUInt("dimSteps", dimSteps);
    preferences.putUInt("dimDelay", dimTickerMillis);

    preferences.end();

    server.send(200, "text/html", "<h1>Configuration Saved!</h1><a href='/xpoe'>Back</a>");
  }
}

void handleRestart()
{
  if (!server.authenticate(http_username, http_password))
  {
    return server.requestAuthentication();
  }
  server.send(200, "text/html", "<h1>Restart Success!</h1><a href='/'>Home</a>");
  delay(1000);
  ESP.restart();
}

// void handleFirmwareUpdate()
// {
//   if (!server.authenticate(http_username, http_password))
//   {
//     return server.requestAuthentication();
//   }
//   server.send(200, "text/html", "<h1>NOT YET SUPPORTED</h1><a href='/'> Go Home</a>");
//   delay(1000);
//   ESP.restart();
// }

//Mac ID 
void handleSelect() {
  String indexStr = server.arg("macIndex");
  int index = indexStr.toInt();
  
  if(index >= 0 && index < MAX_MACIDS) {
    Serial.print("Selected MAC ID Index: ");
    Serial.println(index);
    Serial.print("Selected MAC ID: ");
    Serial.println(macIds[index]);
    
    server.send(200, "text/html", "<html><body><h2>Selection Processed</h2>"
                "<p>Index: " + String(index) + "</p>"
                "<p>MAC ID: " + macIds[index] + "</p>"
                "<p><a href='/'>Back to Home</a></p></body></html>");
  } else {
    server.send(400, "text/html", "<html><body><h2>Invalid Selection</h2><p><a href='/'>Back to Home</a></p></body></html>");
  }
}

void handleConfig() {
  String html = "<!DOCTYPE html><html><head>";
  html += "<title>MAC ID Configuration</title>";
  html += "<style>";
  html += "body{font-family:Arial;margin:20px;}";
  html += ".input-group{margin-bottom:15px;}";
  html += "input{padding:8px;margin:5px;width:300px;}";
  html += "button{padding:8px;margin:5px;}";
  html += "label{display:inline-block;width:80px;}";
  html += "</style>";
  html += "</head><body>";
  html += "<h2>Configure MAC IDs and JWT Tokens</h2>";
  html += "<form action='/savemacId' method='POST'>";
  
  for(int i = 0; i < MAX_MACIDS; i++) {
    html += "<div class='input-group'>";
    html += "<div>";
    html += "<label>MAC ID " + String(i+1) + ":</label>";
    html += "<input type='text' name='mac" + String(i) + "' value='" + macIds[i] + "' pattern='^([0-9A-Fa-f]{2}[:-]){5}([0-9A-Fa-f]{2})$'>";
    html += "</div><div>";
    html += "<label>JWT Token:</label>";
    html += "<input type='text' name='jwt" + String(i) + "' value='" + jwtTokens[i] + "'>";
    html += "</div></div>";
  }
  
  html += "<button type='submit'>Save</button>";
  html += "</form>";
  html += "<p><a href='/'><button>Back to Home</button></a></p>";
  html += "</body></html>";
  
  server.send(200, "text/html", html);
}

void other_routes_setup()
{
  server.on("/", HTTP_GET, handleRoot);
  server.on("/logout", HTTP_GET, handleLogout);
  server.on("/logged-out", HTTP_GET, handleLoggedOut);
  server.on("/wifi", HTTP_GET, handleWiFiConfig);
  server.on("/wifisave", HTTP_POST, handleWiFiSubmit);
  //server.on("/ota", HTTP_GET, handleFirmwareUpdate);
  server.on("/xpoe", handleXPOEConfig);
  server.on("/xpoesave", HTTP_POST, handleXPOESubmit);
  server.on("/restart", HTTP_GET, handleRestart);

  server.on("/savemacId", HTTP_POST, handleSaveMacIds);
  server.on("/handleConfig", HTTP_GET, handleConfig);
  server.on("/select", HTTP_POST, handleSelect);
}
