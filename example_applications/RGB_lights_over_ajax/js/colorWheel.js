
function updateColorWheel(pageX, pageY) {
	var colorElem = document.getElementById('colorWheel');

	var x = 2*(pageX + window.scrollX - colorElem.offsetLeft - (colorElem.width / 2)) / colorElem.width;
	var y = -2*(pageY + window.scrollY - colorElem.offsetTop - (colorElem.height / 2)) / colorElem.height;

	var r = Math.sqrt(Math.pow(x, 2) + Math.pow(y, 2));
	var theta = xy2theta(x, y);

	r = 1.2 * r - 0.1; // Add some dead zone at the center and the outside

	r = Math.min(1.0, r);
	r = Math.max(0.0, r);
	window.saturation = r;
	window.rgb = thetaToRGB(theta);

	updateSampleAndSendToServer();
}

function updateLightness(pageX, pageY) {
	var lightElem = document.getElementById('lightness');

	var x = (pageX + window.scrollX - lightElem.offsetLeft) / lightElem.width;
	x = 1.2 * x - 0.1; // Add some dead zone at the ends

	window.lightnessV = minMaxRound255(255 * x);

	// Set the background lightness of the color wheel
	var colorElem = document.getElementById('colorWheel');
	var myStyle = 'rgb(' + window.lightnessV + ', ' + window.lightnessV + ', ' + window.lightnessV + ')';
	colorElem.style.background = myStyle;

	updateSampleAndSendToServer();
}


window.rgb = [255, 255, 255];
window.saturation = 1.0;
window.lightnessV = 255;


function minMaxRound255(x) {
	if (isNaN(x)) {
		console.log('ERROR: not a number');
		return 0;
	}
	x = Math.round(x);
	x = Math.min(255, x);
	x = Math.max(0, x);
	return x;
}


function hexFormatColors(red, green, blue) {
	// Return an HTML-style color, like "#80ff00"
	var str = '#';
	if (red > 15) {
		str += red.toString(16);
	} else {
		str += '0' + red.toString(16);
	}

	if (green > 15) {
		str += green.toString(16);
	} else {
		str += '0' + green.toString(16);
	}

	if (blue > 15) {
		str += blue.toString(16);
	} else {
		str += '0' + blue.toString(16);
	}

	return str;
}


function updateSwatch(red, green, blue) {
	var swatchDiv = document.getElementById('swatch');
	swatchDiv.style.backgroundColor = 'rgb(' + red + ', ' + green + ', ' + blue + ')';

	var swatchTextDiv = document.getElementById('swatchText');
	swatchTextDiv.innerText = hexFormatColors(red, green, blue);

/*
	// Font color against background color:
	if ((red + green + blue) > 400) {
		swatchDiv.style.color = 'black';
	} else {
		swatchDiv.style.color = 'white';
	}
*/
}


function updateSampleAndSendToServer() {
	var cf = window.saturation;
	var lf = 1 - window.saturation
	var red = cf * window.rgb[0] + lf * window.lightnessV;
	var green = cf * window.rgb[1] + lf * window.lightnessV;
	var blue = cf * window.rgb[2] + lf * window.lightnessV;

	red = minMaxRound255(red);
	green = minMaxRound255(green);
	blue = minMaxRound255(blue);

	updateSwatch(red, green, blue);

	sendRGBtoServer(red, green, blue);
}

