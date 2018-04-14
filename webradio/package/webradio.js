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
//const radio = require("radio-stream");
var radio = require('node-internet-radio');

var ww = new serverbase({ port: 81, dir: __dirname + '/web' });

var currentUrl = "";

String.prototype.replaceall = function (a, b) {
    var s = this;
    while (s.indexOf(a) >= 0) s = s.replace(a, b);
    return s;
}

setInterval(function() {
    try {
    radio.getStationInfo(currentUrl, function(error, station) {
	//console.log(station);
	var ar = {};
	ar.title = station.title;
	ar.bitrate = station.headers['icy-br'];
	ar.genre = station.headers['icy-genre'];
	ar.name = station.headers['icy-name'];
	ww.broadcast(JSON.stringify(ar));
	console.log(ar);
    }, radio.StreamSource.STREAM);
    } catch (error) {};
},5000);

function newUrl(data) {
    currentUrl = data;
    console.log("Now broadcasting url:", currentUrl);
    var ar = {};
    ar['url'] = currentUrl;
    ww.broadcast(JSON.stringify(ar));
}

ww.begin();
ww.onparm('url', function (doc, data) {     //  action if url contains parameter 'url' returns document requested and value of parameter
    console.log("Value of parmeter 'url' in page '%s' is '%s'", doc, data);
})

ww.on('url', newUrl);    //  action if websocket receives 'url' parameter.

ww.on('ping', function (data) {
    console.log("ping");
})

ww.on('connect', function (ws) {    //  action when a web-client is connected. the socket object is returned
    console.log("Now connected %s\n - sending %s", ws._socket.remoteAddress, currentUrl);
    var ar = {};
    ar['url'] = currentUrl;
    ws.send(JSON.stringify(ar));
})

ww.on('listen', function (webport) {   //  action when web-server start to listen.
    console.log('Listening on port %s', webport);
    newUrl("http://streammp3.retro-radio.dk/retro-mp3?type=.mp3");
})
