"use strict";

/*INCLUDE(src/homeView.js)*/
/*INCLUDE(src/chessView.js)*/
/*INCLUDE(src/connectingView.js)*/

class Main {
    constructor() {
        this.currentView = null;
        this.socket = null;
        this.buffer = new ArrayBuffer(1024);
        this.bufferByteView = new Uint8Array(this.buffer);
        this.bufferDataView = new DataView(this.buffer);
        this.bufferDataView.setUint32(0, 65, true);
    }
    switchView(newView) {
        if (this.currentView) this.currentView.close();
        this.currentView = newView;
        if (this.currentView) this.currentView.open();
    }

    connect() {
        if (this.socket) {
            this.socket.removeEventListener("open", this.onSocketOpen);
            this.socket.removeEventListener("message", this.onSocketMessage);
            this.socket.removeEventListener("close", this.onSocketClose);
            this.socket.removeEventListener("error", this.onSocketError);
        }
        this.switchView(Main.connectingView);

        this.onSocketOpen = (event) => {
            this.switchView(Main.homeView);
            this.socket.send(this.bufferByteView.subarray(0, 4));
        };
        this.onSocketMessage = (event) => {
            let receiveData = new DataView(event.data);
            document.getElementById("test").innerHTML = receiveData.getUint32(0, true);
        };
        this.onSocketClose = (event) => {
            this.connect();
        };
        this.onSocketError = (event) => {
            this.connect();
        };
        this.socket = new WebSocket("ws://" + location.host + "/chess");
        this.socket.binaryType = "arraybuffer";
        this.socket.addEventListener("open", this.onSocketOpen);
        this.socket.addEventListener("message", this.onSocketMessage);
        this.socket.addEventListener("close", this.onSocketClose);
        this.socket.addEventListener("error", this.onSocketError);
    }
}

window.onload = function() {
    Main.homeView = new HomeView();
    Main.chessView = new ChessView();
    Main.connectingView = new ConnectingView();
    let main = new Main();
    main.connect();
};