#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

#define NUM_THREADS 8
#define ITERACAO 2

//Relógio
#define TEMPOINICIO 0
#define TEMPOMAX 120

/*Prioridades*/
#define PRIORIDADEGRAVIDA 12
#define PRIORIDADEIDOSO 8
#define PRIORIDADEDEFICIENCIA 4
#define PRIORIDADECOMUM 0

/*Estrutura  de dados para thread Pessoas*/
typedef struct {
    int codigo;     
    int iteracao;
    int prioridade;
    int Chegada;
}Cliente;

/*Estrutura  de dados para thread Relogio*/
typedef struct{ 
    int tempo;
}dadosRelogio;

int caixa = 0;
int pessoasFila = 0;

Cliente cliente[NUM_THREADS];

pthread_cond_t CondPessoas[NUM_THREADS]; 
pthread_mutex_t mutexCaixa = PTHREAD_MUTEX_INITIALIZER;  //caixa
pthread_mutex_t mutexFila = PTHREAD_MUTEX_INITIALIZER; //fila
pthread_cond_t sig_caixa = PTHREAD_COND_INITIALIZER;
pthread_cond_t sig_fila = PTHREAD_COND_INITIALIZER;
dadosRelogio relogio;
int codProxAtende;

/***************************************************************************************************/

char* nome(int codigo) {
    char* nome;
    switch (codigo) {
    case 0: nome = "Vanda";
        break;
    case 1: nome = "Maria";
        break;
    case 2: nome = "Paula";
        break;
    case 3: nome = "Sueli";
        break;
    case 4: nome = "Valter";
        break;
    case 5:    nome = "Marcos";
        break;
    case 6:    nome = "Pedro";
        break;
    case 7:    nome = "Silas";
        break;
    default:
        break;
    }
    return nome;
}

int prioridade(int codigo) {
    int p;
    switch (codigo) {
    case 0: p = PRIORIDADEIDOSO;
        break;
    case 1: p = PRIORIDADEGRAVIDA;
        break;
    case 2: p = PRIORIDADEDEFICIENCIA;
        break;
    case 3: p = PRIORIDADECOMUM;
        break;
    case 4: p = PRIORIDADEIDOSO;
        break;
    case 5:    p = PRIORIDADEGRAVIDA;
        break;
    case 6:    p = PRIORIDADEDEFICIENCIA;
        break;
    case 7:    p = PRIORIDADECOMUM;
        break;
    default:
        break;
    }
    return p;
}

/*
void imprimeFilaAtual(){
	int i;
	printf("Fila: [");
	for (i = 0; i < NUM_THREADS; i++){
		if (cliente[i].Chegada != -1){
			printf("%s:%d ",nome(cliente[i].codigo), cliente[i].prioridade);
		}   
	}
	printf("] \n");
}
*/

/*inicia o relogio da loterica*/
void *iniciaRelogio(void *arg){
	dadosRelogio *relogio = (dadosRelogio *) arg;
	while (1){
		relogio->tempo++;
		sleep(1);
		int i;
	}
}


/*escolhe o proximo a ser atendido de acordo com prioridade, e quantidade de vezes ja usou o caixa*/
int proximoAtender(){
	int i;
	Cliente proximo;
	proximo.codigo = -1;
	for (i = 0; i < NUM_THREADS; i++){
		if ((cliente[i].iteracao>0) && (cliente[i].Chegada > -1)){
			if (proximo.codigo == -1){
				proximo = cliente[i];
			}
			else {
				if(proximo.prioridade < cliente[i].prioridade){
					proximo = cliente[i];
				}
				else if ((proximo.prioridade == cliente[i].prioridade) && (proximo.Chegada > cliente[i].Chegada)){
					proximo = cliente[i];
				}
			}
		}
	}
	return proximo.codigo;
}

/*CONSUMIDOR*/
void Caixa(int codigo){
	pthread_mutex_lock(&mutexCaixa);
	if (pessoasFila >0){
		caixa = 0;
		codProxAtende = proximoAtender();
		if (codProxAtende != -1){
			printf("%s vai ser atendido(a)\n", nome(codProxAtende));
			pthread_cond_signal(&CondPessoas[codProxAtende]);
			sleep(5);
			printf("%s foi para casa\n", nome(cliente[codProxAtende].codigo));
			cliente[codProxAtende].iteracao = cliente[codProxAtende].iteracao -1;
			cliente[codProxAtende].prioridade = prioridade(codProxAtende);
			cliente[codProxAtende].Chegada = -1;
            
			/* imprimeFilaAtual();*/
		}
	}
	pthread_mutex_unlock(&mutexCaixa);    
}

/*verifica quem que usar o caixa*/
void esperar(int codigo) {                
	pthread_mutex_lock(&mutexCaixa);
	if (caixa == 1)
		pthread_cond_wait(&CondPessoas[codigo], &mutexCaixa);
			caixa = 1;
    pthread_mutex_unlock(&mutexCaixa);
}

/* chamar as interatividade do sistema até as interações zerarem - cada pessoa pode ser atendida 2 vezes*/
void *iniciaPessoas(void *arg) { 
	Cliente *threadPessoa = (Cliente *) arg;
	cliente[threadPessoa->codigo].Chegada = relogio.tempo;
	printf("%s entrou na fila caixa\n", nome(threadPessoa->codigo));
	/*imprimeFilaAtual();*/
	while (threadPessoa->iteracao > 0) {
		pthread_mutex_lock(&mutexFila);            
		int i;
		for (i = 0 ; i < NUM_THREADS ; i++){
			if (cliente[i].Chegada!= -1){                
				cliente[i].prioridade ++;
			}
		}
		pessoasFila++;
		esperar(threadPessoa->codigo);
		sleep(1);
		Caixa(threadPessoa->codigo);
		sleep(5);
		pessoasFila--;
		pthread_mutex_unlock(&mutexFila);
	}
	pthread_exit(NULL );
}

int faltaUmSorteio(){
	int contador;
	for (contador = 0; contador < NUM_THREADS ; contador++){
		if (verificaSorteio(contador) == 0){
			return contador;        
		}
	}
	return contador;
}

/*verifica se a pessoa já está na fila*/ 
int verificaSorteio(int numeroSorteado){
	int cont;
	if ((cliente[numeroSorteado].Chegada != -1)||(cliente[numeroSorteado].iteracao == 0)){
		return 1;        
	}
	return 0;
}

/*Cria thread das pessoas aleatoriamente*/
void sorteiaFila(pthread_t threads[], Cliente clientes[]) {
	int i = 0;
	srand((unsigned)time(NULL ));
	while ((i < (NUM_THREADS*ITERACAO))&&(relogio.tempo != TEMPOMAX)) {
		int n = (rand() % NUM_THREADS);  //Sorteia o numero diferente a pessoa
		if (verificaSorteio(n) == 0){
			pthread_create(&threads[i], NULL, iniciaPessoas, &clientes[n]);  //cria a thread das Pessoas
			i++;
			sleep(2);
		}

		int j, finalizou = 1;
		for(j=0; j< NUM_THREADS; j++){
			if(cliente[j].iteracao > 0){
				finalizou = 0;                
			}
		}
		if(finalizou == 1){
			break;  
		}
	}
	printf("Loterica Fechou!! Apenas as pessoas que já estavam na fila serão atendidas.\n");
}

int main(int argc, char **argv) {
	int i;
	pthread_t threadsClientes[NUM_THREADS];

	/*inicia a thread de relogio da loterica*/
	pthread_t threadRelogio;
	relogio.tempo = TEMPOINICIO;
	pthread_create(&threadRelogio, NULL, iniciaRelogio, &relogio); 

	for (i = 0; i < NUM_THREADS; i++) {
		cliente[i].codigo = i; //Criando pessoas de código de 0 a 7 - cada número referente a uma pessoa
		cliente[i].iteracao = ITERACAO; //Cada pessoa só pode ir na loterica duas vezes.
		cliente[i].prioridade = prioridade(cliente[i].codigo);
		cliente[i].Chegada = -1;
	}
	sorteiaFila(threadsClientes, cliente);
	pthread_join(threadRelogio, NULL);

	for (i = 0; i < NUM_THREADS; i++) {
		pthread_join(threadsClientes[i], NULL );
	}
  return EXIT_SUCCESS;
}
