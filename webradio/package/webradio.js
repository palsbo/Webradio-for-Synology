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

var currentUrl;
var currentTitle = "";
var currentInfo = "";

String.prototype.replaceall = function (a, b) {
    var s = this;
    while (s.indexOf(a) >= 0) s = s.replace(a, b);
    return s;
}

function bcurl(data) {
    currentUrl = data;
    currentTitle = "";
    currentInfo = "";
    console.log("Now broadcasting url:", currentUrl);
    ww.broadcast(JSON.stringify({ 'url': currentUrl, 'title': currentTitle, 'info': currentInfo }));
    var stream = new radio.createReadStream(currentUrl)
    stream.on("error", function () {
        currentInfo = "Stream error!";
        ww.broadcast(JSON.stringify({ 'info': currentInfo }));
    })
    stream.on("connect", function () {
        currentInfo = stream.headers['ice-audio-info'].replaceall(';', '<br/>');
        ww.broadcast(JSON.stringify({ 'info': currentInfo }));
        stream.on("metadata", function (title) {
            currentTitle = title.toString().replace('StreamTitle=', '').replace(';StreamUrl=\'\';', '');
            if (currentTitle.indexOf('\'') == 0) currentTitle = currentTitle.substr(1);
            if (currentTitle.lastIndexOf('\'') < currentTitle.length - 3) currentTitle = currentTitle.substr(0, currentTitle.lastIndexOf('\''));
            if (currentTitle == '\';') currentTitle = '';
            ww.broadcast(JSON.stringify({ 'title': currentTitle }));
        });
    });
}

ww.onparm('url', function (doc, data) {     //  action if url contains parameter 'url' returns document requested and value of parameter
    console.log("Value of parmeter 'url' in page '%s' is '%s'", doc, data);
})

ww.on('url', bcurl);    //  action if websocket receives 'url' parameter.

ww.on('connect', function (ws) {    //  action when a web-client is connected. the socket object is returned
    console.log("Now connected - sending %s", currentUrl);
    ws.send(JSON.stringify({ 'url': currentUrl, 'info': "Welcome to webradio" }));
})

ww.on('listen', function (webport) {   //  action when web-server start to listen.
    console.log('Listening on port %s', webport);
    bcurl("http://live-icy.gss.dr.dk:8000/A/A04H.mp3");
})
