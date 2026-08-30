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
                    double cr = -2.0 + j * (3.0 / largura);
                    double ci = -1.5 + i * (3.0 / altura);

                    double zr = 0;
                    double zi = 0;
                    int n;
                    for (n = 0; n < interacao; n++){
                        double zr_novo = zr * zr - zi * zi + cr;
                        double zi_novo = 2 * zr * zi + ci;
                        zr = zr_novo;
                        zi = zi_novo;
                        if (zr * zr + zi * zi > 4.0){
                            break;
                        }
                    }
                }
            }
        }
    }
}