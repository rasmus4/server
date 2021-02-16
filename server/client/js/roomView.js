class RoomView {
    constructor() {
        this.div = document.getElementById("roomView");
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