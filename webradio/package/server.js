'use strict';
const webport = process.env.PORT || 82;
const sockport = 81;

const webbase = require("./webbase")
const WebSocket = require('ws');

const www = new webbase({ port: webport, dir: __dirname + '/web' });
const wss = new WebSocket.Server({ port: sockport });

WebSocket.Server.prototype.broadcast = function (msg) {
    this.clients.forEach(function each(client) {
        if (client.readyState === WebSocket.OPEN) client.send(msg);
    });
}

String.prototype.replaceall = function (a, b) {
    var s = this;
    while (s.indexOf(a) >= 0) s = s.replace(a, b);
    return s;
}

var defaults = { 'url': 'http://streammp3.retro-radio.dk/retro-mp3?type=.mp3', 'gain': 50 };

www.onparm('gain', function (idx, val) {
    console.log("Newgain ", val);
    var ar = {};
    ar.gain = val;
    wss.broadcast(JSON.stringify(ar));
})

www.onparm('url', function (idx, val) {
    console.log("NewUrl ", val);
    var ar = {};
    ar.url = val;
    defaults.url = val;
    wss.broadcast(JSON.stringify(ar));
});

wss.on('connection', function connection(ws) {
    ws.on('message', function incoming(message) {
        try {
            var ar = JSON.parse(message);
            for (var key in ar) {
                defaults[key] = ar[key];
            }
            wss.broadcast(message);
        } catch (err) { console.log("invalid data", message); };
    });
    console.log("Connected to", ws._socket.remoteAddress);
    ws.send(JSON.stringify(defaults));
});