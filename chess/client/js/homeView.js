class HomeView {
    constructor(onCreate, onJoin, onSpectate) {
        this.onCreate = onCreate;
        this.onJoin = onJoin;
        this.onSpectate = onSpectate;
        this.div = document.getElementById("homeView");
        this.createButton = document.getElementById("homeViewCreate");
        this.joinButton = document.getElementById("homeViewJoin");
        this.joinId = document.getElementById("homeViewJoinId");
        this.spectateButton = document.getElementById("homeViewSpectate");
    }
    open() {
        this.joinId.value = "";
        this.onCreateClicked = (event) => {
            this.onCreate();
        }
        this.onJoinClicked = (event) => {
            const number = Number.parseInt(this.joinId.value);
            if (Number.isNaN(number)) return;
            this.onJoin(number);
        }
        this.onSpectateClicked = (event) => {
            const number = Number.parseInt(this.joinId.value);
            if (Number.isNaN(number)) return;
            this.onSpectate(number);
        }
        this.createButton.addEventListener("click", this.onCreateClicked);
        this.joinButton.addEventListener("click", this.onJoinClicked);
        this.spectateButton.addEventListener("click", this.onSpectateClicked);
        this.div.classList.remove("hiddenView");
    }
    close() {
        this.createButton.removeEventListener("click", this.onCreateClicked);
        this.joinButton.removeEventListener("click", this.onJoinClicked);
        this.spectateButton.removeEventListener("click", this.onSpectateClicked);
        this.div.classList.add("hiddenView");
    }
    update(dataView, offset) {}
}
