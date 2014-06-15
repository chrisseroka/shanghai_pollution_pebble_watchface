function fetch_pollution_data(pos) {
  var req = new XMLHttpRequest(),
  version = Date.now(),
  token = 'XAs1txvhW9Y3rmxqK4zA',
  city = 'shanghai';
  req.open('GET', 'http://www.pm25.in/api/querys/pm2_5.json?city='+city+'&token='+token+'&stations=no', true);

  req.onload = function(e) {
  	if (req.readyState == 4 && req.status == 200) {
      if (req.status == 200) {
        var response = JSON.parse(req.responseText);
        console.log('response received:');
        console.log(JSON.stringify(response));
        var pm25 = response[0].pm2_5;
        console.log('pm25:'+pm25);
        Pebble.sendAppMessage({"pm25": ""+pm25+""});
      } else {
      	console.log("Error");
      }
    }
  }
  req.send(null);
}

Pebble.addEventListener("ready", function(e) {
  console.log('Starting Shanghai Pollution Watchface');
  fetch_pollution_data();
});
