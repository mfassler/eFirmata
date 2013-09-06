

function ajax(url, params, cb) {

	var req = new XMLHttpRequest();

	req.open('POST', url, true);
	req.setRequestHeader("Content-type", "application/x-www-form-urlencoded");

	req.onreadystatechange = function () {
		if (req.readyState == 4) {
			if (req.status >= 200 && req.status <= 299 || req.status == 304) {
				console.log('good status');
			} else {
				console.log('In ajax(): bad status from webserver');
				console.log('Server HTTP status is: ', req.status);
			} 

			console.log(req.responseText);
		}
	};

	req.send(params);
}


window.lastRGBSend = 0;

function sendRGBtoServer(red, green, blue) {

	// We only update 10 times per second, max:
	var now = +Date.now();
	if ((window.lastRGBSend + now) < 100) {
		return;
	}
	window.lastRGBSend = now;

	var url = 'postRGB.php';
	var params = 'r=' + red + '&g=' + green + '&b=' + blue;

	ajax(url, params, null);
}


