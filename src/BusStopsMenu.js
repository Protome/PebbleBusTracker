module.exports = { 
	showMenu: function() {
		var UI = require('ui');
		var ajax = require('ajax');
		var MD5 = require('MD5');
		var accel = require('ui/accel');
		var myAPIKey = 'HEZ2Z4NQF4DZWAXI84IJDYU26';
		var currentLatitude = 0;
		var currentLongitude = 0;

		var menu = new UI.Menu({
			backgroundColor: 'white',
			textColor: 'blue',
			highlightBackgroundColor: 'blue',
			highlightTextColor: 'white',
			sections: [{
				items: [{
					title: 'Loading'
				}],
			}]
		});

		var locationOptions = {
			enableHighAccuracy: true, 
			maximumAge: 10000, 
			timeout: 10000
		};

		menu.show();
		accel.init();

		menu.on('accelTap', function(e) {
			updateMenuContent();
		});

		updateMenuContent();

		function locationSuccess(pos) {
			//55.94611043866182 lon= -3.211599140737132
// 			currentLatitude = pos.coords.latitude;
// 			currentLongitude = pos.coords.longitude;
			currentLatitude = 55.94611043866182;
			currentLongitude = -3.211599140737132;
			
			console.log('lat= ' + pos.coords.latitude + ' lon= ' + pos.coords.longitude);
		}

		function locationError(err) {
			console.log('location error (' + err.code + '): ' + err.message);
		}

		Pebble.addEventListener('ready',
														function(e) {
															// Request current position
															navigator.geolocation.getCurrentPosition(locationSuccess, locationError, locationOptions);
														}
													 );

		function updateMenuContent() {
			// Construct URL
			var today = new Date();
			var dd = today.getDate();
			var mm = today.getMonth()+1; //January is 0!
			var yyyy = today.getFullYear();
			var hh = today.getHours();

			if(dd<10){
				dd='0'+dd;
			} 
			if(mm<10){
				mm='0'+mm;
			}
			if(hh<10){
				hh='0'+hh;
			}

			var todayKey = yyyy+mm+dd+hh;
			var apiKey = MD5(myAPIKey + todayKey);
			var URL = 'http://ws.mybustracker.co.uk/?module=json&key=' + apiKey + '&function=getBusStops';
			console.log(URL);
			// Make the request
			ajax(
				{
					url: URL,
					type: 'json'
				},
				function(data) {
					// Success!
					console.log("Successfully fetched bus data!");

					var distanceModifier = 0.004;
					var items = parseIntoItems(data, distanceModifier);

					if (items.length > 0) {
						menu.items(0,items);
					}
					else {
						updateMenuContent();
					}
				},
				function(error) {
					// Failure!
					console.log('Failed fetching bus data: ' + error);
				}
			);

			function parseIntoItems(data, distance) {
				var items = [];

				// Extract data
				var stops = data.busStops;

				navigator.geolocation.getCurrentPosition(locationSuccess, locationError, locationOptions);

				var closeStops = [];
				for (var x = 0; x < stops.length; x++) {
					if ((stops[x].x < (currentLatitude + distance)) && (stops[x].x > (currentLatitude - distance))) {
						if ((stops[x].y < (currentLongitude + distance)) && (stops[x].y > (currentLongitude - distance))) {
							closeStops.push(stops[x]);
						}
					}
				}

				for (var index = 0; index < closeStops.length; index++) {
					var item = {
						title: closeStops[index].name,
						subtitle: 'Services: ' + closeStops[index].services
					};
					items.push(item);
				}

				return items;
			}
		}
	}
};