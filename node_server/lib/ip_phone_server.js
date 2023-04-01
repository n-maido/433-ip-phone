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
	socket.on('web_alive', function() {
		socket.emit('web_alive');
	})

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

		// Set a timeout for the udp response
		var udpErrorTimer = setTimeout(function() {
			socket.emit("udpError");
		}, 800);

		/** 
		 * Handle an incoming message from the udp server
		 * Response messages are json objects in the format: 
		 * {
		 *     "msgType": "commandName",
		 *     "content": "msgContent"
		 * }
		 */  
		client.on('message', function (message, remote) {	
			clearTimeout(udpErrorTimer);
			socket.emit('udpSuccess');

			console.log("UDP Client: message Rx" + remote.address + ':' + remote.port +' - ' + message);

			var reply = JSON.parse(message.toString('utf8'));
			
			switch(reply.msgType) {
				case "new_user":
					socket.emit('new_user', reply.content);
					break;
				case "call_status":
					socket.emit('call_status', reply.content);
					break;
				case "call_started":
					console.log("received call_starting. emitting to socket");
					socket.emit('call_started', reply.content.address);
					break;
				case "make_call":
					socket.emit('make_call', reply.content);
					break;
				case "end_call":
					socket.emit('end_call', reply.content);
					break;
				case "pick_up":
					socket.emit('pick_up', reply.content);
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