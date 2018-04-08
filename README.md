# Webradio for Synology and other platforms
Webradio with node.js server and url broadcast to esp8266 clients
The Node.js server and client can also be used on any other system architecture.
Use the folder 'webradio/package' as base folder for the node application.

##	Server:
The node.js server is a webserver with webpages to select a live stream url.
The server uses Websocket for variable data to/from clients.
When the server receive a stream url, it is broadcasted using Websocket as a JSON message like this:

	{ 'url' : '<http://livestreame.com>' }

The server also broadcasts the following information:
-	StreamTitle
-	samplerate
-	britrate
-	channels
These values are also broadcasted as JSON messages.

##	Web client:
The client can be used as a webradio and for selecting stations.

##	ESP8266 listener:
Using a ESP8266 as a listener, it can be connected to an amplifier for mono (Pin RX). 
By adding a I2S DAC module, the quality improves and the module can provide stereo.

	ESP pin   - I2S signal	ESP32	Wemos-D1
	----------------------------------------------
	GPIO2/TX1   - LRCK	GPIO25	D4		Rød
	GPIO3/RX0   - DATA	GPIO22	RX0		Orange
	GPIO15      - BCLK	GPIO26	D8		Gul



