class ChessView {
    constructor(onMove, onBack) {
        this.onMove = onMove;
        this.onBack = onBack;

        this.div = document.getElementById("chessView");
        this.canvas = document.getElementById("chessViewCanvas");
        this.statusSpan = document.getElementById("chessViewStatus");
        this.backButton = document.getElementById("chessViewBack");
        this.timeSpentSpan = document.getElementById("chessViewTimeSpent");
        this.opponentTimeSpentSpan = document.getElementById("chessViewOpponentTimeSpent");
        this.context = this.canvas.getContext("2d");
        this.board = new Uint8Array(64); // left->right, bottom->top
        this.selectedTile = null;
        this.setTileSize(80);
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
        this.selectedTile = null;
        this.onMousedown = (event) => {
            if (event.button === 0) {
                let tile = {
                    x: Math.trunc(event.offsetX / this.tileSize),
                    y: 7 - Math.trunc(event.offsetY / this.tileSize)
                };
                if (this.selectedTile === null) {
                    this.selectedTile = tile;
                } else {
                    this.onMove(this.selectedTile, tile);
                    this.selectedTile = null;
                }
                this.draw();
            }
        }
        this.canvas.addEventListener("mousedown", this.onMousedown);

        this.onBackClicked = (event) => {
            this.onBack();
        }
        this.backButton.addEventListener("click", this.onBackClicked);

        this.div.classList.remove("hiddenView");
    }
    close() {
        this.canvas.removeEventListener("mousedown", this.onMousedown);
        this.backButton.removeEventListener("click", this.onBackClicked);
        
        this.div.classList.add("hiddenView");
    }
    update(dataView, offset) {
        this.whitesTurn = Boolean(dataView.getUint8(offset++));
        this.winner = dataView.getUint8(offset++);
        this.lastMoveFromIndex = dataView.getUint8(offset++);
        this.lastMoveToIndex = dataView.getUint8(offset++);
        this.timeSpent = Number(dataView.getBigInt64(offset, true)) / 1000000000;
        offset += 8;
        this.opponentTimeSpent = Number(dataView.getBigInt64(offset, true)) / 1000000000;
        offset += 8;
        for (let i = 0; i < 64; ++i) {
            this.board[i] = dataView.getUint8(offset + i);
        }
        this.draw();
    }
    getBoardIndex(x, y) {
        return y * 8 + x;
    }
    draw() {
        for (let x = 0; x < 8; ++x) {
            for (let y = 0; y < 8; ++y) {
                this.context.fillStyle = ((x + y) % 2 == 1) ? "#E3C497" : "#69441B";
                let baseX = x * this.tileSize;
                let baseY = (7 - y) * this.tileSize;
                this.context.fillRect(baseX, baseY, this.tileSize, this.tileSize);

                let index = this.getBoardIndex(x, y);
                let isSelectedTile = (this.selectedTile !== null && x === this.selectedTile.x && y === this.selectedTile.y);
                if (isSelectedTile || (index == this.lastMoveFromIndex || index == this.lastMoveToIndex)) {
                    this.context.globalAlpha = isSelectedTile ? 0.2 : 0.1;
                    this.context.fillStyle = "#00FF00";
                    this.context.fillRect(baseX, baseY, this.tileSize, this.tileSize);
                    this.context.globalAlpha = 1.0;
                }
                
                let piece = this.board[index];
                if (piece !== 0) {
                    if (piece & ProtocolPieces.WHITE_FLAG) {
                        this.context.fillStyle = "#FFFFFF";
                    } else {
                        this.context.fillStyle = "#000000";
                    }
                    this.context.strokeStyle = "#000000";

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
        if (this.winner != ProtocolWinner.NO_WIN) {
            if (this.winner == ProtocolWinner.WHITE_WIN) this.statusSpan.textContent = "White wins!";
            else this.statusSpan.textContent = "Black wins!";
        } else {
            if (this.whitesTurn) this.statusSpan.textContent = "Whites turn";
            else this.statusSpan.textContent = "Blacks turn";
        }
        this.timeSpentSpan.textContent = "Time spent: " + Number(Math.round(this.timeSpent + "e2") + "e-2") + "s";
        this.opponentTimeSpentSpan.textContent = "Time spent: " + Number(Math.round(this.opponentTimeSpent + "e2") + "e-2") + "s";
    }
}