var city = 'beijing';
function fetch_pollution_data(pos) {
  var req = new XMLHttpRequest(),
  version = Date.now(),
  token = 'XAs1txvhW9Y3rmxqK4zA';
  req.open('GET', 'http://www.pm25.in/api/querys/pm2_5.json?city='+city+'&token='+token+'&stations=no', true);

  req.onload = function(e) {
    if (req.readyState == 4 && req.status == 200) {
      if (req.status == 200) {
        var response = JSON.parse(req.responseText);
        console.log('response received:');
        console.log(JSON.stringify(response));
        var pm25 = response[0].pm2_5;
        var pm2524h = response[0].pm2_5_24h;
        var aqi = response[0].aqi;
        console.log('pm25:'+pm25);
        Pebble.sendAppMessage({"pm25": ""+pm25+"","pm2524h":""+pm2524h+"","aqi":""+aqi+""});
      } else {
        console.log("Error");
      }
    }
  };
  req.send(null);
}

Pebble.addEventListener("ready", function(e) {
  fetch_pollution_data();
});

Pebble.addEventListener("appmessage", function(e) {
  fetch_pollution_data();
});

Pebble.addEventListener("showConfiguration", function(e) {
  Pebble.openURL('https://raw.githubusercontent.com/lvx3/shanghai_pollution_pebble_watchface/master/configurable.html');
});

Pebble.addEventListener("webviewclosed", function(e) {
  var options = JSON.parse(decodeURIComponent(e.response));
  city = options.city;
});
