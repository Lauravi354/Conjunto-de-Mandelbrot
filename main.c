#include <stdio.h>
#include <stdlib.h>

int calcular_pixel(double cr, double ci, int interacao){
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
    return n;
}

void salvar_pgm(int **matriz, int altura, int largura, int interacao){
    FILE *arquivo = fopen("mandelbrot.pgm", "w");
    
    if (arquivo == NULL){
        printf("Erro ao abrir arquivo mandelbrot.pgm\n");
        return;
    }
    
    fprintf(arquivo, "P2\n%d %d\n255\n", largura, altura);
    
    for (int i = 0; i < altura; i++){
        for (int j = 0; j < largura; j++){
            int pixel = (matriz[i][j] * 255) / interacao;
            fprintf(arquivo, "%d ", pixel);
        }
        fprintf(arquivo, "\n");
    }
    fclose(arquivo);
}

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
            return 1;
        }
        else{
            int **matriz = (int**)malloc(altura * sizeof(int*));
            for (int k = 0; k < altura; k++){
                matriz[k] = (int*)malloc(largura * sizeof(int));
            }
            
            int i;
            int j;
            for (i = 0; i < altura; i++){
                for(j = 0; j < largura; j++){
                    double cr = -2.0 + j * (3.0 / largura);
                    double ci = -1.5 + i * (3.0 / altura);

                    matriz[i][j] = calcular_pixel(cr, ci, interacao);
                }
            }
            
            salvar_pgm(matriz, altura, largura, interacao);
            printf("Imagem salva em mandelbrot.pgm\n");
            
            for (int k = 0; k < altura; k++){
                free(matriz[k]);
            }
            free(matriz);
        }
    }
}