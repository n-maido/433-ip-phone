/**
 * Web server for ip phone
 * Webpage url: 192.168.7.2:8080
 * Launch server with: $ node server.js
 * Modified from the UDP relay server sample code and from assignment 3
 */

var PORT_NUMBER = 8080;


var http = require('http');
var fs   = require('fs');
var path = require('path');
var mime = require('mime');

/* 
 * Create the static web server
 */
var server = http.createServer(function(request, response) {
	var filePath = false;
	
	if (request.url == '/') {
		filePath = 'public/index.html';
	} else if (request.url == '/saveContact') {
		saveContact(request, response);
		return;
	} else {
		filePath = 'public' + request.url;
	}
	
	var absPath = './' + filePath;
	serveStatic(response, absPath);
});


server.listen(PORT_NUMBER, function() {
	console.log("Server listening on port " + PORT_NUMBER);
});

function serveStatic(response, absPath) {
	fs.exists(absPath, function(exists) {
		if (exists) {
			fs.readFile(absPath, function(err, data) {
				if (err) {
					send404(response);
				} else {
					sendFile(response, absPath, data);
				}
			});
		} else {
			send404(response);
		}
	});
}

function send404(response) {
	response.writeHead(404, {'Content-Type': 'text/plain'});
	response.write('Error 404: resource not found.');
	response.end();
}

function sendFile(response, filePath, fileContents) {
	response.writeHead(
			200,
			{"content-type": mime.lookup(path.basename(filePath))}
		);
	response.end(fileContents);
}

function saveContact(request, response) {
	let contactsPath = './public/data/contacts.json';
	request.on('data', function(data) { 
		console.log("save endpt reached: " + data);
		let contact = JSON.parse(data);

		fs.readFile(contactsPath, function(err, data) {
			if (err) {
				console.log(err);
				response.send(err);
			} else {
				console.log("read json file");
				let contacts = JSON.parse(data);
				contacts.push(contact);
				console.log(contacts);

				// TODO: file not saving
				fs.writeFile(contactsPath, JSON.stringify(contacts), function(err){
					if (err) {
						console.log(err);
						response.send(err);
					} else {
						console.log("wrote to json file");
					}
				});
			}
		});
	});
	response.end();
}


/*
 * Create the beatbox server to listen for the websocket
 */
var beatboxServer = require('./lib/ip_phone_server');
beatboxServer.listen(server);

