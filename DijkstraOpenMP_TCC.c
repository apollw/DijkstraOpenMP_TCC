// DijkstraOpenMP_TCC.c : Este arquivo contém a função 'main'. A execução do programa começa e termina ali.
//

#define _CRT_SECURE_NO_WARNINGS 
#include <omp.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#define MAX_NOS 3500
#define NUM_VERTICES 256 /*512*/ /*1024*/ /*2048*/ /*4096*/ /*8192*/ /*16384*/
#define MIN_PESO 1
#define MAX_PESO 20
#define DIST_MAX INT_MAX

struct No {
    int vertice;
    int peso;
    struct No* prox;
};

struct Grafo {
    struct No* cabeca[NUM_VERTICES];
    int numVertices;
};

struct No* criarNo(int v, int p) {
    struct No* novoNo = (struct No*)malloc(sizeof(struct No));
    novoNo->vertice = v;
    novoNo->peso = p;
    novoNo->prox = NULL;
    return novoNo;
}

struct Grafo* criarGrafo(int vertices) {
    struct Grafo* grafo = (struct Grafo*)malloc(sizeof(struct Grafo));
    grafo->numVertices = vertices;

    for (int i = 0; i < vertices; i++) {
        grafo->cabeca[i] = NULL;
    }

    return grafo;
}

void adicionarAresta(struct Grafo* grafo, int orig, int dest, int peso) {
    struct No* novoNo = criarNo(dest, peso);
    novoNo->prox = grafo->cabeca[orig];
    grafo->cabeca[orig] = novoNo;
}

void imprimirGrafo(struct Grafo* grafo) {
    printf("\nGrafo:\n");
    for (int i = 0; i < grafo->numVertices; i++) {
        struct No* temp = grafo->cabeca[i];
        printf("Vertice %d: ", i);
        while (temp != NULL) {
            printf("(%d,%d) -> ", temp->vertice, temp->peso);
            temp = temp->prox;
        }
        printf("NULL\n");
    }
}

// Função para salvar o grafo em um arquivo
void salvarGrafo(struct Grafo* grafo, const char* nomeArquivo) {
    FILE* arquivo = fopen(nomeArquivo, "w");
    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo %s.\n", nomeArquivo);
        return;
    }

    // Escreve o número de vértices no arquivo
    fprintf(arquivo, "%d\n", grafo->numVertices);

    // Escreve as arestas do grafo no arquivo
    for (int i = 0; i < grafo->numVertices; i++) {
        struct No* temp = grafo->cabeca[i];
        while (temp != NULL) {
            fprintf(arquivo, "%d %d %d\n", i, temp->vertice, temp->peso);
            temp = temp->prox;
        }
    }

    fclose(arquivo);
    printf("Grafo salvo com sucesso no arquivo %s.\n", nomeArquivo);
}

// Função para carregar o grafo de um arquivo
struct Grafo* carregarGrafo(const char* nomeArquivo) {
    FILE* arquivo = fopen(nomeArquivo, "r");
    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo %s.\n", nomeArquivo);
        return NULL;
    }

    int numVertices;
    fscanf(arquivo, "%d", &numVertices);

    struct Grafo* grafo = criarGrafo(numVertices);

    int origem, destino, peso;
    while (fscanf(arquivo, "%d %d %d", &origem, &destino, &peso) == 3) {
        adicionarAresta(grafo, origem, destino, peso);
    }

    fclose(arquivo);
    printf("Grafo carregado com sucesso do arquivo %s.\n", nomeArquivo);
    return grafo;
}

void dijkstraOpenMPGeral(struct Grafo* grafo, int inicio) {
    int  distancias[NUM_VERTICES];
    bool visitados[NUM_VERTICES];

    int num_threads = 4;

    omp_set_num_threads(num_threads); //Setando internamente à função o número de threads máximo

    //Não há necessidade de paralelizar esse loop
    for (int i = 0; i < NUM_VERTICES; i++) {
        distancias[i] = INT_MAX;
        visitados[i] = false;
    }

    distancias[inicio] = 0;

    int count; //count deve ser inicializado fora do loop, em OpenMP
    #pragma omp parallel for num_threads(num_threads) //Definindo num_threads threads para uso no laço externo e geral
    for (count = 0; count < NUM_VERTICES - 1; count++) {
        int u = -1;
        for (int i = 0; i < NUM_VERTICES; i++) {

            /*int id = omp_get_thread_num();
            printf("Thread %d esta executando a iteracao %d do loop\n", id, i);*/

            if (!visitados[i] && (u == -1 || distancias[i] < distancias[u])) {
            #pragma omp critical //Aqui uma região crítica
                {
                    if (!visitados[i] && (u == -1 || distancias[i] < distancias[u])) {
                        u = i;
                    }
                }
            }
        }

        visitados[u] = true;

        struct No* v = grafo->cabeca[u];

        while (v != NULL) {
            if (!visitados[v->vertice] &&
                distancias[u] + v->peso < distancias[v->vertice]) {
                distancias[v->vertice] = distancias[u] + v->peso;
            }
            v = v->prox;
        }
    }

    /*printf("\nDistancias minimas a partir do vertice %d:\n", inicio);
    for (int i = 0; i < NUM_VERTICES; i++) {
        printf("Vertice %d: %d\n", i, distancias[i]);
    }*/
}

int main(int argc, char* argv[]) {

    omp_set_num_threads(8); //Setando o número máximo de threads para 8

    struct Grafo* grafo = criarGrafo(NUM_VERTICES);
    int peso = 0;
    int numArestas = 0;
    int vertice_de_entrada = 0;

    const char* grafo256 = "D:\\Grafos\\grafo256.txt";
    /*const char* grafo512 = "D:\\Grafos\\grafo512.txt";
    const char* grafo1024 = "D:\\Grafos\\grafo1024.txt";
    const char* grafo2048 = "D:\\Grafos\\grafo2048.txt";
    const char* grafo4096 = "D:\\Grafos\\grafo4096.txt";
    const char* grafo8192 = "D:\\Grafos\\grafo8192.txt";
    const char* grafo16384 = "D:\\Grafos\\grafo16384.txt";*/

    /*for (int i = 0; i < NUM_VERTICES; i++) {
        for (int j = i + 1; j < NUM_VERTICES; j++) {
            peso++;
            adicionarAresta(grafo, i, j, peso);
            adicionarAresta(grafo, j, i, peso);

            numArestas++;
            if (peso > 20)
                peso = 0;
        }
    }    */

    //Cálculo do Tamanho do Grafo
    for (int i = 0; i < NUM_VERTICES; i++) {
        for (int j = i + 1; j < NUM_VERTICES; j++) {
            numArestas++;
        }
    }

    printf("Numero de Vertices = %d\n", NUM_VERTICES);
    printf("Numero de Arestas = %d\n", numArestas);

    grafo = carregarGrafo(grafo256);

    //imprimirGrafo(grafo);

    /*------------------------------Paralelização dijkstraOpenMPGeral------------------------------------*/

    int n;
    int num_threads = 4;
    printf("\nUtilizando OpenMP - Implementacao Interna Geral - %d threads\n", num_threads);
    //Utilizando a diretiva de OpenMP internamente
    for (n = 0; n < 30; n++) {
        //int id = omp_get_thread_num();
        //printf("Thread %d esta executando a iteracao %d do loop\n", id, n);
        double start = omp_get_wtime();
        dijkstraOpenMPGeral(grafo, n);
        double end = omp_get_wtime();
        printf("Tempo de Execucao %d = %3.5f ms\n", n + 1, (end - start) * 1000);
    }

    free(grafo);

    return 0;
}