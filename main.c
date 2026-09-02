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

void salvar_pgm(int **matriz, int altura, int largura, int interacao, char *nome_arquivo){
    FILE *arquivo = fopen(nome_arquivo, "w");
    
    if (arquivo == NULL){
        printf("Erro ao abrir arquivo de saída\n");
        return;
    }
    
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



void *normalizar_pthread(void *arg){
    DadosThread *args = (DadosThread *)arg;

    for (int i = args->linha_inicio; i < args->linha_fim; i++){
        for (int j = 0; j < args->largura; j++){
            args->matriz[i][j] = (args->matriz[i][j] * 255) / args->interacao;
        }
    }

    free(args);
    return NULL;
}

int normalizar_matriz_pthread(int **matriz, int altura, int largura, int interacao, int threads){

    pthread_t tids[threads];
    DadosThread *dados[threads];

    int linhas_por_thread = altura / threads;

    for (int t = 0; t < threads; t++){

        dados[t] = (DadosThread *)malloc(sizeof(DadosThread));

        if (dados[t] == NULL){
            printf("Erro ao alocar dados da thread\n");
            return 1;
        }

        dados[t]->matriz = matriz;
        dados[t]->altura = altura;
        dados[t]->largura = largura;
        dados[t]->interacao = interacao;

        dados[t]->linha_inicio = t * linhas_por_thread;

        dados[t]->linha_fim =
            (t == threads - 1) ? altura : (t + 1) * linhas_por_thread;

        int ret = pthread_create(
            &tids[t],
            NULL,
            normalizar_pthread,
            (void *)dados[t]
        );

        if (ret != 0){
            printf("Erro ao criar thread %d\n", t);
            return 1;
        }
    }

    for (int t = 0; t < threads; t++){

        int ret = pthread_join(tids[t], NULL);

        if (ret != 0){
            printf("Erro ao aguardar thread %d\n", t);
            return 1;
        }
    }

    return 0;
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

void salvar_pgm_normalizado(int **matriz, int altura, int largura, char *nome_arquivo){

    FILE *arquivo = fopen(nome_arquivo, "w");

    if (arquivo == NULL){
        printf("Erro ao abrir arquivo de saída\n");
        return;
    }

    for (int i = 0; i < altura; i++){
        for (int j = 0; j < largura; j++){
            fprintf(arquivo, "%d ", matriz[i][j]);
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

            struct timespec inicio;
            struct timespec fim;



            clock_gettime(CLOCK_MONOTONIC, &inicio);

            int **matriz_serial = executar_mandelbrot_serial(altura, largura, interacao);

            if (matriz_serial == NULL){
                return 1;
            }

            clock_gettime(CLOCK_MONOTONIC, &fim);

            double tempo_serial = (fim.tv_sec - inicio.tv_sec) +
               (fim.tv_nsec - inicio.tv_nsec) / 1000000000.0;

            salvar_pgm(matriz_serial, altura, largura, interacao, "mandelbrot_lvsa_serial.pgm");



            clock_gettime(CLOCK_MONOTONIC, &inicio);

            int **matriz_openmp = executar_mandelbrot_openmp(altura, largura, interacao, threads);

            if (matriz_openmp == NULL){
                return 1;
            }

            clock_gettime(CLOCK_MONOTONIC, &fim);

            double tempo_openmp = (fim.tv_sec - inicio.tv_sec) + (fim.tv_nsec - inicio.tv_nsec) / 1000000000.0;


            salvar_pgm(matriz_openmp, altura, largura, interacao, "mandelbrot_lvsa_openmp.pgm");



            clock_gettime(CLOCK_MONOTONIC, &inicio);

            int **matriz_pthread1 = executar_mandelbrot_pthread(altura, largura, interacao, threads);

            if (matriz_pthread1 == NULL){
                return 1;
            }

            clock_gettime(CLOCK_MONOTONIC, &fim);

            double tempo_pthread1 = (fim.tv_sec - inicio.tv_sec) + (fim.tv_nsec - inicio.tv_nsec) / 1000000000.0;


            salvar_pgm(matriz_pthread1, altura, largura, interacao, "mandelbrot_lvsa_pthreads1.pgm");



            int **matriz_pthread2 = executar_mandelbrot_serial(altura, largura, interacao);

            if (matriz_pthread2 == NULL){
                return 1;
            }

            clock_gettime(CLOCK_MONOTONIC, &inicio);

            int erro_pthread2 = normalizar_matriz_pthread(
                matriz_pthread2,
                altura,
                largura,
                interacao,
                threads
            );

            if (erro_pthread2 != 0){
                return 1;
            }

            clock_gettime(CLOCK_MONOTONIC, &fim);

            double tempo_pthread2 = (fim.tv_sec - inicio.tv_sec) +
                (fim.tv_nsec - inicio.tv_nsec) / 1000000000.0;

            salvar_pgm_normalizado(
                matriz_pthread2,
                altura,
                largura,
                "mandelbrot_lvsa_pthreads2.pgm"
            );



            FILE *arquivo_tempo = fopen("times.txt", "w");

            if (arquivo_tempo == NULL){
                printf("Erro ao criar arquivo times.txt\n");
                return 1;
            }

            fprintf(arquivo_tempo, "Serial: %f segundos\n", tempo_serial);
            fprintf(arquivo_tempo, "OpenMP: %f segundos\n", tempo_openmp);
            fprintf(arquivo_tempo, "Pthreads 1: %f segundos\n", tempo_pthread1);
            fprintf(arquivo_tempo, "Pthreads 2: %f segundos\n", tempo_pthread2);

            fclose(arquivo_tempo);



            for (int k = 0; k < altura; k++){
                free(matriz_serial[k]);
                free(matriz_openmp[k]);
                free(matriz_pthread1[k]);
                free(matriz_pthread2[k]);
            }

            free(matriz_serial);
            free(matriz_openmp);
            free(matriz_pthread1);
            free(matriz_pthread2);
        }
    }
}