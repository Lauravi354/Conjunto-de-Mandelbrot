#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]){

    if (argc < 5 || argc > 5){
        printf("formato esperado: ./mandelbrot altura largura interações threads");
        return 1;
    }
    else{
        int altura = atoi(argv[1]);
        int largura = atoi(argv[2]);
        int interacao = atoi(argv[3]);
        int threads = atoi(argv[4]);
    
        if (altura < 1 || altura > 800 || largura < 1 || largura > 600 || interacao < 1 || interacao > 100 || threads < 1 || threads > 4){
            printf("você excedeu algum limite de valor");
        }
        else{
            int i;
            int j;
            for (i = 0; i < altura; i++){
                for(j = 0; j < largura; j++){
                    double cr = -2 + i * (3 / largura);
                    double ci = -1.5 + j * (3 / altura);

                    int c = cr + ci*i;
                }
            }
        }
    }
}