// Establish a WebSocket connection to the server and listen for incoming data.
const ws = new WebSocket('ws://localhost:8080');

ws.onopen = function() {
    console.log("Connected to WebSocket server.");
    ws.send(JSON.stringify({ message: "Start communication!" }));
};

ws.onerror = function(error) {
    console.error("WebSocket Error:", error);
};

// Function to update the battery level display
function updateBatteryLevel(percentage) {
    const batteryLevel = document.getElementById('batteryLevel');
    batteryLevel.style.height = percentage + '%';

    if (percentage > 75) {
        batteryLevel.style.backgroundColor = 'green';
    } else if (percentage > 50) {
        batteryLevel.style.backgroundColor = 'yellow';
    } else if (percentage > 25) {
        batteryLevel.style.backgroundColor = 'orange';
    } else {
        batteryLevel.style.backgroundColor = 'red';
    }

    batteryLevel.textContent = percentage + '%';
}

// Function to update the speed display
function updateSpeed(speed) {
    const maxSpeed = 3; // Max speed in m/s
    const rotation = (speed / maxSpeed) * 90; // Convert speed to rotation angle
    const needle = document.getElementById('needle');

    needle.style.transform = 'translateX(-50%) rotate(' + rotation + 'deg)';

    const speedDisplay = document.getElementById('speedDisplay');
    speedDisplay.textContent = speed.toFixed(1) + ' m/s';
}

// Handle incoming WebSocket messages
ws.onmessage = function(event) {
    console.log("Received data:", event.data);
    try {
        const data = JSON.parse(event.data);
        if (data.type === 'video') {
            document.getElementById('videoFrame').src = 'data:image/jpeg;base64,' + data.data;
        } else {
            updateBatteryLevel(data.battery_percentage.toFixed(0));
            document.getElementById('distance').textContent = data.distance_mm.toFixed(0);
            const speed = parseFloat(data.speed_y.toFixed(2));
            updateSpeed(speed);
            document.getElementById('speedY').textContent = data.speed_y.toFixed(1);
        }
    } catch (error) {
        console.error('Error parsing WebSocket message:', error);
    }
};