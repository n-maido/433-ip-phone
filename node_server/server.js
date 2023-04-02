/**
 * Web server for ip phone
 * Webpage url: 192.168.7.2:8080
 * Launch server with: $ node server.js
 * Modified from the UDP relay server sample code and from assignment 3
 */

var PORT_NUMBER = 8080;


var http = require('http');
var fs   = require('fs');
const { writeFile } = require("fs/promises");
var path = require('path');
var mime = require('mime');

/* 
 * Create the static web server
 */
var server = http.createServer(function(request, response) {
	var filePath = false;
	console.log("received request url: " + request.url);
	
	if (request.url === '/') {
		filePath = 'public/index.html';
		var absPath = './' + filePath;
		serveStatic(response, absPath);
	} else if (request.url === '/saveContact') {
		saveContact(request, response);
	} else if (request.url === '/deleteContact') {
		deleteContact(request, response);
	} else {
		filePath = 'public' + request.url;
		var absPath = './' + filePath;
		serveStatic(response, absPath);
	}
	
	// var absPath = './' + filePath;
	// serveStatic(response, absPath);
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
	console.log("save endpt reached");
	let contactsPath = './public/data/contacts.json';
	request.on('data', async function(data) { 
		console.log("received data: " + data);
		let contact = JSON.parse(data);

		let contactsJSON = fs.readFileSync(contactsPath, 'utf-8');
		let contacts = JSON.parse(contactsJSON);
		contacts.push(contact);

		fs.writeFileSync(contactsPath, JSON.stringify(contacts), 'utf-8');
		response.end();

	});
	response.end();
}

function deleteContact(request, response) {
	console.log("delete endpt reached");
	let contactsPath = './public/data/contacts.json';
	request.on('data', async function(data) { 
		console.log("received data: " + data);
		let address = JSON.parse(data);

		let contactsJSON = fs.readFileSync(contactsPath, 'utf-8');
		let contacts = JSON.parse(contactsJSON);
		contacts = contacts.filter(elem=>elem.sipAddress!==address)

		fs.writeFileSync(contactsPath, JSON.stringify(contacts), 'utf-8');
		response.end();
	});
	response.end();
}


/*
 * Create the beatbox server to listen for the websocket
 */
var beatboxServer = require('./lib/ip_phone_server');
beatboxServer.listen(server);

