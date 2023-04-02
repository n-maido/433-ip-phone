"use strict";
/**
 * Client-side interactions with the browser
 * Initiate a call by entering a sip address or calling a saved contact
 * Written by Nhi Mai-Do, modified from the UDP lecture code and Assignment 3
 */

var callInProgress = false;
var phoneAlive = false;
var webAlive = false;

var savedContacts = [];

const Status = {
	None: 0,
	Incoming: 1,
	Ongoing: 2,
	Error: 3
}

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

// Make connection to server when web page is fully loaded.
var socket = io.connect();

$(document).ready(function() {
	// populate saved contacts table
	loadContacts();

	// Call a sip address
	$('#callBtn').click(function(){
		var callee = $('#sipInput').val();

		if (callee === '') {
			window.alert("Please fill out the SIP address before proceeding");
			return;
		}

		// validate sip uri format
		if (!validateSIP(callee)) {
			window.alert(`Invalid SIP URI format.\nPlease enter a URI in the format"sip:user@host:optional_port"\nwhere "host" is an IP address or domain`);
			return;
		}
		makeCall(callee);
	});

	// Hang up a call
	$('#ongoingHangUpBtn').click(function() {
		// do we need to supply addresses to hang up a call?
		// remove if not needed
		var calleeText = $('#ongoingText').text().split(" ");
		console.log(`calleeText = ${calleeText}`);
		var callee = calleeText[1];

		sendCommandViaUDP(`end_call=${callee}`);

		socket.on('end_call', function(result) {
			console.log(result);
			if (result.toLowerCase() === "error") {
				callInProgress = true;
				setStatusBox(Status.Error, result);
			} 	
		});
	});

	$('#incomingHangUpBtn').click(function() {
		// do we need to supply addresses to hang up a call?
		// remove if not needed
		var calleeText = $('#incomingText').text().split(" ");
		console.log(`calleeText = ${calleeText}`);
		var callee = calleeText[3];

		sendCommandViaUDP(`end_call=${callee}`); //should we use end_call or have a new cmd reject call?

		socket.on('end_call', function(result) {
			if (result.toLowerCase() === "error") {
				callInProgress = true;
				setStatusBox(Status.Error, result);
			} 	
		});
	});

	// Pick up an incoming call
	$('#incomingPickUpBtn').click(function() {
		// do we need to supply addresses to hang up a call?
		// remove if not needed
		var calleeText = $('#incomingText').text().split(" ");
		console.log(`calleeText = ${calleeText}`);
		var callee = calleeText[3];

		sendCommandViaUDP(`pick_up=${callee}`); //should we use end_call or have a new cmd reject call?

		socket.on('pick_up', function(result) {
			console.log(result);
			if (result.toLowerCase() === "error") {
				callInProgress = true;
				setStatusBox(Status.Error, result);
			} 	
		});
	});


	// Call a saved contact
	// $("#contactsTable .tableCallBtn").click(function() {
	$('#contactsTable').on('click', '.tableCallBtn', function() {
		console.log("contact clicked");
		// get the row
		var row = $(this).closest("tr");

		// callee address is the 2nd td in the row
		var callee = row.find("td:nth-child(2)").text(); 
		
		makeCall(callee);
	});

	// Add a contact
	$('#addContactBtn').click(function(){
		let name = $('#contactNameInput').val();
		let address = $('#contactAddressInput').val();

		if (name === '' || address === '') {
			window.alert("Please fill out all fields before proceeding");
			return;
		}

		// validate sip uri format
		if (!validateSIP(address)) {
			window.alert(`Invalid SIP URI format.
			Please enter a URI in the format "sip:user@host:optional_port"
			where "host" is an IP address or domain`);
			return;
		}

		let contact = {
			name: name,
			sipAddress: address
		}

		sendCommandViaUDP(`add_contact=${name}&${address}`)
		socket.on('add_contact', function(result) {
			if (result.toLowerCase() !== 'success') {
				console.log(result);
			}
		});

		savedContacts.push(contact);
		appendContact(name, address);

		$('#contactNameInput').val('');
		$('#contactAddressInput').val('');

		// save to json file
		console.log("sending post json request");
		$.post('/saveContact', JSON.stringify(contact), function(response) {
			console.log('Successfully saved contact to file');
			console.log(response);
		}, 'json');
	});

	// Volume control
	$('#volumeDown').click(function(){
		var volume = parseInt($('#curVolume').val());
		volume = isNaN(volume) ? -1 : volume - 5;

		if (volume >= 0 && volume <= 100) {
			sendCommandViaUDP(`set_volume ${volume}`);
			socket.on('volume', function(result) {
				if (result.toLowerCase() === "error") {
					callInProgress = true;
					setStatusBox(Status.Error, result);
				} 
			});
		}		
	});
	$('#volumeUp').click(function(){
		var volume = parseInt($('#curVolume').val());
		volume = isNaN(volume) ? -1 : volume + 5;
		console.log("vol = " + volume);

		if (volume >= 0 && volume <= 100) {
			sendCommandViaUDP(`set_volume ${volume}`);
			socket.on('volume', function(result) {
				if (result.toLowerCase() === "error") {
					callInProgress = true;
					setStatusBox(Status.Error, result);
				} 
			});
		}
	});

	// Gain control
	$('#gainDown').click(function(){
		var gain = parseInt($('#curGain').val());
		gain = isNaN(gain) ? -1 : gain - 5;

		if (gain >= 0 && gain <= 100) {
			sendCommandViaUDP(`set_gain ${gain}`);
			socket.on('gain', function(result) {
				if (result.toLowerCase() === "error") {
					callInProgress = true;
					setStatusBox(Status.Error, result);
				} 
			});
		}
	});

	$('#gainUp').click(function(){
		var gain = parseInt($('#curGain').val());
		gain = isNaN(gain) ? -1 : gain + 5;

		if (gain >= 0 && gain <= 100) {
			sendCommandViaUDP(`set_gain ${gain}`);
			socket.on('gain', function(result) {
				if (result.toLowerCase() === "error") {
					callInProgress = true;
					setStatusBox(Status.Error, result);
				} 
			});
		}
	});

	// Stop program
	$('#stop').click(function(){
		shutdown();
	});
});


function sendCommandViaUDP(message) {
	socket.emit('udpCommand', message);
};

/**
 * Get the call status
 * Called every 500ms
 */
function call_status(){
	sendCommandViaUDP("call_status");
	socket.on('call_status', (result) => {
		switch(result.status) {
			case Status.Incoming:
				callInProgress = true;
				setStatusBox(Status.Incoming, result);
				break;
			case Status.Ongoing:
				callInProgress = true;
				setStatusBox(Status.Ongoing, result);
				break;
			case Status.Error:
				callInProgress = false;
				setStatusBox(Status.Error, result);
				break;
			case Status.None:
				callInProgress = false;
				setStatusBox(Status.None, "");
				break;
		}
	})
};

setInterval(() => {call_status()}, 500);

/**
 * Called every 1s
 * Checks if the web server and c application is responsive 
 */
function checkAlive() {
	// Check if the server is responding
	socket.emit('web_alive');

	// If the server hasn't responded for 800ms, show error
	var serverErrorTimer = setTimeout(function() {
		webAlive = false;
		$('#webStatus').text('NOT RUNNING');
		$('#webStatus').css('color', 'red');

	}, 800);

	socket.on('web_alive', function(){
		clearTimeout(serverErrorTimer);
		webAlive = true;
		$('#webStatus').text('RUNNING');
		$('#webStatus').css('color', 'green');
	})

	// Check if the udp server is responding
	// Server sets a timeout after sending a udp msg to the c app
	// If it receives a response, it notifies us
	socket.on('udpError', function(){
		phoneAlive = false;
		$('#phoneStatus').text('OFF');
		$('#phoneStatus').css('color', 'red');
	})

	socket.on('udpSuccess', function(){
		phoneAlive = true;
		$('#phoneStatus').text('ON');
		$('#phoneStatus').css('color', 'green');
	})
}

setInterval(() => {checkAlive()}, 1000);

/**
 * Sends a msg to the C app to call the provided sip address
 */
function makeCall(callee) {
	if (callInProgress) {
		window.alert("There is call in progress. Please hang up and try again.");
		return;
	}

	sendCommandViaUDP(`make_call=${callee}`);

	socket.on('make_call', function(result) {
		console.log(result);
		if (result.toLowerCase() === "error") {
			callInProgress = false;
			setStatusBox(Status.Error, result);
		} 	
	});
}

/**
 * Validates the SIP URI
 * Should be in the format "sip:user@host:optional_port"
 * host = IP address or domain
 */
function validateSIP(address) {
	// regex pattern from: https://regex101.com/r/5OJJxF/1/ 
	let sipREGEX = /^(sip?):([^@\n]+)(?:@(.+))?$/;
	return sipREGEX.test(address);
}

function setStatusBox(status, data) {
	switch (status) {
		case Status.Incoming:
			// show incoming box, hide the others
			console.log("showing incoming status");
			$('#ongoingBox').hide();
			$('#errorBox').hide();
			$('#incomingText').text(`Incoming call from ${data.address}`);
			$('#incomingBox').show();
			break;
		case Status.Ongoing:
			$('#incoming').hide();
			$('#errorBox').hide();
			$('#ongoingText').text(`Calling ${data.address}`);
			$('#curVolume').val(data.vol);
			$('#curGain').val(data.gain);
			$('#ongoingBox').show();
			break;
		case Status.Error:
			$('#incomingBox').hide();
			$('#ongoingBox').hide();	
			$('#errorText').text(`Error: ${data.error}`);
			$('#errorBox').show();
			break;
		case Status.None:
			$('#incomingBox').hide();
			$('#ongoingBox').hide();	
			$('#errorBox').hide();	
			break;	
	}
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

// Appends a contact to the contacts table
function appendContact(name, address) {
	let tableRow = `<tr>
		<td>${name}</td>
		<td>${address}</td>
		<td>
			<input type="button" class="tableCallBtn" value="Call">
		</td>
	</tr>`

	$('#contactsTable tr:last').after(tableRow);
}

function loadContacts() {
	// load from json file
	$.getJSON("data/contacts.json", function(data){
		data.forEach((contact) => {
			savedContacts.push(contact);
			appendContact(contact.name, contact.sipAddress);

			sendCommandViaUDP(`add_contact=${contact.name}&${contact.sipAddress}`);

			socket.on('add_contact', function(result) {
				if (result.toLowerCase() !== 'success') {
					console.log(result);
				}
			});
		});
	}).fail(function(){
		console.log("Error loading saved contacts.");
	});
}

function shutdown() {
	// save contacts to json file

	// stop server
	sendCommandViaUDP("stop");

	// show status: app is unavailable
}
