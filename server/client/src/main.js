const socket = new WebSocket("ws://" + location.host + "/chess");
socket.addEventListener("open", function (event) {
    socket.send("Hello world!");
});

socket.addEventListener("message", function (event) {
    document.getElementById("test").innerHTML = event.data;
});