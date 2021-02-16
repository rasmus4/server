class ChessView {
    constructor() {
        this.div = document.getElementById("chessView");
    }
    open() {
        this.div.classList.remove("hiddenView");
    }
    close() {
        this.div.classList.add("hiddenView");
    }
    update(dataView, offset) {
        
    }
}