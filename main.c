#include "blackjack.h"

int main(int argc, char **argv)
{

  struct ring_socket socket_recvfrom, socket_sendto;

  ring_socket_init(&socket_recvfrom, PORT_READ, argv[2], BIND);
  ring_socket_init(&socket_sendto, PORT_WRITE, argv[3], CONNECT);

  int id = atoi(argv[1]);

#ifdef DEBUG
  printf("%d %d\n", socket_recvfrom.fd, socket_sendto.fd);
  printf("%s %s %s\n", argv[1], argv[2], argv[3]);
#endif

  struct ring_pkg pkg;

  /* --- Player --- */

  if (id)
  {
    /* --- Sync --- */

    for (;;)
    {
      recv(socket_recvfrom.fd, &pkg, sizeof(struct ring_pkg), 0);

      switch (pkg.type)
      {
      case SYNC:
        ++pkg.addr;
      case READY:
        send(socket_sendto.fd, &pkg, sizeof(struct ring_pkg), 0);
      }

      if (pkg.type == READY)
        break;
    }

    puts("Connection established!"); /* Conexao estabelecida com sucesso */
    puts("Your game will start soon...");

    int n_players = pkg.addr;
    struct player *players = malloc(sizeof(struct player) * (n_players + 1));

    for (int8_t i = 0; i <= n_players; ++i)
      player_hand_clean(players + i);

    /* --- Game Loop --- */

    for (;;)
    {
      recv(socket_recvfrom.fd, &pkg, sizeof(struct ring_pkg), 0);

      switch (pkg.type)
      {
      case START:
        for (int8_t i = 0; i <= n_players; ++i)
        {
          player_hand_clean(players + i);
        }
        break;

      case BROADCAST:
        if (pkg.bet == -1)
          player_card_store(players + pkg.addr, pkg.card);
        else
          player_bet_store(players + pkg.addr, pkg.bet);

        table_print(players, n_players);
        break;

      case ASK:
        if (pkg.addr != id)
          break;

        if (pkg.bet == -1)
        {
          printf("[%c] Stand [%c] Hit [%c] Double Down [%c] Split", STAND, HIT, DOUBLE_DOWN, SPLIT);
          printf("\n\n> Enter your action: ");

          pkg.card.rank = (int8_t)getc(stdin);
        }
        else
        {
          float values[5] = {5, 10, 20, 50, 100};
          int choice;

          for (int i = 0; i < 5; i++)
            printf("[%d] $%.2f ", i + 1, values[i]);
          printf("\n\n> Make your bet: ");

          scanf("%d", &choice);
          pkg.bet = values[choice - 1];
        }

        while (getc(stdin) != '\n')
          ; /* Limpar o buffer */

        break;
      }

      send(socket_sendto.fd, &pkg, sizeof(struct ring_pkg), 0);
    }
  }

  /* --- Dealer --- */

  else
  {
    /* --- Sync --- */

    bzero(&pkg, sizeof(struct ring_pkg));
    pkg.type = SYNC;

    for (;;)
    {
      send(socket_sendto.fd, &pkg, sizeof(struct ring_pkg), 0);

      if (recv(socket_recvfrom.fd, &pkg, sizeof(struct ring_pkg), MSG_DONTWAIT) != -1)
      { /* recv pkg */
        pkg.type = READY;

        send(socket_sendto.fd, &pkg, sizeof(struct ring_pkg), 0);
        break;
      }
    }

    puts("Connection established!"); /* Conexao estabelecida com sucesso */

    for (;;)
    {
      recv(socket_recvfrom.fd, &pkg, sizeof(struct ring_pkg), 0);

      if (pkg.type == READY)
        break;

#ifdef DEBUG
      else
        printf("pkg of type: %c not %c %d\n", pkg.type, READY, SYNC);
#endif
    }

    // printf("Nº of Players: %d\n", pkg.addr);
    puts("> Press ENTER to start");

    while (getc(stdin) != '\n')
      ;

    /* --- Game Loop --- */

    struct deck deck;
    int n_players = pkg.addr;

    struct player *players = malloc(sizeof(struct player) * (n_players + 1));

    deck_init(&deck);

    for (;;)
    {
      deck_shuffle(&deck);

      pkg.type = START;

      send(socket_sendto.fd, &pkg, sizeof(struct ring_pkg), 0);
      recv(socket_recvfrom.fd, &pkg, sizeof(struct ring_pkg), 0);

      /* --- Aposta inicial --- */
      for (int8_t i = 0; i <= n_players; i++)
      {
        pkg.type = ASK;
        pkg.addr = i;
        pkg.bet = 100;

        send(socket_sendto.fd, &pkg, sizeof(struct ring_pkg), 0);
        recv(socket_recvfrom.fd, &pkg, sizeof(struct ring_pkg), 0);

        pkg.type = BROADCAST;
        pkg.addr = i;

        send(socket_sendto.fd, &pkg, sizeof(struct ring_pkg), 0);
        recv(socket_recvfrom.fd, &pkg, sizeof(struct ring_pkg), 0);
      }

      /* --- Envia a Primeira Carta --- */

      for (int8_t i = 0; i <= n_players; ++i)
      {
        player_hand_clean(players + i);

        pkg.type = BROADCAST;
        pkg.addr = i;
        pkg.bet = -1;

        pkg.card = deck_draw(&deck);
        player_card_store(players + i, pkg.card);

        printf("Sending 1st card to Player %d\n", (int)i);

        send(socket_sendto.fd, &pkg, sizeof(struct ring_pkg), 0);
        recv(socket_recvfrom.fd, &pkg, sizeof(struct ring_pkg), 0);
      }

      struct card secret = deck_draw(&deck); /* Compra a Carta Secreta do Dealer*/

      /* --- Envia a Segunda Carta dos Jogadores --- */

      for (int8_t i = 1; i <= n_players; ++i)
      {
        pkg.type = BROADCAST;
        pkg.addr = i;
        pkg.bet = -1;

        pkg.card = deck_draw(&deck);
        player_card_store(players + i, pkg.card);

        printf("Sending 2nd card to Player %d\n", (int)i);

        send(socket_sendto.fd, &pkg, sizeof(struct ring_pkg), 0);
        recv(socket_recvfrom.fd, &pkg, sizeof(struct ring_pkg), 0);
      }

      /* --- Ask For Insurance --- */

      /* --- Handle Players --- */

      for (int8_t i = 1; i <= n_players; ++i)
      {
        printf("Handling Player %d...\n", i);

        for (;;)
        {
          pkg.type = ASK;
          pkg.addr = i;
          pkg.bet = -1;

          send(socket_sendto.fd, &pkg, sizeof(struct ring_pkg), 0);
          recv(socket_recvfrom.fd, &pkg, sizeof(struct ring_pkg), 0);

          switch (pkg.card.rank)
          {
          case STAND:
            goto loop_exit;

          case HIT:
            pkg.type = BROADCAST;
            pkg.card = deck_draw(&deck);

            player_card_store(players + i, pkg.card);

            send(socket_sendto.fd, &pkg, sizeof(struct ring_pkg), 0);
            recv(socket_recvfrom.fd, &pkg, sizeof(struct ring_pkg), 0);

            if (player_hand_value(players + i) > 21)
              goto loop_exit;

            break;
          case DOUBLE_DOWN:
            pkg.type = BROADCAST;
            pkg.addr = i;
            pkg.bet = players[i].bet * 2;

            send(socket_sendto.fd, &pkg, sizeof(struct ring_pkg), 0);
            recv(socket_recvfrom.fd, &pkg, sizeof(struct ring_pkg), 0);

            pkg.type = BROADCAST;
            pkg.addr = i;
            pkg.bet = -1;

            pkg.card = deck_draw(&deck);
            player_card_store(players + i, pkg.card);

            send(socket_sendto.fd, &pkg, sizeof(struct ring_pkg), 0);
            recv(socket_recvfrom.fd, &pkg, sizeof(struct ring_pkg), 0);

            goto loop_exit;

            break;
          }
        }

      loop_exit:
      }

      /* --- Handle Dealer --- */

      /* --- Send the Secret Card --- */

      player_card_store(players, secret);

      pkg.type = BROADCAST;
      pkg.addr = 0;
      pkg.card = secret;

      send(socket_sendto.fd, &pkg, sizeof(struct ring_pkg), 0);
      recv(socket_recvfrom.fd, &pkg, sizeof(struct ring_pkg), 0);

      /* --- Calculate the hand of the players --- */

      pkg.type = BROADCAST;
      pkg.addr = 0;

      /* --- BlackJack H17 - Hits on a soft 17 --- */

      while (player_hand_value(players) < 17)
      {
        pkg.card = deck_draw(&deck);
        player_card_store(players, pkg.card);

        send(socket_sendto.fd, &pkg, sizeof(struct ring_pkg), 0);
        recv(socket_recvfrom.fd, &pkg, sizeof(struct ring_pkg), 0);
      }

      /* --- End Of Round --- */

      // printf("=============================\n");

      puts("\n*** ROUND FINISHED ***\n");
      printf("Dealer's hand: %d\n", player_hand_value(players));

      table_print_dealer(players, n_players);

      for (int8_t i = 1; i <= n_players; ++i)
      {
        int8_t hand_value = player_hand_value(players + i);

        printf("Player %d Score: %d ", i, hand_value);

        if (
            hand_value > 21 ||
            (hand_value <= player_hand_value(players) && player_hand_value(players) <= 21))

          printf("[LOST]\n");
        else
          printf("[WON]\n");
      }

      /* --- Handle New Round --- */

      puts("\n*** NEW ROUND ***\n");
      puts("> Press ENTER to start");

      while (getc(stdin) != '\n')
        ; /* Espera receber um '\n' */
    }

    free(players);
  }

  return 0;
}
