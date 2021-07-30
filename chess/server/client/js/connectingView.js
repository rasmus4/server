class ConnectingView {
    constructor() {
        this.div = document.getElementById("connectingView");
    }
    open() {
        this.div.classList.remove("hiddenView");
    }
    close() {
        this.div.classList.add("hiddenView");
    }
}