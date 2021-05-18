class RoomView {
    constructor(onBack) {
        this.onBack = onBack;
        this.div = document.getElementById("roomView");
        this.gameId = document.getElementById("roomViewId");
        this.backButton = document.getElementById("roomViewBack");
    }
    open() {
        this.onBackClicked = (event) => {
            this.onBack();
        }
        this.backButton.addEventListener("click", this.onBackClicked);
        this.div.classList.remove("hiddenView");
    }
    close() {
        this.backButton.removeEventListener("click", this.onBackClicked);
        this.div.classList.add("hiddenView");
    }
    update(dataView, offset) {
        this.gameId.value = dataView.getInt32(offset, true).toString();
    }
}