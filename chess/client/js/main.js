class Main {
    constructor() {
        this.currentView = null;
        this.socket = null;
        this.connected = false;
        this.buffer = new ArrayBuffer(1024);
        this.bufferByteView = new Uint8Array(this.buffer);
        this.bufferDataView = new DataView(this.buffer);

        this.onCreate = () => {
            this.bufferDataView.setUint8(0, ProtocolClientOp.CREATE);
            this.socket.send(this.bufferByteView.subarray(0, 1));
        };
        this.onJoin = (id) => {
            this.bufferDataView.setUint8(0, ProtocolClientOp.JOIN);
            this.bufferDataView.setInt32(1, id, true);
            this.socket.send(this.bufferByteView.subarray(0, 5));
        };
        this.onSpectate = (id) => {
            this.bufferDataView.setUint8(0, ProtocolClientOp.SPECTATE);
            this.bufferDataView.setInt32(1, id, true);
            this.socket.send(this.bufferByteView.subarray(0, 5));
        };
        this.homeView = new HomeView(this.onCreate, this.onJoin, this.onSpectate);
        this.onMove = (from, to) => {
            this.bufferDataView.setUint8(0, ProtocolClientOp.MOVE);
            this.bufferDataView.setUint8(1, from.x);
            this.bufferDataView.setUint8(2, from.y);
            this.bufferDataView.setUint8(3, to.x);
            this.bufferDataView.setUint8(4, to.y);
            this.socket.send(this.bufferByteView.subarray(0, 5));
        };
        this.onBack = () => {
            this.bufferDataView.setUint8(0, ProtocolClientOp.BACK);
            this.socket.send(this.bufferByteView.subarray(0, 1));
        };
        this.onScroll = (up) => {
            this.bufferDataView.setUint8(0, ProtocolClientOp.SCROLL);
            this.bufferDataView.setUint8(1, up ? 1 : 0);
            this.socket.send(this.bufferByteView.subarray(0, 2));
        };
        this.chessView = new ChessView(this.onMove, this.onBack, this.onScroll);
        this.connectingView = new ConnectingView();

        this.roomView = new RoomView(this.onBack);
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
                let receiveData = new DataView(event.data);
                if (!this.connected) {
                    let version = receiveData.getUint32(0, true);
                    if (version === protocolVersion) {
                        this.connected = true;
                        this.setView(null);
                    } else {
                        // Perhaps the game updated, reload page.
                        console.log("Reloading page...");
                        setTimeout(() => {
                            location.reload();
                        }, 1000);
                        this.clearSocketListeners();
                    }
                    return;
                }

                let op = receiveData.getUint8(0);
                switch (op) {
                    case ProtocolServerOp.HOME:
                        this.homeView.update(receiveData, 1);
                        this.setView(this.homeView);
                        break;
                    case ProtocolServerOp.ROOM:
                        this.roomView.update(receiveData, 1);
                        this.setView(this.roomView);
                        break;
                    case ProtocolServerOp.CHESS:
                        this.chessView.update(receiveData, 1);
                        this.setView(this.chessView);
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