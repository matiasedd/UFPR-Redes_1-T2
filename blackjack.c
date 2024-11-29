#include "blackjack.h"

void card_print(struct card c) {
  printf("%d ", (int) c.rank); //printf("%2"PRId8"|%c\n", c.rank, (char) c.suit);
  switch (c.suit) {
	case 'H': printf("♥ "); break;
	case 'D': printf("♦ "); break;
	case 'S': printf("♠ "); break;
	case 'C': printf("♣ "); break;
  }
}

/* Nao eh preguica, eh loop unrolling
 * obs: talvez seja preguica :p 
 */

void deck_init(struct deck *d) {
  for (int i = 0; i < 13; ++i) { 
    d->cards[i*4].rank = i;
    d->cards[i*4].suit = 'C';

    d->cards[i*4+1].rank = i;
    d->cards[i*4+1].suit = 'D';

    d->cards[i*4+2].rank = i;
    d->cards[i*4+2].suit = 'H';

    d->cards[i*4+3].rank = i;
    d->cards[i*4+3].suit = 'S';

  }
}

void deck_shuffle(struct deck *d) {
  struct card aux;
  int gamble;

  d->head = 51;

  srand(52); /* dps eu mudo isso aqui */

  for (int i = 0; i < 52; ++i) {
    gamble = rand() % 52;

     aux = d->cards[i];
     d->cards[i] = d->cards[gamble];
     d->cards[gamble] = aux;
  }

}

struct card deck_draw(struct deck *d) {
  return d->cards[d->head--];
}

/* --- Player --- */

void player_init(struct player *p) { /* Zera a memoria de uma struct Player */
  bzero(p, sizeof (struct player));
}

void player_card_store(struct player *p, struct card c) { /* Adiciona uma carta na struct Player */
  p->hand[p->index++] = c;
}

void player_bet_store(struct player *p, float b) {
  p->bet = b;
}

void player_hand_print(struct player p) { /* Imprime a mao do jogador */
  for (int i = 0; i < p.index; ++i)
    card_print(p.hand[i]);
  printf("\n");
}

void player_hand_clean(struct player *p) { /* Zera a mao do jogador */
  bzero(p, sizeof (struct player));
}

int8_t player_hand_value(struct player *p) { /* Retorna o melhor valor das cartas na mao de um jogador */
  int i;
  int8_t aux;

  for (i = 0, aux = 0; i < (p->index); ++i)
    aux += p->hand[i].rank;

  return aux;
}

float player_bet(struct player *p) { /* Aposta do jogador */
	float values[5] = {5, 10, 20, 50, 100};

	for (int i = 0; i < 4; i++)
		printf("[%d] $%.2f ", i+1, values[i]);
	printf("\nMake your bet: ");
	
	int choice;
	scanf("%d", &choice);

	return values[choice-1];
}

void table_print(struct player *players, int8_t n_players) {
  //printf("\033[2J=============================\n");
  printf("\033[2J----------\n\n");

  for (int8_t i = 0; i <= n_players; ++i) {
    printf("PLAYER\t%d\n", i);
	printf("Amount:\t$%.2f\n", players[i].bet);	

	printf("Cards:\t");
    player_hand_print(*(players + i));
	printf("\n");
  }

  //printf("=============================\n\033[u");
  //printf("=============================\n");
  // printf("----------\n");
}

void table_print_dealer(struct player *players, int8_t n_players) {
  //printf("=============================\n");

  for (int8_t i = 0; i <= n_players; ++i) {
    printf("PLAYER\t%d\n", i);

	printf("Cards:\t");
    player_hand_print(*(players + i));
    printf("\n");
  }

  //printf("=============================\n");
}

/* --- Ring --- */

int ring_socket_init(struct ring_socket *r_socket, uint16_t port, char *ipv4_addr, int type) {
  r_socket->fd = socket(AF_INET, SOCK_DGRAM, 0);

  bzero(&r_socket->addr, sizeof (struct sockaddr_in));

  r_socket->addr.sin_family = AF_INET;                       /* IPV4 Family */
  r_socket->addr.sin_port = htons(port);                     /* RFC 1700 uses big-endian ; Port: 0 to 2^10 - 1 */
  inet_pton(AF_INET, ipv4_addr, &(r_socket->addr.sin_addr)); /* Converts char* to addr */

  #ifdef DEBUG
    printf("%s\n", ipv4_addr);
  #endif

  switch(type) {
    case BIND:
      return bind(r_socket->fd, (struct sockaddr*) &(r_socket->addr), sizeof (struct sockaddr_in));
    case CONNECT:
      return connect(r_socket->fd, (struct sockaddr*) &(r_socket->addr), sizeof (struct sockaddr_in));
    default:
      return -1;
  }

}
