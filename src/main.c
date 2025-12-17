#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <string.h>



int main()
{
    char tekst_usera[4096];

    while (1)
    {

        if (fgets(tekst_usera, sizeof(tekst_usera), stdin) == NULL)
        {
            break;
        }
        printf("Read: %s", tekst_usera);

        ssize_t len_usera = strlen(tekst_usera);
        if (len_usera >0 && tekst_usera[len_usera - 1] == '\n')
        {
            tekst_usera[len_usera - 1] = '\0';
        }

        //eee chhyba fgets nei wczyta wiecej znakow niz 4096 ale sprawdz pozniej
        if (len_usera > 4096)
        {
            printf("Buffer Overflow\n");
            break;
        }

        if (len == 0)
        {
            continue;
        }

        //wyjscie
        if (strcmp(tekst_usera, "quit") == 0 || strcmp(tekst_usera, "exit") == 0)
        {
            break;
        }

        printf("czy nam dziala i co dziala? : %s\n", tekst_usera);




   }


}
