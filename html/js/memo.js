const characters = [
    "awesomo",
    "kyle",
    "trump",
    "kenny",
    "butters",
    "towelie",
    "scott",
    "lorde",
    "craig",
    "timmy"
];

const cards = characters.concat(characters); // Doubler les cartes pour créer des paires

const gameBoard = document.getElementById("game-board");

// Mélanger les cartes
cards.sort(() => 0.5 - Math.random());

// Créer les cartes
for (let i = 0; i < cards.length; i++) {
    const card = document.createElement("div");
    card.classList.add("card");
    card.setAttribute("data-character", cards[i]);

    const image = document.createElement("img");
    image.src = `img/characters/${cards[i]}.webp`;

    card.appendChild(image);
    gameBoard.appendChild(card);

    card.addEventListener("click", flipCard);
}

let flippedCards = [];
let matchedCards = [];

function flipCard() {
    if (flippedCards.length < 2 && !this.classList.contains("flipped") && !matchedCards.includes(this)) {
        this.classList.add("flipped");
        flippedCards.push(this);

        if (flippedCards.length === 2) {
            setTimeout(checkMatch, 1000);
        }
    }
}

function checkMatch() {
    const card1 = flippedCards[0];
    const card2 = flippedCards[1];

    if (card1.getAttribute("data-character") === card2.getAttribute("data-character")) {
        card1.removeEventListener("click", flipCard);
        card2.removeEventListener("click", flipCard);
        matchedCards.push(card1, card2);

        if (matchedCards.length === cards.length) {
            setTimeout(() => {
                alert("Félicitations ! Vous avez gagné le jeu !");
            }, 500);
        }
    } else {
        card1.classList.remove("flipped");
        card2.classList.remove("flipped");
    }

    flippedCards = [];
}
