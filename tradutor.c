#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINESZ 256
#define MAX_PARAM 3 // Maximo de 3 parametros por funcao
#define MAX_VARS 12 // Maximo de 12 variaveis locais por funcao

typedef struct {
  int nome;

  /*
   * Vetor de caracteres que identifica o tipo de cada parametro
   * \0 - Sem parametro
   * a  - array
   * i  - vetor
   */
  char param[MAX_PARAM];

  /*
   * Vetor que inteiros que identifica o tipo de cada variavel local
   * -1 - Sem variavel local
   * 0  - Variavel local de pilha
   * 1  - Variavel local de registrador
   * n  - Array de n posicoes
   */
  int var_loc[MAX_VARS];

} Func;

/*
 * Inicializa a funcao tornando todos os campos da struct nulos
 */
void inicializaFuncao(Func *f) {
  f->nome = 0;

  // Inicializa todos os parametros como '\0'
  for (int i = 0; i < MAX_PARAM; i++) {
    f->param[i] = '\0';
  }

  // Inicializa todas as variaveis locais como -1
  for (int i = 0; i < MAX_VARS; i++) {
    f->var_loc[i] = -1;
  }
}

// Remove o '\n' do fim da linha
void remove_newline(char *ptr) {
  while (*ptr) {
    if (*ptr == '\n')
      *ptr = 0;
    else
      ptr++;
  }
}

int main() {
  char line[LINESZ];
  int count = 0; // contagem de linhas lidas
  int r;         // quantidade de valores lidos pelo sscanf
  int f;         // nome (numero) da funcao atual
  Func funcao = {.nome = 0,
                 .param = {'\0', '\0', '\0'},
                 .var_loc = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}};
  int var_loc_count; // contagem do total de variaveis locais da funcao
  int ret;           // valor retornado pela funcao atual

  // Lê uma linha por vez
  while (fgets(line, LINESZ, stdin) != NULL) {
    count++;
    remove_newline(line);
    printf("linha %d: %s\n", count, line);

    // Verifica se a linha começa com function e quantos parametros possui

    // Funcao de 3 parametros
    r = sscanf(line, "function f%d p%c%*d p%c%*d p%c%*d", &funcao.nome,
               &funcao.param[0], &funcao.param[1], &funcao.param[2]);
    if (r == 4) {
      continue;
    }

    // Funcao de 2 parametros
    r = sscanf(line, "function f%d p%c%*d p%c%*d", &funcao.nome,
               &funcao.param[0], &funcao.param[1]);
    if (r == 3) {
      continue;
    }

    // Funcao de 1 parametro
    r = sscanf(line, "function f%d p%c%*d", &funcao.nome, &funcao.param[0]);
    if (r == 2) {
      continue;
    }

    // Funcao sem parametros
    r = sscanf(line, "function f%d", &f);
    if (r == 1) {
      continue;
    }

    // Verifica se a linha começa com def (definicao de variaveis locais)
    if (strncmp(line, "def", 3) == 0) {
      continue;
    }

    // Verifica se a linha começa com enddef (fim da definicao de variaveis locais)
    if (strncmp(line, "enddef", 6) == 0) {
      continue;
    }

    // Verifica se a linha começa return e qual valor retornado
    r = sscanf(line, "return ci%d", &ret);
    if (r == 1) {
      continue;
    }

    // Verifica se a linha começa com end (fim da definicao da funcao)
    if (strncmp(line, "end", 3) == 0) {
      // Reinicializa a struct funcao
      inicializaFuncao(&funcao);

      continue;
    }

    // Verificacao dos tipos das variaveis locais
    
    // Verifica se a linha define uma variavel inteira de pilha
    r = sscanf(line, "var vi%d", &var_loc_count);
    if (r == 1) {
      funcao.var_loc[var_loc_count - 1] = 0;
      continue;
    }
    // Verifica se a linha define uma variavel inteira de registrador
    r = sscanf(line, "reg ri%d", &var_loc_count);
    if (r == 1) {
      funcao.var_loc[var_loc_count - 1] = 1;
      continue;
    }
    // Verifica se a linha define um array e qual o seu tamanho
    unsigned int tamanho_vetor;
    r = sscanf(line, "vet va%d size ci%d", &var_loc_count, &tamanho_vetor);
    if (r == 2) {
      funcao.var_loc[var_loc_count - 1] = tamanho_vetor;
      continue;
    }
  }

  return 0;
}
