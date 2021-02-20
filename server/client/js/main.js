"use strict";
class Main {
    constructor() {
        this.currentView = null;
        this.socket = null;
        this.connected = false;
        this.buffer = new ArrayBuffer(1024);
        this.bufferByteView = new Uint8Array(this.buffer);
        this.bufferDataView = new DataView(this.buffer);

        this.homeView = new HomeView();
        this.onMove = (from, to) => {
            this.bufferDataView.setUint8(0, ProtocolClientOp.MOVE);
            this.bufferDataView.setUint8(1, from.x);
            this.bufferDataView.setUint8(2, from.y);
            this.bufferDataView.setUint8(3, to.x);
            this.bufferDataView.setUint8(4, to.y);
            this.socket.send(this.bufferByteView.subarray(0, 5));
        }
        this.chessView = new ChessView(this.onMove);
        this.connectingView = new ConnectingView();
        this.roomView = new RoomView();
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
            console.log("socket open!");
        };
        this.onSocketMessage = (event) => {
            try {
                console.log(event.data);
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

                let op = receiveData.getUint8(0, true);
                switch (op) {
                    case ProtocolServerOp.HOME:
                        this.setView(this.homeView);
                        this.homeView.update(receiveData, 1);
                        break;
                    case ProtocolServerOp.ROOM:
                        this.setView(this.roomView);
                        this.roomView.update(receiveData, 1);
                        break;
                    case ProtocolServerOp.CHESS:
                        this.setView(this.chessView);
                        this.chessView.update(receiveData, 1);
                        break;
                    default: throw "Invalid view!";
                }
            } catch (error) {
                console.error("Error handling server message:", error);
                // TODO what do on errors?
            }
        };
        this.onSocketClose = (event) => {
            this.connected = false;
            this.clearSocketListeners();
            setTimeout(() => {
                this.connect();
            }, 5000);
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