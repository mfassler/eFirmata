
var colorWrapperElem = document.getElementById('colorWheelWrapper');
var colorElem = document.getElementById('colorWheel');
var lightElem = document.getElementById('lightness');

window.trackColorWheel = false;
window.trackLightness = false;


// Disable the default drag-n-drop behavior of the imgs:
colorElem.ondragstart = function() {
	return false;
};
lightElem.ondragstart = function() {
	return false;
};

function isTouchDevice() {
	// Stackoverflow #4817029
	return !!('ontouchstart' in window) // works on most browsers 
		|| !!('onmsgesturechange' in window); // works on ie10
}


if (isTouchDevice()) { // This is a touch device (e.g., a smartphone)
	colorElem.ontouchstart = function (evt) {
		window.trackColorWheel = true;
		window.trackLightness = false;
		updateColorWheel(evt.touches[0].clientX, evt.touches[0].clientY);
	};

	colorElem.ontouchmove = function (evt) {
		if (window.trackColorWheel) {
			updateColorWheel(evt.touches[0].clientX, evt.touches[0].clientY);
			evt.preventDefault();
		}
	};

	lightElem.ontouchstart = function (evt) {
		window.trackLightness = true;
		window.trackColorWheel = false;
		updateLightness(evt.touches[0].clientX, evt.touches[0].clientY);
	};

	lightElem.ontouchmove = function (evt) {
		if (window.trackLightness) {
			updateLightness(evt.touches[0].clientX, evt.touches[0].clientY);
			evt.preventDefault();
		}
	};

	window.ontouchend = function () {
		window.trackColorWheel = false;
		window.trackLightness = false;
	};

} else { // This is a mouse device (e.g., a desktop computer)
	colorElem.onmousedown = function (evt) {
		if (evt.button == 0) {
			window.trackColorWheel = true;
			window.trackLightness = false;
			updateColorWheel(evt.x, evt.y);
		}
	};

	colorElem.onmousemove = function (evt) {
		if (window.trackColorWheel) {
			updateColorWheel(evt.x, evt.y);
		}
	};

	lightElem.onmousedown = function (evt) {
		if (evt.button == 0) {
			window.trackLightness = true;
			window.trackColorWheel = false;
			updateLightness(evt.x, evt.y);
		}
	};

	lightElem.onmousemove = function (evt) {
		if (window.trackLightness) {
			updateLightness(evt.x, evt.y);
		}
	};

	window.onmouseup = function (evt) {
		if (evt.button == 0) {
			window.trackColorWheel = false;
			window.trackLightness = false;
		}
	};
}

