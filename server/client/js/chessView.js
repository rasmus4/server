class ChessView {
    constructor() {
        this.div = document.getElementById("chessView");
        this.canvas = document.getElementById("chessViewCanvas");
        this.context = this.canvas.getContext("2d");
        this.boardState = new Uint8Array(64); // left->right, top->bottom
        this.initBoardStateTest();
        this.setTileSize(80);
        this.draw();
    }
    // TODO remove
    initBoardStateTest() {
        this.boardState[0] = this.boardState[7] = ProtocolPieces.ROOK;
        this.boardState[1] = this.boardState[6] = ProtocolPieces.KNIGHT;
        this.boardState[2] = this.boardState[5] = ProtocolPieces.BISHOP;
        this.boardState[3] = ProtocolPieces.QUEEN;
        this.boardState[4] = ProtocolPieces.KING;
        for (let i = 0; i < 8; ++i) this.boardState[8 + i] = ProtocolPieces.PAWN;

        this.boardState[56] = this.boardState[63] = ProtocolPieces.ROOK | ProtocolPieces.WHITE_FLAG;
        this.boardState[57] = this.boardState[62] = ProtocolPieces.KNIGHT | ProtocolPieces.WHITE_FLAG;
        this.boardState[58] = this.boardState[61] = ProtocolPieces.BISHOP | ProtocolPieces.WHITE_FLAG;
        this.boardState[59] = ProtocolPieces.QUEEN | ProtocolPieces.WHITE_FLAG;
        this.boardState[60] = ProtocolPieces.KING | ProtocolPieces.WHITE_FLAG;
        for (let i = 0; i < 8; ++i) this.boardState[48 + i] = ProtocolPieces.PAWN | ProtocolPieces.WHITE_FLAG;
    }
    setTileSize(tileSize) {
        this.tileSize = tileSize;
        this.canvas.width = tileSize * 8;
        this.canvas.height = tileSize * 8;

        this.context.setTransform(1, 0, 0, 1, 0, 0);
        this.context.lineWidth = 2;
        this.context.lineCap = "round";
    }
    open() {
        this.div.classList.remove("hiddenView");
    }
    close() {
        this.div.classList.add("hiddenView");
    }
    update(dataView, offset) {
        
    }
    draw() {
        for (let x = 0; x < 8; ++x) {
            for (let y = 0; y < 8; ++y) {
                this.context.fillStyle = ((x + y) % 2 == 0) ? "#E3C497" : "#69441B";
                let baseX = x * this.tileSize;
                let baseY = y * this.tileSize;
                this.context.fillRect(baseX, baseY, this.tileSize, this.tileSize);
                
                let piece = this.boardState[y * 8 + x];
                if (piece !== 0) {
                    if (piece & ProtocolPieces.WHITE_FLAG) {
                        this.context.fillStyle = "#FFFFFF";
                        this.context.strokeStyle = "#000000";
                    }
                    else {
                        this.context.fillStyle = "#000000";
                        this.context.strokeStyle = "#000000";
                    }

                    switch (piece & ProtocolPieces.PIECE_MASK) {
                        case ProtocolPieces.PAWN: {
                            const verticalPad = this.tileSize / 5;
                            const horizontalPad = this.tileSize / 4;
                            this.context.beginPath();
                            this.context.moveTo(baseX + horizontalPad, baseY + this.tileSize - verticalPad);
                            this.context.lineTo(baseX + this.tileSize - horizontalPad, baseY + this.tileSize - verticalPad);
                            this.context.lineTo(baseX + this.tileSize / 2, baseY + verticalPad);
                            this.context.lineTo(baseX + horizontalPad, baseY + this.tileSize - verticalPad);
                            this.context.fill();
                            this.context.stroke();
                            break;
                        }
                        case ProtocolPieces.BISHOP: {
                            const horizontalLowerPad = this.tileSize / 3.5;
                            const horizontalUpperPad = this.tileSize / 2.2;
                            const verticalPad = this.tileSize / 8;

                            this.context.beginPath();
                            this.context.moveTo(baseX + horizontalLowerPad, baseY + this.tileSize - verticalPad);
                            this.context.lineTo(baseX + this.tileSize - horizontalLowerPad, baseY + this.tileSize - verticalPad);
                            this.context.lineTo(baseX + this.tileSize - horizontalUpperPad, baseY + verticalPad);
                            this.context.lineTo(baseX + horizontalUpperPad, baseY + verticalPad);
                            this.context.lineTo(baseX + horizontalLowerPad, baseY + this.tileSize - verticalPad);
                            this.context.fill();
                            this.context.stroke();
                            break
                        }
                        case ProtocolPieces.KNIGHT: {
                            const horizontalLowerPad = this.tileSize / 4;
                            const horizontalUpperPad = this.tileSize / 2.5;
                            const horizontalHeadPad = this.tileSize / 5;
                            const verticalPad = this.tileSize / 8;
                            const verticalBottomHeadPad = this.tileSize / 2.5;
                            const verticalMiddleHeadPad = this.tileSize / 3;

                            this.context.beginPath();
                            this.context.moveTo(baseX + horizontalLowerPad, baseY + this.tileSize - verticalPad);
                            this.context.lineTo(baseX + this.tileSize - horizontalLowerPad, baseY + this.tileSize - verticalPad);
                            this.context.lineTo(baseX + this.tileSize - horizontalUpperPad, baseY + verticalPad);
                            this.context.lineTo(baseX + horizontalUpperPad, baseY + verticalPad);
                            this.context.lineTo(baseX + horizontalHeadPad, baseY + verticalMiddleHeadPad);
                            this.context.lineTo(baseX + horizontalUpperPad, baseY + verticalBottomHeadPad);
                            this.context.lineTo(baseX + horizontalLowerPad, baseY + this.tileSize - verticalPad);
                            this.context.fill();
                            this.context.stroke();
                            break;
                        }
                        case ProtocolPieces.ROOK: {
                            const verticalPad = this.tileSize / 8;
                            const horizontalPad = this.tileSize / 4;
                            this.context.fillRect(baseX + horizontalPad, baseY + verticalPad, this.tileSize - 2 * horizontalPad, this.tileSize - 2 * verticalPad);
                            this.context.strokeRect(baseX + horizontalPad, baseY + verticalPad, this.tileSize - 2 * horizontalPad, this.tileSize - 2 * verticalPad);
                            break;
                        }
                        case ProtocolPieces.QUEEN: {
                            const horizontalBasePad = this.tileSize / 4;
                            const horizontalFirstTopPad = this.tileSize / 4.8;
                            const horizontalFirstDipPad = this.tileSize / 3;
                            const horizontalSecondTopPad = this.tileSize / 2.8;
                            const horizontalSecondDipPad = this.tileSize / 2.3;
                            
                            const verticalMiddlePad = this.tileSize / 5;
                            const verticalSecondPad = this.tileSize / 4;
                            const verticalFirstPad = this.tileSize / 3;
                            const verticalDipPad = this.tileSize * 3 / 5;

                            this.context.beginPath();
                            this.context.moveTo(baseX + horizontalBasePad, baseY + this.tileSize - verticalMiddlePad);
                            this.context.lineTo(baseX + this.tileSize - horizontalBasePad, baseY + this.tileSize - verticalMiddlePad);
                            this.context.lineTo(baseX + this.tileSize - horizontalBasePad, baseY + verticalDipPad);

                            this.context.lineTo(baseX + this.tileSize - horizontalFirstTopPad, baseY + verticalFirstPad);
                            this.context.lineTo(baseX + this.tileSize - horizontalFirstDipPad, baseY + verticalDipPad);

                            this.context.lineTo(baseX + this.tileSize - horizontalSecondTopPad, baseY + verticalSecondPad);
                            this.context.lineTo(baseX + this.tileSize - horizontalSecondDipPad, baseY + verticalDipPad);

                            this.context.lineTo(baseX + this.tileSize / 2, baseY + verticalMiddlePad);

                            this.context.lineTo(baseX + horizontalSecondDipPad, baseY + verticalDipPad);
                            this.context.lineTo(baseX + horizontalSecondTopPad, baseY + verticalSecondPad);

                            this.context.lineTo(baseX + horizontalFirstDipPad, baseY + verticalDipPad);
                            this.context.lineTo(baseX + horizontalFirstTopPad, baseY + verticalFirstPad);

                            this.context.lineTo(baseX + horizontalBasePad, baseY + verticalDipPad);
                            this.context.lineTo(baseX + horizontalBasePad, baseY + this.tileSize - verticalMiddlePad);
                            this.context.fill();
                            this.context.stroke();
                            break;
                        }
                        case ProtocolPieces.KING: {
                            const horizontalBasePad = this.tileSize / 4;
                            const horizontalUpperPad = this.tileSize / 5;
                            const horizontalDipPad = this.tileSize / 2.5;
                            const verticalPad = this.tileSize / 4;
                            const verticalFirstPad = this.tileSize / 3.5;

                            this.context.beginPath();
                            this.context.moveTo(baseX + horizontalBasePad, baseY + this.tileSize - verticalPad);
                            this.context.lineTo(baseX + this.tileSize - horizontalBasePad, baseY + this.tileSize - verticalPad);
                            this.context.lineTo(baseX + this.tileSize - horizontalBasePad, baseY + this.tileSize / 2);

                            this.context.lineTo(baseX + this.tileSize - horizontalUpperPad, baseY + verticalFirstPad);
                            this.context.lineTo(baseX + this.tileSize - horizontalDipPad, baseY + this.tileSize / 2);
                            this.context.lineTo(baseX + this.tileSize / 2, baseY + verticalPad);
                            this.context.lineTo(baseX + horizontalDipPad, baseY + this.tileSize / 2);
                            this.context.lineTo(baseX + horizontalUpperPad, baseY + verticalFirstPad);

                            this.context.lineTo(baseX +  horizontalBasePad, baseY + this.tileSize / 2);
                            this.context.lineTo(baseX + horizontalBasePad, baseY + this.tileSize - verticalPad);
                            this.context.fill();
                            this.context.stroke();
                            break;
                        }
                    }
                }
            }
        }
    }
}