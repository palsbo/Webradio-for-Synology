/*
  options for constructor:
    port: webserver port (default 80)
    url: <url for websocket server incl. port ('localhost:81')
    OBS! if url is omitted (default), websocket will be on same location and port as webserver
    dir: <base folder (relative to server folder) where webpages are located. default __dirname (server folder)
    doc: document to be used, if no document is givne in web-url. default is '/index.html'

    if the web url contains parameters, an event can be raised for each value.
    giving the url: http://server/index.html?name=erik the name will raise the following callback
    <serverbase obj>.onparm('name', func) will call the function 'func(page, value)' wher page is the name of the requested page, value is the value of the parameter
    data from web-socket client is in JSON format;

    data from websocket client is expected as a JSON msg. ex. "{'url': 'http://myserver.com'}"
    to register a callback for 'url' do this:
    <serverbase obj>.on('url', func) wher func is function func(data) {} (data is the value of 'url')
    
*/
const serverbase = require("./serverbase");
const radio = require("radio-stream");

var ww = new serverbase({ port: 81, dir: __dirname + '/web' });

var currentUrl = "";
var myInfo = {};

String.prototype.replaceall = function (a, b) {
    var s = this;
    while (s.indexOf(a) >= 0) s = s.replace(a, b);
    return s;
}

function toar(s) {
    s = s.replaceall('\u0000', '').replaceall('\x0D\x0A','').replaceall('x27','');
    if (s.substr(s.length - 1) == ";") s = s.substr(0, s.length - 1);
    var ar = s.split(";");
    var ar2 = {};
    for (i in ar) {
        var pair = ar[i].split("=");
        var val = String(pair[1]);
        if (val[0] == "'") val = val.substr(1);
        if (val[val.length - 1] == "'") val = val.substr(0, val.length - 1);
        ar2[pair[0]] = val;
    }
    return ar2;
}

function bcurl(data) {
    currentUrl = data;
    console.log("Now broadcasting url:", currentUrl);
    var ar = {};
    ar['url'] = currentUrl;
    for (key in myInfo) myInfo[key] = '';
    ww.broadcast(JSON.stringify(ar));
    ww.broadcast(JSON.stringify(myInfo));
    var stream = new radio.createReadStream(currentUrl)
    stream.on("error", function () {
        console.log("Ups! - Stream error");
    })
    stream.on("connect", function () {
        var s = stream.headers['ice-audio-info'].replaceall('ice-', '');
        var ar = toar(s);
        for (key in ar) myInfo[key] = ar[key];
        ww.broadcast(JSON.stringify(myInfo));
        stream.on("metadata", function (title) {
            var ar = toar(title);
            for (key in ar) myInfo[key] = ar[key];
            ww.broadcast(JSON.stringify(myInfo));
        });
    });
}

ww.begin();
ww.onparm('url', function (doc, data) {     //  action if url contains parameter 'url' returns document requested and value of parameter
    console.log("Value of parmeter 'url' in page '%s' is '%s'", doc, data);
})

ww.on('url', bcurl);    //  action if websocket receives 'url' parameter.

ww.on('ping', function (data) {
    console.log("ping");
})

ww.on('connect', function (ws) {    //  action when a web-client is connected. the socket object is returned
    console.log("Now connected %s\n - sending %s", ws._socket.remoteAddress, currentUrl);
    var ar = {};
    ar['url'] = currentUrl;
    ws.send(JSON.stringify(ar));
    ws.send(JSON.stringify(myInfo));
})

ww.on('listen', function (webport) {   //  action when web-server start to listen.
    console.log('Listening on port %s', webport);
    bcurl("http://streammp3.retro-radio.dk/retro-mp3?type=.mp3");
})
