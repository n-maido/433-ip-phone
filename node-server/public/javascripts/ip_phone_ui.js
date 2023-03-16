"use strict";
/**
 * Client-side interactions with the browser
 * Initiate a call by entering a sip address or calling a saved contact
 * Written by Nhi Mai-Do, modified from the UDP lecture code and Assignment 3
 */


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
function showCallBox() {
	$('#call-box').show();
}

// Hides the call box when the user hangs up
function hideCallBox() {
	$('#call-box').hide();
}
