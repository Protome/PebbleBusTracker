var UI = require('ui');
var busStopsMenu = require('BusStopsMenu');

var main = new UI.Card({
	title: 'BusTracker',
	subtitle: 'API Test',
	body: '',
	subtitleColor: 'indigo', // Named colors
	bodyColor: '#9a0036' // Hex colors
});

main.show();

main.on('click', 'select', function(e) {
	busStopsMenu.showMenu();
});

