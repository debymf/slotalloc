#include "slots.h"

struct Cromossomo{
	struct Slots individuos[240];
	float fitness;
};

struct Populacao{
	struct Cromossomo populacao[200];
};

struct Roleta{
	float acumulada;
	float probabilidade;
};