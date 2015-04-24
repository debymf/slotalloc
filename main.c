#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "genetica.h"
#include "voos.h"

#define INICIO 960
#define MAX_POP 200
#define MAX_GEN 10000


struct Dados_voos voos[100];
int num_voos=0;
struct Slots slots_voos[240];
struct Populacao pop;
struct Populacao pop_temp;
int empresas[9];

int rdtsc()    
{    
	__asm__ __volatile__("rdtsc");    
} 


void lerDados(){
	FILE *fp;
	char buffer[100];

	fp = fopen("dia20_sempico_sol.csv", "r");
	 
	while(fgets(buffer, 200, fp) != NULL)
	{
		voos[num_voos].companhia_aerea = atoi(strtok(buffer,","));
		voos[num_voos].id = atoi(strtok(NULL,","));
		voos[num_voos].status = atoi(strtok(NULL,","));
		voos[num_voos].previsto = atoi(strtok(NULL,","));
		voos[num_voos].confirmado = atoi(strtok(NULL,","));
		num_voos++;
		
	}	
}

void alocarSlots(){
	int i;
	int posicao;
	
	for(i=0;i<240;i++){
		slots_voos[i].id_slot = i+1;
		slots_voos[i].hora_slot = INICIO+i;
		slots_voos[i].id_voo = 0;
		slots_voos[i].hora_voo = 0;
		slots_voos[i].fixado = 0;
		slots_voos[i].hora_voo_real=0;
	}
	
	for(i=0;i<9;i++){
		empresas[i] = 0;
	}
	
	for(i=0;i<num_voos;i++){
		if((voos[i].status==2)&&(voos[i].confirmado<=(INICIO+240))){
			posicao = voos[i].confirmado - INICIO;
			slots_voos[posicao].id_slot = voos[i].id;
			slots_voos[posicao].id_voo = voos[i].id;
			slots_voos[posicao].hora_voo = voos[i].previsto;
			slots_voos[posicao].hora_voo_real = voos[i].confirmado;
			slots_voos[posicao].empresa = voos[i].companhia_aerea;
			slots_voos[posicao].fixado = 1;
			empresas[voos[i].companhia_aerea] = 1;
		}
	}
	
}

void geraCromossomo(struct Slots *individuo){
	int i;
	int random_number;
	int cont=1;
	for(i=0;i<240;i++){
		individuo[i].id_slot =  slots_voos[i].id_slot;
		individuo[i].hora_slot = slots_voos[i].hora_slot; 
		individuo[i].id_voo = slots_voos[i].id_voo;
		individuo[i].hora_voo = slots_voos[i].hora_voo;
		individuo[i].fixado = slots_voos[i].fixado;
		individuo[i].hora_voo_real=slots_voos[i].hora_voo_real;
		individuo[i].empresa=slots_voos[i].empresa;
	}
	
	for(i=0;i<num_voos;i++){
		if((voos[i].status!=2)&&(voos[i].status!=1)){
			srand ( time(NULL) );
			random_number = rand()%240;
			while(individuo[random_number].id_voo!=0){
				srand(rdtsc());
				random_number = rand()%240;
			}
			individuo[random_number].id_slot = voos[i].id;
			individuo[random_number].id_voo = voos[i].id;
			individuo[random_number].hora_voo = voos[i].previsto;
			individuo[random_number].hora_voo_real = voos[i].confirmado;
			individuo[random_number].empresa = voos[i].companhia_aerea;
		}
	}
	
	for(i=0;i<240;i++){
		if(individuo[i].id_voo==0){
			individuo[i].id_slot = cont;
			cont++;
		}
	}
}

float calculaFitness(struct Slots *individuo){
	int i;
	int bonus=0;
	int atraso=0;
	int atraso_total=0;
	float fitness;
	
	for(i=0;i<240;i++){
		if(empresas[individuo[i].empresa]==1){
			if(((individuo[i].hora_voo_real)-(individuo[i].hora_slot))>0){
				bonus=bonus+1;
			}
		}
		if(individuo[i].hora_voo!=0){
			atraso = (individuo[i].hora_slot)-(individuo[i].hora_voo);
			if(atraso<(-5)){
				atraso = abs(atraso);
			}
			atraso_total = atraso_total + atraso;
		}
	}
	
	
	fitness = (1/(float )atraso_total);
	if (bonus>0)
		fitness = fitness * bonus;
	
	return fitness;
}

void geraPopulacao(){
	int j;

	for (j=0;j<MAX_POP;j++){
		geraCromossomo(&pop.populacao[j].individuos[0]);
		pop.populacao[j].fitness = calculaFitness(&pop.populacao[j].individuos[0]);
	}
	
}

int selecionaProgenitor(){
	struct Roleta rolar[MAX_POP];
	int i;
	int achou=0;
	double sorteio;
	int selecionado;
	double total_fitness=0;
	
	for(i=0;i<MAX_POP;i++){
		total_fitness += pop.populacao[i].fitness;
	}
	
	for(i=0;i<MAX_POP;i++){
		rolar[i].probabilidade = (pop.populacao[i].fitness)/total_fitness;
	}
	
	rolar[0].acumulada=0;
	for(i=1;i<MAX_POP;i++){
		rolar[i].acumulada = rolar[i].probabilidade + rolar[i-1].acumulada;
		
	}
	
	while(achou==0){
		srand(rdtsc()); 
		sorteio  = rand()/(double)RAND_MAX;
		for(i=0;i<MAX_POP;i++){
			if((sorteio>=rolar[i].acumulada)&&(sorteio<((rolar[i].acumulada)+(rolar[i].probabilidade)))){
				selecionado = i;
				achou=1;
				break;
			}
		}
	}
	return selecionado;
}

void recombinar(struct Cromossomo pai, struct Cromossomo mae, int cont){
	int binario[240];
	int temp1[240];
	int temp2[240];
	int i,j,k;
	int achou=0;
	int random_number;
	struct Cromossomo filho1, filho2;
	
	/* gera string binaria aleatoria*/
	for(i=0;i<240;i++){
		srand(rdtsc());  
		if((rand()%10)>6){
			binario[i]=1;
		}
		else{
			binario[i]=0;
			
		}
	}

	/*Organiza os proximos filhos*/
	
	for(i=0;i<240;i++){
		filho1.individuos[i].id_slot=0;
		filho1.individuos[i].hora_slot = 0;
		filho1.individuos[i].id_voo = 0;
		filho1.individuos[i].hora_voo = 0;
		filho1.individuos[i].fixado = 0;
		
		filho2.individuos[i].id_slot=0;
		filho2.individuos[i].hora_slot = 0;
		filho2.individuos[i].id_voo = 0;
		filho2.individuos[i].hora_voo = 0;
		filho2.individuos[i].fixado = 0;
	}
	
	
	for(i=0;i<240;i++){
		if (binario[i]==1){
			filho1.individuos[i] = mae.individuos[i];
			filho2.individuos[i] = pai.individuos[i];
		}
	}

	for(i=0;i<240;i++){
		if(binario[i]==0){
			for(j=0;j<240;j++){
				achou=0;
				for(k=0;k<240;k++){
					if(mae.individuos[j].id_slot==filho2.individuos[k].id_slot){
						achou=1;
						break;
					}
				}
				if (achou==0){
					filho2.individuos[i] = mae.individuos[j];
					break;
				}
				
			}
		}
		
	}
	
	for(i=0;i<240;i++){
		if(binario[i]==0){
			for(j=0;j<240;j++){
				achou=0;
				for(k=0;k<240;k++){
					if(pai.individuos[j].id_slot==filho1.individuos[k].id_slot){
						achou=1;
						break;
					}
				}
				if (achou==0){
					filho1.individuos[i] = pai.individuos[j];
					break;
				}
				
			}
		}
		
	}
	
	pop_temp.populacao[cont]=filho1;
	pop_temp.populacao[cont+1]=filho2;
}

void mutar(int cont){
	int random_number;
	int i,j;
	struct Slots temp;
	
	srand(rdtsc());  
	random_number = rand()%100;
	
	if (random_number==5){
		do{	srand(rdtsc());  
			i = rand()%240;
			temp = pop.populacao[cont].individuos[i];
		}while(temp.fixado==1);
		do{
			srand(rdtsc()); 
			j = rand()%240;
		}while(pop.populacao[cont].individuos[j].fixado==1);
		pop.populacao[cont].individuos[i] = pop.populacao[cont].individuos[j];
		pop.populacao[cont].individuos[j] = temp;
	}
}

void realocarSlots(){
	int p1,p2,cont=0;
	int i,j,k;
	float maior=0;
	int hora;
	int min;
	int dono_maior;
	FILE *fp;
	clock_t begin, end;
	double time_spent;

	begin = clock();
	
	struct Cromossomo pai;
	struct Cromossomo mae;
	
	geraPopulacao();
	fp = fopen("resultados2/converge.csv", "w");
	for(j=0;j<300000;j++){
		if ((j%100)==0){
			fprintf(fp,"Geracao: %d\n", j);
		}
		cont=0;
		for(i=0;i<(MAX_POP/2.5);i++){
			p1 = selecionaProgenitor();
			p2 = selecionaProgenitor();
			
			pai = pop.populacao[p1];
			mae = pop.populacao[p2];
			recombinar(pai,mae,cont);
			
			pop_temp.populacao[cont].fitness = calculaFitness(&pop_temp.populacao[cont].individuos[0]);
			pop_temp.populacao[cont+1].fitness = calculaFitness(&pop_temp.populacao[cont+1].individuos[0]);
			
			mutar(cont);
			mutar(cont+1);
			cont=cont+2;
		}
		for(i=0;i<40;i++){
			maior=0;
			for(k=0;k<MAX_POP;k++){
				
				if (pop.populacao[k].fitness>maior){
				
					maior = pop.populacao[k].fitness;
					dono_maior=k;
					if(i==0){
						fprintf(fp,"\n\n\nFITNESS: %f\n", maior);
					}
				}
				
			}
			pop_temp.populacao[cont] = pop.populacao[dono_maior];
			pop.populacao[dono_maior].fitness = 0; 
			cont++;
		}
		pop=pop_temp;
	}
	maior=0;
	for(i=0;i<MAX_POP;i++){
		if (pop.populacao[i].fitness>maior){
			maior = pop.populacao[i].fitness;
			dono_maior=i;
		}
	}
	
	
	for(i=0;i<240;i++){
		if(pop.populacao[dono_maior].individuos[i].id_voo!=0){
			hora = pop_temp.populacao[dono_maior].individuos[i].hora_slot/60;
			min = pop_temp.populacao[dono_maior].individuos[i].hora_slot%60;
			if (min<10){
				fprintf(fp,"%d,%d,%d:0%d,",pop_temp.populacao[dono_maior].individuos[i].empresa,pop_temp.populacao[dono_maior].individuos[i].id_voo,hora,min);
			}
			else{
				fprintf(fp,"%d,%d,%d:%d,",pop_temp.populacao[dono_maior].individuos[i].empresa,pop_temp.populacao[dono_maior].individuos[i].id_voo,hora,min);
			}
			hora = pop_temp.populacao[dono_maior].individuos[i].hora_voo/60;
			min = pop_temp.populacao[dono_maior].individuos[i].hora_voo%60;
			if(min<10){
				fprintf(fp,"%d:0%d\n",hora,min);	
			}else{
				fprintf(fp,"%d:%d\n",hora,min);	
			}
				
		}
		
		
	}
	
	fprintf(fp,"\n\n\nFITNESS: %f\n", maior);
	end = clock();
	time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
	fprintf(fp, "TEMPO: %lf\n", time_spent);
}

int main(){
	lerDados();
	alocarSlots();
	realocarSlots();
	
	return 0;
}
