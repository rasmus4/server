"use strict";
class Main {
    constructor() {
        this.homeView = new HomeView();
        this.chessView = new ChessView();
        this.connectingView = new ConnectingView();
        this.roomView = new RoomView();

        this.currentView = null;
        this.socket = null;
        this.connected = false;
        this.buffer = new ArrayBuffer(1024);
        this.bufferByteView = new Uint8Array(this.buffer);
        this.bufferDataView = new DataView(this.buffer);
        this.bufferDataView.setUint32(0, 65, true);
    }
    setView(newView) {
        if (newView === this.currentView) return;

        if (this.currentView) this.currentView.close();
        this.currentView = newView;
        if (this.currentView) this.currentView.open();
    }

    clearSocketListeners() {
        this.socket.removeEventListener("open", this.onSocketOpen);
        this.socket.removeEventListener("message", this.onSocketMessage);
        this.socket.removeEventListener("close", this.onSocketClose);
    }

    connect() {
        this.setView(this.connectingView);

        this.onSocketOpen = (event) => {
            this.socket.send(this.bufferByteView.subarray(0, 4));
        };
        this.onSocketMessage = (event) => {
            try {
                let receiveData = new DataView(event.data);
                if (!this.connected) {
                    let version = receiveData.getUint32(0, true);
                    if (version === protocolVersion) {
                        this.connected = true;
                        this.setView(null);
                    } else {
                        // Perhaps the game updated, reload page.
                        setTimeout(() => {
                            location.reload();
                        }, 1000);
                        this.clearSocketListeners();
                    }
                    return;
                }

                let view = receiveData.getUint8(0, true);
                switch (view) {
                    case ProtocolView.HOME:
                        this.setView(this.homeView);
                        this.homeView.update(receiveData, 4);
                        break;
                    case ProtocolView.ROOM:
                        this.setView(this.roomView);
                        this.roomView.update(receiveData, 4);
                        break;
                    case ProtocolView.CHESS:
                        this.setView(this.chessView);
                        this.chessView.update(receiveData, 4);
                        break;
                    default: {
                        console.error("Invalid view:", view);
                        this.setView(null);
                    }
                }
            } catch (error) {
                console.error("Error handling server message:", error);

            }
        };
        this.onSocketClose = (event) => {
            this.connected = false;
            this.clearSocketListeners();
            setTimeout(() => {
                this.connect();
            }, 1000);
        };
        this.socket = new WebSocket("ws://" + location.host + "/chess");
        this.socket.binaryType = "arraybuffer";
        this.socket.addEventListener("open", this.onSocketOpen);
        this.socket.addEventListener("close", this.onSocketClose);
        this.socket.addEventListener("message", this.onSocketMessage);
    }
}

window.onload = function() {
    let main = new Main();
    main.connect();
};