# Webradio for Synology and other platforms
Webradio with node.js server and url broadcast to esp8266 clients
The Node.js server and client can also be used on any other system architecture.
Use the folder 'webradio/package' as base folder for the node application.

##	Server:
The node.js server is a webserver with webpages to select a live stream url.
The server uses Websocket for variable data to/from clients.

For a ESP-based client se the websocket example in this library: https://github.com/palsbo/espaudio

