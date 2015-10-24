Pebble.addEventListener('ready', function(){
  console.log('PebbleKit JS Ready!');
});

Pebble.addEventListener('showConfiguration', function() {
  var url = 'https://protome.github.io/PebbleBorder/';
  console.log('Showing configuration page: ' + url);
  
  Pebble.openURL(url);
});

Pebble.addEventListener('webviewclosed', function(e){
  var configData = JSON.parse(decodeURIComponent(e.response));
  console.log('Configuration page returned: ' + JSON.stringify(configData));
  var dict = {};
  dict.KEY_BACKGROUND_COLOUR = parseInt(configData.backgroundColor, 16);
  dict.KEY_TEXT_COLOUR = parseInt(configData.textColor, 16);
  dict.KEY_TOP_COLOUR = parseInt(configData.topColor, 16);
  dict.KEY_LEFT_COLOUR = parseInt(configData.leftColor, 16);
  dict.KEY_RIGHT_COLOUR = parseInt(configData.rightColor, 16);
  dict.KEY_BOTTOM_COLOUR = parseInt(configData.bottomColor, 16);

    Pebble.sendAppMessage( dict,
      function() {
      console.log('Send successful!');
    }, 
      function() {
      console.log('send failed :(');
    });
});