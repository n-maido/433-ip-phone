"use strict";
/**
 * Client-side interactions with the browser
 * Initiate a call by entering a sip address or calling a saved contact
 * Written by Nhi Mai-Do, modified from the UDP lecture code and Assignment 3
 */

var mySipAddress = "sip:address" //Later, remove?

/**
 * Called every second
 * Maybe: get the call statistics?
 */
function heartbeat_info(){
	sendCommandViaUDP("get_info");
	socket.on('heartbeat', (result) => {
	
	})
};

// Make connection to server when web page is fully loaded.
var socket = io.connect();

$(document).ready(function() {
	// Call a sip address
	$('#callBtn').click(function(){
		var callee = $('#sipInput').val();

		sendCommandViaUDP(`{\"cmd\": \"make_call\", \"caller\": \"${mySipAddress}\", \"callee\": \"${callee}\" }`);

		showCallBox(callee);

		socket.on('make_call', function(result) {
			console.log(result);
		});
	});

	// Hang up a call
	$('#hangUpBtn').click(function(){
		// do we need to supply addresses to hang up a call?
		// remove if not needed
		var callee = $('#sipInput').val();

		sendCommandViaUDP(`{\"cmd\": \"hang_up\", \"caller\": \"${mySipAddress}\", \"callee\": \"${callee}\" }`);

		hideCallBox();

		socket.on('hang_up', function(result) {
			console.log(result);
		});
	});

	// TODO: call a saved contact

	// Stop program
	$('#stop').click(function(){
		sendCommandViaUDP("stop");
	});
});

function sendCommandViaUDP(message) {
	socket.emit('udpCommand', message);
};

// setInterval(() => {heartbeat_info()}, 1000);

// Shows the call box when we initiate a call
// User can hang up the call form here
function showCallBox(callee) {
	$('#call-box-text').text(`Calling ${callee}`);
	$('#call-box').show();
}

// Hides the call box when the user hangs up
function hideCallBox() {
	$('#call-box-text').text('');
	$('#call-box').hide();
}
