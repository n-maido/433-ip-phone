"use strict";
/**
 * Client-side interactions with the browser
 * Initiate a call by entering a sip address or calling a saved contact
 * Written by Nhi Mai-Do, modified from the UDP lecture code and Assignment 3
 */

var callInProgress = false;
var username = "user";
var mySipAddress = "sip:address"; //Later, remove?

var savedContacts = [{
		name: "Nhi 1",
		sipAddress: "sip:nhi@192.168.1.54"
	},
	{
		name: "Nhi 2",
		sipAddress: "sip:nhi@192.168.1.55"
	}
];

var successToast = Toastify({
	text: "Success",
	duration: 1500,
	newWindow: false,
	close: true,
	gravity: "top", // `top` or `bottom`
	position: "center", // `left`, `center` or `right`
	stopOnFocus: true, // Prevents dismissing of toast on hover
	style: {
	  background: "#2ecc71",
	  fontFamily: "sans-serif"
	},
});

var errorToast = Toastify({
	text: "Error",
	duration: 1500,
	newWindow: false,
	close: true,
	gravity: "top", // `top` or `bottom`
	position: "center", // `left`, `center` or `right`
	stopOnFocus: true, // Prevents dismissing of toast on hover
	style: {
	  background: "#e74c3c",
	  fontFamily: "sans-serif"
	},
});

/**
 * Get the call status
 * Called every 500ms
 */
function call_status(){
	sendCommandViaUDP("call_stats");
	socket.on('call_stats', (result) => {
	
	})
};

// Make connection to server when web page is fully loaded.
var socket = io.connect();

$(document).ready(function() {
	// show the onboarding prompt
	onboardUser();

	// populate saved contacts table
	loadContacts();

	// Call a sip address
	$('#callBtn').click(function(){
		var callee = $('#sipInput').val();
		makeCall(callee);
	});

	// Hang up a call
	$('#hangUpBtn').click(function() {
		// do we need to supply addresses to hang up a call?
		// remove if not needed
		var calleeText = $('#call-box-text').text().split(" ");
		console.log(`calleeText = ${calleeText}`);
		var callee = calleeText[1];

		sendCommandViaUDP(`end_call=${callee}`);

		hideCallBox();

		socket.on('end_call', function(result) {
			console.log(result);
			callInProgress = false;
		});
	});

	// Call a saved contact
	$(".tableCallBtn").click(function() {
		// get the row
		var row = $(this).closest("tr");

		// callee address is the 2nd td in the row
		var callee = row.find("td:nth-child(2)").text(); 
		
		makeCall(callee);
	});

	// Stop program
	$('#stop').click(function(){
		sendCommandViaUDP("stop");
	});
});


function sendCommandViaUDP(message) {
	socket.emit('udpCommand', message);
};

// setInterval(() => {heartbeat_info()}, 1000);

/**
 * Sends a msg to the C app to call the provided sip address
 */
function makeCall(callee) {
	if (callInProgress) {
		window.alert("There is call in progress. Please hang up and try again.");
		return;
	}
	sendCommandViaUDP(`make_call=${callee}`);

	showCallBox(callee);

	socket.on('make_call', function(result) {
		console.log(result);
		callInProgress = true;
	});
}

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

function onboardUser() {
	username = window.prompt("Welcome to the 433 IP Phone! \nPlease enter a username:");
	sendCommandViaUDP(`new_user=${username}`);
	socket.on('new_user', function(result) {
		if (result.toLowerCase() === 'success') {
			successToast.options.text = "Successfully joined the network";
			successToast.showToast();
			successToast.options.text = "Success"
		} else {
			errorToast.options.text = "Error joining network"
			errorToast.showToast();
			errorToast.options.text = "Error"
		}
	});
}

function loadContacts() {
	let count = 0;

	savedContacts.forEach((contact) => {
		let tableRow = `<tr>
			<td>${contact.name}</td>
			<td>${contact.sipAddress}</td>
			<td>
				<input type="button" class="tableCallBtn" value="Call">
			</td>
		</tr>`

		$('#contactsTable tr:last').after(tableRow);
		count++;
	});

}
