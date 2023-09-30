const loadMap = true;

//Socket
const URL = "http://localhost:3000";
const socket = io(URL);

//Map
let map;
let zoom = 16;
let center = { lat: 38.930151042644205, lng: -9.004660423555128 };

let pastPos = [];

let boatMarker;
let flightPath;
let markerArray = [];

//Wind
//let wind = 0;

function initMap() {
  if (!loadMap) return;
  map = new google.maps.Map(document.getElementById("map"), {
    center,
    zoom,
    streetViewControl: false,
  });

  boatMarker = new google.maps.Marker({
    position: center,
    title: "Hello World!",
    icon: "img/nav.png"
  });
  boatMarker.setMap(map);

  flightPath = new google.maps.Polyline({
    path: pastPos,
    geodesic: true,
    strokeColor: "#666688",
    strokeOpacity: 1.0,
    strokeWeight: 2,
  });
  flightPath.setMap(map);

  socket.on('Coords', (c) => {
    map.setCenter(c);
    boatMarker.setPosition(c);

    c.lat += 0.000003
    pastPos.push(c);
    flightPath.setPath(pastPos);
    console.log("Coords received", c);

  });

  socket.on('Wind', (wind) => {
    document.getElementById('headwind').style.transform = " translate(-50%, -52%) rotate(" + wind + "deg)";
  })

  socket.on('HDG', (hdg) => {
    document.getElementById('head').style.transform = "rotate(" + hdg + "deg)";
  })

  socket.on('Tilt', (tilt) => {
    document.getElementById('tilt').style.transform = "rotate(" + tilt + "deg)";
    if (Math.abs(tilt) > 35) {
      document.getElementById('tilt').style.filter = "hue-rotate(180deg) sepia(100%) saturate(1000%)";
    } else {
      document.getElementById('tilt').style.filter = ""
    }
  })

  socket.on('Radio', (r) => {
    if (r == 0) {
      document.getElementById('rf').innerText = "RF: In use";
      document.getElementById('rf').style = "color: lime";
      document.getElementById('lora').innerText = "LoRa: No signal";
      document.getElementById('lora').style = "color: tomato";
    }
  })

  socket.on('Nav', (n) => {
    let text = "Nav Mode: ";
    switch (n) {
      case 0:
        text += "Manual";
        break;
      case 1:
        text += "Auto";
        break;
      case 2:
        text += "Home";
        break;
      case 4:
        text += "Loiter";
        break;
      default:
        text += "N/A"
    }
    document.getElementById("nav").innerText = text;
  })

  socket.on('LoRa', (rssi) => {
    let text = "LoRa: ";
    let style = "color:";
    if (rssi >= -70) {
      text += "Good";
      style += "lime";
    } else if (rssi >= -90) {
      style += "yellow;"
      text += "Average";
    } else {
      text += "Weak";
      style += "tomato";
    }
    text += " (" + rssi + "dBm)"
    document.getElementById('lora').innerText = text;
    document.getElementById('lora').style = style;

    document.getElementById('rf').innerText = "RF: No signal";
    document.getElementById('rf').style = "color: tomato";
  });

  socket.on('ip', (ip) => {
    let text = "IP: ";
    let style = "color:";
    if (ip == '0.0.0.0') {
      text += "No IP";
      style += 'yellow';
    } else {
      text += ip;
      style += 'lime';
    }
    document.getElementById('ip').innerText = text;
    document.getElementById('ip').style = style;
  });

  google.maps.event.addListener(map, "click", (event) => {
    addMarker(event.latLng, map);
  });
  //rts send loi auto

  document.getElementById("rts").onclick = () => {
    socket.emit("newNav", 2);
  }
  document.getElementById("loi").onclick = () => {
    socket.emit("newNav", 4);
  }
  document.getElementById("auto").onclick = () => {
    socket.emit("newNav", 1);
  }
  document.getElementById("send").onclick = () => {
    socket.emit("newMission", markerArray.map((e) => { return { lat: e.position.lat(), lng: e.position.lng() }; }));
  }

}

let labelIndex = 1;

// Adds a marker to the map.
function addMarker(location, map) {
  // Add the marker at the clicked location, and add the next-available label
  // from the array of alphabetical characters.
  const marker = new google.maps.Marker({
    position: location,
    label: "" + labelIndex,
    map: map,
  });
  const i = labelIndex;
  markerArray.push(marker);
  marker.addListener("click", (evt) => {
    marker.setMap(null);
    document.getElementById('waypoints').children[i - 1].style.display = "none";
  });
  let li = document.createElement("li");
  li.appendChild(document.createTextNode("Waypoint " + labelIndex++));
  document.getElementById('waypoints').appendChild(li);
}


