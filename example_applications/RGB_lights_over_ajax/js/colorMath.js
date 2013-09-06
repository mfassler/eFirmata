

function xy2theta(x, y) {
	var theta;

	if (x == 0) {
		if (y == 0) {  // Dead center; theta is not defined
			theta = 0;
		} else if (y > 0) {
			// Pointing straight up
			theta = Math.PI / 2;
		} else {
			// Pointing straight down
			theta = 3 * Math.PI / 2;
		}
	} else {
		theta = Math.atan(y/x);

		if (x > 0) {
			if (y < 0) { // Quadrant IV
				theta += 2 * Math.PI;
			}
		} else {
			// Quadrants II and III
			theta += Math.PI;
		}
	}

	return theta;
}



function thetaToRGB(theta) {

	// Circles repeat themselves every 2*Pi:
	theta = theta % (2 * Math.PI);

	// Divide the circle into six regions:
	//var H = theta * (6 / (2*Math.PI));
	var H = theta * (3 / Math.PI);

	var r,g,b;

	// We have 6 zones:
	switch(Math.floor(H)){
		case 0:
			r = 1;
			g = H;
			b = 0;
			break;
		case 1:
			r = 2 - H;
			g = 1;
			b = 0;
			break;
		case 2:
			r = 0;
			g = 1;
			b = H - 2;
			break;
		case 3:
			r = 0;
			g = 4 - H;
			b = 1;
			break;
		case 4:
			r = H - 4;
			g = 0;
			b = 1;
			break;
		case 5:
			r = 1;
			g = 0;
			b = 6 - H;
			break;
		default:
			console.log('wtf?');
			break;
	}
	return [Math.round(255*r), Math.round(255*g), Math.round(255*b)];
}



