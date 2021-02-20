class HomeView {
    constructor(onCreate, onJoin) {
        this.onCreate = onCreate;
        this.onJoin = onJoin;
        this.div = document.getElementById("homeView");
        this.createButton = document.getElementById("homeViewCreate");
        this.joinButton = document.getElementById("homeViewJoin");
        this.joinId = document.getElementById("homeViewJoinId");
    }
    open() {
        this.onCreateClicked = (event) => {
            this.onCreate();
        }
        this.onJoinClicked = (event) => {
            const number = Number.parseInt(this.joinId.value);
            if (Number.isNaN(number)) return;
            this.onJoin(number);
        }
        this.createButton.addEventListener("click", this.onCreateClicked);
        this.joinButton.addEventListener("click", this.onJoinClicked);
        this.div.classList.remove("hiddenView");
    }
    close() {
        this.createButton.removeEventListener("click", this.onCreateClicked);
        this.joinButton.removeEventListener("click", this.onJoinClicked);
        this.div.classList.add("hiddenView");
    }
    update(dataView, offset) {}
}
