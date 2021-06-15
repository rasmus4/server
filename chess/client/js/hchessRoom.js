class HchessRoom extends Room {
    constructor() {
        super();
        this.roomType = ProtocolRoomType.HCHESS; 
        this.friendlyFire = false;
    }

    setBuffer(buffer) {
        buffer.setUint8(1, this.roomType);
        buffer.setUint8(2, this.friendlyFire);
    }

    bufferLength() {
        return 3;
    }
}