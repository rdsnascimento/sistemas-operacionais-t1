#include "simplegrade.h"
#include "roteador.h"

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
void imprime(entrada * rotas, int num_rotas, uint32_t * pacotes, int num_pacotes, entrada * filtros, int num_filtros, int num_enlaces, uint32_t * resultado){
	printf("\nTabela de Roteamento: ");
	printf("\t");
	for(int i=0; i < num_rotas; i++){
		printByte(rotas[i].endereco);
		printf("/%d ", rotas[i].mascara);
		
		if((i+1) != num_rotas){
			printf("%d\t", rotas[i].enlace);
		}else{
			printf("%d\n", rotas[i].enlace);
		}
	}
	
	if(num_filtros!=0){
		printf("Lista a ser filtrada: \t");
		for(int i=0; i < num_filtros; i++){
			printByte(filtros[i].endereco);
			printf("/%d ", filtros[i].mascara);
			
			if((i+1) != num_filtros){
				printf("%d\t", filtros[i].enlace);
			}else{
				printf("%d\n", filtros[i].enlace);
			}
		}
	}
	
	printf("Fila de Entrada: \t");
	for(int i=0; i < num_pacotes; i++){
		printByte(pacotes[i]);
		if((i+1) != num_pacotes){
			printf("\t");
		}else{
			printf("\n");
		}
	}
	
	printf("Pacote(s) encaminhado(s) para:\n");
	for(int i=0; i < num_enlaces; i++){
		printf("Enlace [%d]: %d\n", i, resultado[i]);
	}
	printf("\n");
}



void test(){
	struct roteamento rotas[] = {{0b0010011000001010000000000000000, 16, 2}, {0b0010011000001010000010100000000, 24, 1}}; //19.5.0.0/16 2 e  19.5.5.0/24 1
	int num_rotas = 2;
	uint32_t pacotes [] = {0b0010011000001010000010100000000}; //19.5.5.0
	int num_pacotes = 1;
	struct roteamento filtros[] = {}; 
	int num_filtros = 0;
	int num_enlaces = 3;
	uint32_t * resultado = roteamento(rotas, num_rotas, pacotes, num_pacotes, filtros, num_filtros, num_enlaces);
	
	DESCRIBE("A tabela de roteamento é composta por 19.5.0.0/16 2 e 19.5.5.0/24 1");
	IF("Se o ip de entrada for 19.5.5.0");
	THEN("Espero que a saida vá para o enlace 1 porque tem maior prioridade");
	isEqual(resultado[1], 1, 1);
	imprime(rotas, num_rotas, pacotes, num_pacotes, filtros, num_filtros, num_enlaces, resultado);
	
	uint32_t pacotes1 [] = {0b0010011000001010000000000000001}; //19.5.0.1
	resultado = roteamento(rotas, num_rotas, pacotes1, num_pacotes, filtros, num_filtros, num_enlaces);
	IF("Se o ip de entrada for 19.5.0.1");
	THEN("Espero que a saida vá para o enlace 1 porque tem maior prioridade");
	isEqual(resultado[2], 1, 1);
	imprime(rotas, num_rotas, pacotes1, num_pacotes, filtros, num_filtros, num_enlaces, resultado);
	
	uint32_t pacotes2 [] = {0b11111110000000000000000000000001}; //254.0.0.1 
	resultado = roteamento(rotas, num_rotas, pacotes2, num_pacotes, filtros, num_filtros, num_enlaces);
	IF("Se o ip de entrada for 254.0.0.1");
	THEN("Não está em nenhuma rota, espero que seja rejeitado");
	isEqual(resultado[0], 1, 1);
	imprime(rotas, num_rotas, pacotes2, num_pacotes, filtros, num_filtros, num_enlaces, resultado);
	
	
	uint32_t pacotes3 [] = {0b0010011000001010000000000000001, 0b11111110000000000000000000000001, 0b0010011000001010000010100000000}; //19.5.5.0, 19.5.0.1, 254.0.0.1 
	num_pacotes = 3;
	resultado = roteamento(rotas, num_rotas, pacotes3, num_pacotes, filtros, num_filtros, num_enlaces);
	IF("Se a entrada for uma lista de pacotes com os endereços 19.5.5.0, 19.5.0.1 e 254.0.0.1");
	THEN("Espero que seja encaminhados para os seus respectivos enlaces de saída");
	isEqual(resultado[0], 1, 1);
	isEqual(resultado[1], 1, 1);
	isEqual(resultado[2], 1, 1);
	imprime(rotas, num_rotas, pacotes3, num_pacotes, filtros, num_filtros, num_enlaces, resultado);
}

void test2(){
	struct roteamento rotas[] = {{0b0010011000001010000000000000000, 16, 2}, {0b0010011000001010000010100000000, 24, 1}}; //19.5.0.0/16 2 e  19.5.5.0/24 1
	int num_rotas = 2;
	uint32_t pacotes [] = {0b11111111000000000000000000000001, 0b0010011000001010000010100000001, 0b0010011000001010000000000000001}; //255.0.0.1, 19.5.5.1, 19.5.0.1
	int num_pacotes = 3;
	struct roteamento filtros[] = {{0b11111111000000000000000000000000, 16, 0}, {0b0010011000001010000010100000000, 24, 0}};
	int num_filtros = 2;
	int num_enlaces = 3;
	uint32_t * resultado = roteamento(rotas, num_rotas, pacotes, num_pacotes, filtros, num_filtros, num_enlaces);
	
	DESCRIBE("A tabela de roteamento é composta por 19.5.0.0/16 2 e 19.5.5.0/24 1");
	IF("Tenho dois pacotes para ser filtrado (255.0.0.1 e 19.5.5.1)");
	THEN("Espero que rejeite os dois e mande o endereço 19.5.0.1 para o Enlace 2");
	isEqual(resultado[0], 2, 1);
	isEqual(resultado[1], 0, 1);
	isEqual(resultado[2], 1, 1);
	imprime(rotas, num_rotas, pacotes, num_pacotes, filtros, num_filtros, num_enlaces, resultado);
}


int main(int argc __attribute__ ((unused)), char ** argv __attribute__ ((unused))){
	DESCRIBE("Simulador\n");
	test();
	test2();

	GRADEME();
	if (grade==maxgrade)
		return 0;
	else return grade;

}
