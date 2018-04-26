var express = require('express');
const http = require('http');
const url = require('url');
var app = express()
var onlist = {};
var parmlist = {};

var _options = {
    doc: '/index.html',
    dir: __dirname,
    port: 80,
    url: null
}

//  todo!   must be implemented in class
function checkArgs(req, res) {
    for (rid in req.query) {
        for (id in parmlist) {
            if (id == rid) parmlist[id](req.params[0], req.query[rid]);
        }
    }
}

app.get(/^(.+)$/, function (req, res) {
    if (req.params[0] == '/') req.params[0] = _options.doc;
    checkArgs(req, res);
    res.sendFile(_options.dir + req.params[0]);
});

function handleStream(ar) {
    for (id in ar) {
        for (on in onlist) {
            if (id == on) onlist[on](ar[id])
        }
    }
}

class webbase {
    constructor(options) {
        if (options != undefined) {
            console.log()
            for (var opt in options) {
                _options[opt] = options[opt];
            }
        }
        const server = http.createServer(app);
        var tempopt;
        if (!_options.url) tempopt = { server };
        else {
            var ar = _options.url.split(':');
            console.log(ar);
            tempopt = { server: ar[0], port: ar[1] };
        }
        server.listen(_options.port, function () {
            handleStream({ "listen": _options.port })
        })
    }

    on(id, func) {
        onlist[id] = func;
    }

    onparm(id, func) {
        parmlist[id] = func;
    }
}

module.exports = webbase;
