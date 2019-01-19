//RAFAEL DE SOUSA NASCIMENTO 15200084
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h> 
#include <pthread.h>

/******************************************************************
 * ESTRUTURAS 
 ******************************************************************/
typedef struct roteamento {
    uint32_t endereco;
    char mascara; /* entre 1 e 31 bits, quantos bits da parte de rede */
    char enlace; /* enlace de saida */
} entrada;

typedef struct elemento {
    uint32_t endereco;
    struct elemento *prox;
} elemento;

typedef struct fila {
    struct elemento* inicio;
    struct elemento* fim;
} fila;

typedef struct teste {
    entrada * rotas;
    int num_rotas;
    uint32_t * pacotes;
    int num_pacotes;
    entrada * filtros;
    int num_filtros;
    int num_enlaces;
} teste;

/******************************************************************
 * ESCOPO DAS FUNCOES
 ******************************************************************/
fila* criar();
int vazio(fila*q);
elemento* alocar(uint32_t endereco);
void adicionar(fila *q, uint32_t * pacotes, int num_pacotes);
elemento* remover(fila *q);
void display(fila*q);
void* adicionarFila(void *arg);
void* processarFila(void *arg);
uint32_t pegarBitsMascara(char numBits);
uint32_t * roteamento(entrada * rotas, int num_rotas, uint32_t * pacotes, int num_pacotes, entrada * filtros, int num_filtros, int num_enlaces);

/******************************************************************
 * GLOBAIS
 ******************************************************************/
fila *queue;
uint32_t * saida;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;

/******************************************************************
 * Fila  
 ******************************************************************/
fila* criar() { //Cria a fila
    fila *q = (fila*) malloc(sizeof (fila));
    if (!q)
        exit(1);
    else {
        q->inicio = NULL;
        q->fim = NULL;
    }
    return q;
}

int vazio(fila*q) { //Verifica se esta vazio
    if (q == NULL)
        return 1;
    if (q->inicio == NULL)
        return 1;
    else
        return 0;
}

elemento* alocar(uint32_t endereco) { //Alocando elemento
    elemento* nodo = (elemento*) malloc(sizeof (elemento));
    if (!nodo)
        exit(1);
    else {
        nodo->endereco = endereco;
        nodo->prox = NULL;
        return nodo;
    }
}

void adicionar(fila *q, uint32_t * pacotes, int num_pacotes) { //Adicionando na fila
    elemento *nodo;
    for (int i = 0; i < num_pacotes; i++) {
        nodo = alocar(pacotes[i]);
        if (!nodo)
            exit(1);
        if (q->fim == NULL)
            q->inicio = nodo;
        else
            q->fim->prox = nodo;
        q->fim = nodo;
    }
}

elemento* remover(fila *q) { //Removendo da fila
    if (vazio(q)) return 0; //se vazio

    elemento* nodo = q->inicio;
    q->inicio = q->inicio->prox;
    if (q->inicio != NULL)
        q->fim = NULL;
    return nodo;
}

void display(fila*q) { //Mostrando na tela
    if (vazio(q)) {
        printf("Fila Vazia\n");
        return;
    }
    elemento*aux = q->inicio;
    printf("\nElementos da fila:\n");
    while (aux != NULL) {
        printf("Endereco: %i\n", aux->endereco);
        aux = aux->prox;
    }
    printf("\n");
}

/******************************************************************
 * Adicionando na fila usando thread
 ******************************************************************/
void* adicionarFila(void *arg) {
    teste * p = (teste *) arg;
    adicionar(queue, p->pacotes, p->num_pacotes);
    pthread_exit(NULL);
}

/******************************************************************
 * Removendo e processando da fila usando thread
 ******************************************************************/
void* processarFila(void *arg) {
    teste * p = (teste *) arg;
    uint32_t tmpMaskRota = 0;
    uint32_t tmpMaskPedido = 0;
    uint32_t guardaMascara = 0;
    int guardaIndice = -1;
    uint32_t rejeita;
    elemento *aux;

    if (!vazio(queue)) { //se a fila nao estiver vazia
        //---------------------------------------------------- Sessão crítica 1 -------------------------------------------------------/
        pthread_mutex_lock(&mutex);
        aux = remover(queue); //tira o elemento da fila e processa
        pthread_mutex_unlock(&mutex);

        rejeita = 0;
        guardaIndice = -1;
        //passa o pedido pelo filtro
        for (int k = 0; k < p->num_filtros; k++) {
            tmpMaskRota = pegarBitsMascara(p->filtros[k].mascara);
            tmpMaskRota = p->filtros[k].endereco & tmpMaskRota; //tenho a mascara do filtro aqui
            tmpMaskPedido = pegarBitsMascara(p->filtros[k].mascara);
            tmpMaskPedido = (aux->endereco) & tmpMaskPedido;

            if (tmpMaskPedido == tmpMaskRota) { //rejeita pacote
                rejeita = 1;
                break;
            }
        }
        //se o pedido nao foi rejeitado
        if (rejeita == 0) {
            for (int j = 0; j < p->num_rotas; j++) { //cada pacote analisa todas as rotas a ser encaminhada
                tmpMaskRota = pegarBitsMascara(p->rotas[j].mascara);
                tmpMaskRota = p->rotas[j].endereco & tmpMaskRota; //tenho a mascara da rota aqui
                tmpMaskPedido = pegarBitsMascara(p->rotas[j].mascara);
                tmpMaskPedido = (aux->endereco) & tmpMaskPedido; //tenho a mascara do pedido aqui
                if (tmpMaskPedido == tmpMaskRota) { //se tiver os mesmos prefixos (a mascara do pacote for igual a mascara da rota)
                    if (aux->endereco >= tmpMaskRota) { //se ip do pacote que esta entrando eh maior ou igual a mascara da rota
                        if (tmpMaskRota > guardaMascara) { //guarda a mascara atual e o indice se a condicao for verdade, ou seja, tem maior prefixo
                            guardaMascara = tmpMaskRota;
                            guardaIndice = j;
                        } else {
                            if ((tmpMaskRota == guardaMascara)&&(p->rotas[j].mascara > p->rotas[(int) guardaIndice].mascara)) { //se a mascara for igual, mas tem um prefixo de maior comprimento
                                guardaIndice = j;
                            }
                        }
                    }
                }
            }
        }
        //---------------------------------------------------- Sessão crítica 2 -------------------------------------------------------/
        pthread_mutex_lock(&mutex1);
        if (guardaIndice != -1) {
            saida[(int) p->rotas[guardaIndice].enlace]++; //incrementa saida
        } else {
            saida[0]++;
        }
        pthread_mutex_unlock(&mutex1);
        guardaMascara = 0;
    }
    pthread_exit(NULL);
}


/******************************************************************
 * Entrada 
 ******************************************************************/
uint32_t pegarBitsMascara(char numBits) { //pega a mascara
    uint32_t temp = 0b11111111111111111111111111111111;
    numBits = 32 - numBits;
    for (int i = 0; i < numBits; i++) {
        temp = temp << 1;
    }
    return temp;
}

uint32_t * roteamento(entrada * rotas, int num_rotas, uint32_t * pacotes, int num_pacotes, entrada * filtros, int num_filtros, int num_enlaces) {
    queue = criar(); //criando fila
    saida = (uint32_t *) malloc(sizeof (uint32_t) * num_enlaces);

    teste * tmp = malloc(sizeof (teste));
    tmp->rotas = rotas;
    tmp->num_rotas = num_rotas;
    tmp->pacotes = pacotes;
    tmp->num_pacotes = num_pacotes;
    tmp->filtros = filtros;
    tmp->num_filtros = num_filtros;
    tmp->num_enlaces = num_enlaces;

    pthread_t id1, id2, id3, id4, id5;

    //THREAD 0 - Adiciona na Fila
    pthread_create(&id1, NULL, adicionarFila, tmp);
    pthread_join(id1, NULL);

    while (!vazio(queue)) {
        pthread_create(&id2, NULL, processarFila, tmp); //THREAD 1
        pthread_create(&id3, NULL, processarFila, tmp); //THREAD 2
        pthread_create(&id4, NULL, processarFila, tmp); //THREAD 3
        pthread_create(&id5, NULL, processarFila, tmp); //THREAD 4
        pthread_join(id2, NULL);
        pthread_join(id3, NULL);
        pthread_join(id4, NULL);
        pthread_join(id5, NULL);
    }
    free(tmp);
    free(queue);
    return saida;
}