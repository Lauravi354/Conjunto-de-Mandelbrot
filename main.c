#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <pthread.h>

typedef struct DadosThread{
    int **matriz;
    int altura;
    int largura;
    int interacao;
    int linha_inicio;
    int linha_fim;

    int id;
    int total_threads;
} DadosThread;

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

int** executar_mandelbrot_serial(int altura, int largura, int interacao){
    int **matriz = (int**)malloc(altura * sizeof(int*));
    
    if (matriz == NULL){
        printf("Erro ao alocar memória para matriz\n");
        return NULL;
    }
    
    for (int k = 0; k < altura; k++){
        matriz[k] = (int*)malloc(largura * sizeof(int));
        
        if (matriz[k] == NULL){
            printf("Erro ao alocar memória para linha %d\n", k);
            for (int i = 0; i < k; i++){
                free(matriz[i]);
            }
            free(matriz);
            return NULL;
        }
    }
    
    for (int i = 0; i < altura; i++){
        for(int j = 0; j < largura; j++){
            double cr = -2.0 + j * (3.0 / largura);
            double ci = -1.5 + i * (3.0 / altura);

            matriz[i][j] = calcular_pixel(cr, ci, interacao);
        }
    }
    
    return matriz;
}

int** executar_mandelbrot_openmp(int altura, int largura, int interacao, int threads){
    int **matriz = (int**)malloc(altura * sizeof(int*));
    
    if (matriz == NULL){
        printf("Erro ao alocar memória para matriz\n");
        return NULL;
    }
    
    for (int k = 0; k < altura; k++){
        matriz[k] = (int*)malloc(largura * sizeof(int));
        
        if (matriz[k] == NULL){
            printf("Erro ao alocar memória para linha %d\n", k);
            for (int i = 0; i < k; i++){
                free(matriz[i]);
            }
            free(matriz);
            return NULL;
        }
    }
    
    #pragma omp parallel for num_threads(threads)
    for (int i = 0; i < altura; i++){
        for(int j = 0; j < largura; j++){
            double cr = -2.0 + j * (3.0 / largura);
            double ci = -1.5 + i * (3.0 / altura);

            matriz[i][j] = calcular_pixel(cr, ci, interacao);
        }
    }
    
    return matriz;
}

void *calcular_pthread(void *arg){
    DadosThread *args = (DadosThread *)arg;
    
    for (int i = args->linha_inicio; i < args->linha_fim; i++){
        for(int j = 0; j < args->largura; j++){
            double cr = -2.0 + j * (3.0 / args->largura);
            double ci = -1.5 + i * (3.0 / args->altura);

            args->matriz[i][j] = calcular_pixel(cr, ci, args->interacao);
        }
    }
    free(args);
    return NULL;
}

void *calcular_pthread_2(void *arg){
    DadosThread *args = (DadosThread *)arg;
    
    for (int i = args->id; i < args->altura; i += args->total_threads){
        for(int j = 0; j < args->largura; j++){
            double cr = -2.0 + j * (3.0 / args->largura);
            double ci = -1.5 + i * (3.0 / args->altura);

            args->matriz[i][j] = calcular_pixel(cr, ci, args->interacao);
        }
    }
    
    free(args);
    return NULL;
}

int** executar_mandelbrot_pthread(int altura, int largura, int interacao, int threads){
    int **matriz = (int**)malloc(altura * sizeof(int*));
    
    if (matriz == NULL){
        printf("Erro ao alocar memória para matriz\n");
        return NULL;
    }
    
    for (int k = 0; k < altura; k++){
        matriz[k] = (int*)malloc(largura * sizeof(int));
        
        if (matriz[k] == NULL){
            printf("Erro ao alocar memória para linha %d\n", k);
            for (int i = 0; i < k; i++){
                free(matriz[i]);
            }
            free(matriz);
            return NULL;
        }
    }
    
    pthread_t tids[threads];
    DadosThread *dados[threads];
    
    int linhas_por_thread = altura / threads;
    
    for (int t = 0; t < threads; t++){
        dados[t] = (DadosThread *)malloc(sizeof(DadosThread));
        
        dados[t]->matriz = matriz;
        dados[t]->altura = altura;
        dados[t]->largura = largura;
        dados[t]->interacao = interacao;
        dados[t]->linha_inicio = t * linhas_por_thread;
        dados[t]->linha_fim = (t == threads - 1) ? altura : (t + 1) * linhas_por_thread;
        
        int ret = pthread_create(&tids[t], NULL, calcular_pthread, (void *)dados[t]);
        
        if (ret != 0){
            printf("Erro ao criar thread %d\n", t);
            return NULL;
        }
    }
    
    for (int t = 0; t < threads; t++){
        int ret = pthread_join(tids[t], NULL);
        
        if (ret != 0){
            printf("Erro ao aguardar thread %d\n", t);
        }
    }
    
    return matriz;
}

int** executar_mandelbrot_pthread_2(int altura, int largura, int interacao, int threads){
    int **matriz = (int**)malloc(altura * sizeof(int*));
    
    if (matriz == NULL){
        printf("Erro ao alocar memória para matriz\n");
        return NULL;
    }
    
    for (int k = 0; k < altura; k++){
        matriz[k] = (int*)malloc(largura * sizeof(int));
        
        if (matriz[k] == NULL){
            printf("Erro ao alocar memória para linha %d\n", k);
            for (int i = 0; i < k; i++){
                free(matriz[i]);
            }
            free(matriz);
            return NULL;
        }
    }
    
    pthread_t tids[threads];
    DadosThread *dados[threads];

    for (int t = 0; t < threads; t++){
        dados[t] = (DadosThread *)malloc(sizeof(DadosThread));

        dados[t]->matriz = matriz;
        dados[t]->altura = altura;
        dados[t]->largura = largura;
        dados[t]->interacao = interacao;
        dados[t]->id = t;
        dados[t]->total_threads = threads;

        int ret = pthread_create(
            &tids[t],
            NULL,
            calcular_pthread_2,
            (void *)dados[t]
        );

        if (ret != 0){
            printf("Erro ao criar thread %d\n", t);
            return NULL;
        }
    }

        for (int t = 0; t < threads; t++){
        int ret = pthread_join(tids[t], NULL);
        
        if (ret != 0){
            printf("Erro ao aguardar thread %d\n", t);
        }
    }
    
    return matriz;
    
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

            struct timespec inicio;
            struct timespec fim;
            clock_gettime(CLOCK_MONOTONIC, &inicio);

            int **matriz = executar_mandelbrot_pthread_2(altura, largura, interacao, threads);
            if (matriz == NULL){
                return 1;
            }

            clock_gettime(CLOCK_MONOTONIC, &fim);

            double tempo = (fim.tv_sec - inicio.tv_sec) +
               (fim.tv_nsec - inicio.tv_nsec) / 1000000000.0;

            printf("Tempo Pthread estrategia 2: %f segundos\n", tempo);
            
            salvar_pgm(matriz, altura, largura, interacao);
            printf("Imagem salva em mandelbrot.pgm\n");
            
            for (int k = 0; k < altura; k++){
                free(matriz[k]);
            }
            free(matriz);
        }
    }
}