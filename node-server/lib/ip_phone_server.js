"use strict";
/**
 * Respond to commands over a websocket to relay UDP commands to the local beatbox C program
 * Written by Nhi Mai-Do, modified from the UDP lecture code and Assignment 3
 */

var socketio = require('socket.io');
var io;

var dgram = require('dgram');

exports.listen = function(server) {
	io = socketio.listen(server);
	io.set('log level 1');

	io.sockets.on('connection', function(socket) {
		handleCommand(socket);
	});
};

function handleCommand(socket) {
	// Passed string of command to relay
	socket.on('udpCommand', function(data) {
		console.log('udpCommand command: ' + data);

		// Info for connecting to the local process via UDP
		var PORT = 11037;
		var HOST = '127.0.0.1';
		var buffer = new Buffer(data);

		var client = dgram.createSocket('udp4');

		client.on('listening', function () {
			var address = client.address();
			console.log('UDP Client: listening on ' + address.address + ":" + address.port);
		});

		client.send(buffer, 0, buffer.length, PORT, HOST, function(err, bytes) {
			if (err) 
				throw err;
			console.log('UDP message sent to ' + HOST +':'+ PORT);
		});

		/** 
		 * Handle an incoming message from the udp server
		 * Response messages are json objects in the format: 
		 * {
		 *     "msgType": "commandName",
		 *     "content": "msgContent"
		 * }
		 */  
		client.on('message', function (message, remote) {			
			console.log("UDP Client: message Rx" + remote.address + ':' + remote.port +' - ' + message);

			var reply = JSON.parse(message.toString('utf8'));
			switch(reply.msgType) {
				case "heartbeat":
					socket.emit('heartbeat', reply.content);
					break;
				case "make_call":
					socket.emit('make_call', reply.content);
					break;
				case "hang_up":
					socket.emit('hang_up', reply.content);
					break;
			}
			client.close();

		});

		client.on("UDP Client: close", function() {
			console.log("closed");
		});
		client.on("UDP Client: error", function(err) {
			console.log("error: ",err);
		});
	});
};