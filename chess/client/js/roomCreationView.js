class RoomCreationView {
    constructor(onCreate) {
        this.onCreate = onCreate;
        this.div = document.getElementById("roomCreationView");
        this.createButton = document.getElementById("roomCreationViewCreate");
        this.friendlyFire = document.getElementById("roomCreationViewFriendlyFire");
    }
    open() {
        this.onCreateClicked = (event) => {
            let room = new HchessRoom();
            room.friendlyFire = this.friendlyFire.checked;
            this.onCreate(room);
        }
        this.createButton.addEventListener("click", this.onCreateClicked);
        this.div.classList.remove("hiddenView");
    }
    close() {
        this.createButton.removeEventListener("click", this.onCreateClicked);
        this.div.classList.add("hiddenView");
    }
    update(dataView, offset) {}
}