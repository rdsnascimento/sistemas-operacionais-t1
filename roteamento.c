//RAFAEL DE SOUSA NASCIMENTO 15200084
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h> 

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

uint32_t pegarBitsMascara(char numBits) { //pega a mascara
    uint32_t temp = 0b11111111111111111111111111111111;
    numBits = 32 - numBits;
    for (int i = 0; i < numBits; i++) {
        temp = temp << 1;
    }
    return temp;
}

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
 * Solução
 ******************************************************************/
uint32_t * roteamento(entrada * rotas, int num_rotas, uint32_t * pacotes, int num_pacotes, entrada * filtros, int num_filtros, int num_enlaces) {
    fila *q = criar(); //criando fila
    adicionar(q, pacotes, num_pacotes); //adicionando na fila

    uint32_t tmpMaskRota = 0;
    uint32_t tmpMaskPedido = 0;
    uint32_t guardaMascara = 0;
    int guardaIndice = -1;
    uint32_t rejeita;
    uint32_t * saida = (uint32_t *) malloc(sizeof (uint32_t) * num_enlaces);

    while (!vazio(q)) { //enquanto a fila nao estiver vazia
        elemento *aux = remover(q); //tira o primeiro da fila e processa

        rejeita = 0;
        guardaIndice = -1;

        //passa o pedido pelo filtro
        for (int k = 0; k < num_filtros; k++) {
            tmpMaskRota = pegarBitsMascara(filtros[k].mascara);
            tmpMaskRota = filtros[k].endereco & tmpMaskRota; //tenho a mascara do filtro aqui
            tmpMaskPedido = pegarBitsMascara(filtros[k].mascara);
            tmpMaskPedido = (aux->endereco) & tmpMaskPedido;

            if (tmpMaskPedido == tmpMaskRota) { //rejeita pacote
                rejeita = 1;
                break;
            }
        }

        //se o pedido nao foi rejeitado
        if (rejeita == 0) {
            for (int j = 0; j < num_rotas; j++) { //cada pacote analisa todas as rotas a ser encaminhada
                tmpMaskRota = pegarBitsMascara(rotas[j].mascara);
                tmpMaskRota = rotas[j].endereco & tmpMaskRota; //tenho a mascara da rota aqui
                tmpMaskPedido = pegarBitsMascara(rotas[j].mascara);
                tmpMaskPedido = (aux->endereco) & tmpMaskPedido; //tenho a mascara do pedido aqui
                if (tmpMaskPedido == tmpMaskRota) { //se tiver os mesmos prefixos (a mascara do pacote for igual a mascara da rota)
                    if (aux->endereco >= tmpMaskRota) { //se ip do pacote que esta entrando eh maior ou igual a mascara da rota
                        if (tmpMaskRota > guardaMascara) { //guarda a mascara atual e o indice se a condicao for verdade, ou seja, tem maior prefixo
                            guardaMascara = tmpMaskRota;
                            guardaIndice = j;
                        } else {
                            if ((tmpMaskRota == guardaMascara)&&(rotas[j].mascara > rotas[(int) guardaIndice].mascara)) { //se a mascara for igual, mas tem um prefixo de maior comprimento
                                guardaIndice = j;
                            }
                        }
                    }
                }
            }
        }
        if (guardaIndice != -1) {
            saida[(int) rotas[guardaIndice].enlace]++;
        } else {
            saida[0]++;
        }
        guardaMascara = 0;
    }
    free(q);
    return saida;
}

/**
//imprime num em binario
void printBits(void const * const ptr){
    unsigned char *b = (unsigned char*) ptr;
    unsigned char byte;
    
    for (int i=sizeof(uint32_t)-1; i>=0; i--){
        for (int j=7;j>=0;j--){
            byte = (b[i]>>j) & 1;
            printf("%u", byte);
        }
        if(i>0){
                        printf(".");
                }
    }
    printf("\n");
}

//imprime ip
void printByte(uint32_t num){
        uint32_t temp = num;
    for(int i=0; i<4; i++){
                if(i==0){
                        printf("%d.", temp >> 24);
                }else{
                        num = num << 8;
                        temp = num >> 24;
                        if(i<3){
                                printf("%d.", temp);
                        }else{
                                printf("%d", temp);
                        }
                }
        }
}

//funcao que chama impressao
void imprime(uint32_t num){
        //printf("Decimal: \t%d\n", num); 
        printf("End. IP: \t"); 
        printByte(num);
        printf("Binário: \t");
        printBits(&num);
        printf("\n");
}**/
