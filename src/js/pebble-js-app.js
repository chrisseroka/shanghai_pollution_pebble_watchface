function fetch_pollution_data(pos) {
  var req = new XMLHttpRequest(),
  city = localStorage.getItem("city") ? localStorage.getItem("city") : "beijing",
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
    Pebble.openURL('https://cdn.rawgit.com/lvx3/shanghai_pollution_pebble_watchface/14a8d83283b5e95f4349b82a1b416385b27624dd/configurable.html');
});

Pebble.addEventListener("webviewclosed", function(e) {
  console.log("Response: " + e);
  var options = JSON.parse(decodeURIComponent(e.response));
  console.log(JSON.stringify(options));
  localStorage.setItem("city", options.city.toLowerCase());
  fetch_pollution_data();
});
