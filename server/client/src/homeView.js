class HomeView {
    constructor() {
        this.div = document.getElementById("homeView");
    }
    open() {
        this.div.classList.remove("hiddenView");
    }
    close() {
        this.div.classList.add("hiddenView");
    }
}