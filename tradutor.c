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
   * 0  - Variavel local de registrador
   * 1  - Variavel local de pilha
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

/*
 * Aloca o espaço na pilha necessario para essa funcao
 * Imprime também o offset em que seram armazenados os parametros (se
 * necessario) e as variaveis locais
 */
void alocaPilha(Func *f) {
  /*
   * Conta o total de variaveis local de pilha e parametros para alocar a pilha
   */
  int bytesPilha = 0;

  // Conta os parametros
  int i = 0;
  while (i < MAX_PARAM) {
    int param = f->param[i];
    if (param == '\0') { // nao tem mais parametros
      break;
    } else if (param == 'i') { // parametro inteiro
      bytesPilha = (bytesPilha + 4);
    } else if (param == 'a') {
      bytesPilha = (bytesPilha + 8); // multiplica por 8 pois se trata de um
    }

    // imprime o offset em que o parametro sera armazenado se necessario
    printf("# p%c%d: -%d(%%rbp)\n", param, i + 1, bytesPilha);
    i++;
  }

  // Conta as variaveis locais
  // Em caso de array é contado o tamanho do array
  i = 0;
  while (i < MAX_VARS) {
    int var = f->var_loc[i];
    if (var == -1) { // nao tem mais variaveis locais
      break;
    } else if (var == 1) { // variavel de registrador
      bytesPilha = bytesPilha + (var * 4);
      // imprime o offset em que a variavel local será armazenada
      printf("# vi%d: -%d(%%rbp)\n", i + 1, bytesPilha);
    } else if (var >= 1) { // array na pilha
      bytesPilha = bytesPilha + (var * 4);
      // imprime o offset em que a primeira posição do array será armazenada
      printf("# va%d: -%d(%%rbp)\n", i + 1, bytesPilha);
    }
    i++;
  }

  if (bytesPilha != 0) {
    // Alocar um espaço de tamanho multiplo de 16 na pilha
    bytesPilha += 16 - (bytesPilha % 16);

    printf("subq $%d, %%rsp\n\n", bytesPilha);
  }
}

/*
 * Imprime o inicio da definição de uma funcao em assembly:
 * Define o simbolo global da função
 * Cria o label do inicio da função
 * Cria o registro de ativação da função
 * Aloca espaço na pilha para as variaveis locais se necessario
 */
void traduzInicioFuncao(Func *f) {
  printf(".globl f%d\n", f->nome);
  printf("f%d:\n\n", f->nome);

  printf("pushq %%rbp\n");
  printf("movq %%rsp, %%rbp\n\n");

  alocaPilha(f);
}

/*
 * Traduz a instrução de retorno da função
 */
void traduzRetorno(int ret) {
  // TODO
  // Traduz apenas retornos de constantes inteiras por enquanto
  printf("movl $%d, %%eax\n", ret);
  printf("leave\n");
  printf("ret\n\n");
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

  printf(".text\n\n");

  // Lê uma linha por vez
  while (fgets(line, LINESZ, stdin) != NULL) {
    count++;
    remove_newline(line);
    // printf("linha %d: %s\n", count, line);

    /*
     * Verifica se a linha começa com function e quantos parametros possui
     */
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

    // Verifica se a linha começa com enddef (fim da definicao de variaveis
    // locais)
    if (strncmp(line, "enddef", 6) == 0) {
      // Traduz o inicio da definicao da funcao para assembly
      traduzInicioFuncao(&funcao);
      continue;
    }

    // Verifica se a linha começa return e qual valor retornado
    r = sscanf(line, "return ci%d", &ret);
    if (r == 1) {
      traduzRetorno(ret);
      continue;
    }

    // Verifica se a linha começa com end (fim da definicao da funcao)
    if (strncmp(line, "end", 3) == 0) {
      // Reinicializa a struct funcao
      inicializaFuncao(&funcao);

      continue;
    }

    // Verificacao dos tipos das variaveis locais

    // Verifica se a linha define uma variavel inteira de registrador
    r = sscanf(line, "reg ri%d", &var_loc_count);
    if (r == 1) {
      funcao.var_loc[var_loc_count - 1] = 0;
      continue;
    }
    // Verifica se a linha define uma variavel inteira de pilha
    r = sscanf(line, "var vi%d", &var_loc_count);
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
