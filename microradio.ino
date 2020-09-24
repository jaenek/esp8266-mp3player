#include <FS.h>
#include <LittleFS.h>
#include "web.h"
#include "player.h"

void setup() {
	Serial.begin(115200);

	LittleFS.begin();

	WiFi.mode(WIFI_AP);
	WiFi.softAP(ap_ssid, ap_pass);

	Serial.print("AP IP address: ");
	Serial.println(WiFi.softAPIP());

	server.on("/setup", HTTP_GET, []{ servefile("/wifisetup.html"); });
	server.on("/setup", HTTP_POST, []{ savewifisetup(server.arg("ssid"), server.arg("pass")); servefile("/wifiresponse.html"); });
	server.onNotFound([]{ redirect("/setup"); });

	server.begin();

	Serial.println("Connecting to WiFi");
	WiFi.mode(WIFI_AP_STA);
	loadwifisetup();

	while (WiFi.status() != WL_CONNECTED) {
		server.handleClient();
	}
	Serial.println("Connected");

	server.stop();

	player = new Player();

	server.on("/", []{ redirect("control.html"); });
	server.on("/list", []{ server.send(200, "text/html", player->liststations()); });
	server.on("/select", []{  redirect("control.html"); player->setstation(server.arg("id").toInt()); });
	server.on("/delete", []{ player->deletestation(server.arg("id").toInt()); redirect("control.html"); });
	server.on("/add", []{ player->addstation(server.arg("name"), server.arg("url")); redirect("control.html"); });
	server.on("/stop", []{ redirect("control.html"); player->stop(); });
	server.on("/volup", []{ redirect("control.html"); player->volup(); });
	server.on("/voldown", []{ redirect("control.html"); player->voldown(); });
	server.onNotFound([]{ if (LittleFS.exists(server.uri())) servefile(server.uri()); else redirect("control.html"); });
	server.begin();
	Serial.println("Serving control panel at http://" + WiFi.localIP().toString());

	WiFi.softAP(ap_ssid + " - " + WiFi.localIP().toString(), ap_pass);
}

void loop() {
	server.handleClient();

	player->loop();
}
