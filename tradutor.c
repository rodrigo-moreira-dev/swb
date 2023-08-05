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
   * a  - parametro array
   * i  - parametro inteiro
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
 * Calcula o offset que um parametro ou uma variavel local está armazenada na
 * pilha
 *
 * char tipo:
 *  'p' - parametro
 *  'v' - variavel local
 *
 * int posicao:
 *  a posição da variavel local ou do parametro por exemplo
 *  1 para pi1 ou vi1
 *  2 para pi2 ou vi2
 */
int getOffset(Func f, char tipo, int posicao) {
  int offset = 0;

  /*
   * Conta a quantidade de parametros e os tipos de cada um deles
   * Para calcular a quantidade de bytes necessaria para o offset
   */
  int i = 0;
  while (i < MAX_PARAM) {
    char tipo_parametro = f.param[i];

    if (tipo_parametro == '\0') { // nao tem mais parametros
      break;
    } else if (tipo_parametro == 'i') { // parametro inteiro
      offset = (offset + 4); // soma 4 no offset pois é o tamanho do inteiro
    } else if (tipo_parametro == 'a') { // parametro array
      // Soma um padding pois os ponteiros tem tamanho 8 e devem estar numa
      // posicao multipla de 8
      if (offset != 0) {
        offset += 8 - (offset % 8);
      }

      offset = (offset + 8); // soma 8 pois é o tamanho de um ponteiro
    }

    // retorna o offset atual pois é o da variavel desejada
    if (tipo == 'p' && posicao == (i + 1)) {
      return offset;
    }
    i++;
  }

  /*
   * Conta a quantidade de variaveis locais e os tipos de cada uma delas
   * Para calcular a quantidade de bytes necessaria para o offset
   *
   * Caso a variavel local seja um array, será levado em conta o seu tamanho
   */
  i = 0;
  while (i < MAX_VARS) {
    int tipo_var_loc = f.var_loc[i];

    if (tipo_var_loc == -1) { // nao tem mais variaveis locais
      break;
    } else if (tipo_var_loc == 1) { // variavel local de pilha
      offset = offset + 4;          // soma 4 pois é o tamanho de uma variavel
                                    // local (todas são inteiro)
    } else if (tipo_var_loc >= 1) { // variavel local array
      offset =
          offset + (tipo_var_loc * 4); // soma o tamanho do array multiplicada
                                       // por 4 (tamanho de um inteiro)
    }

    // retorna o offset atual pois é o da variavel desejada
    if (tipo == 'v' && posicao == (i + 1)) {
      return offset;
    }

    i++;
  }
  return 0;
}

/*
 * Aloca o espaço na pilha necessario para essa funcao
 * Imprime também o offset em que seram armazenados os parametros (se
 * necessario) e as variaveis locais
 */
void alocaPilha(Func f) {
  int bytesPilha = 0;
  /*
   * Imprime os offsets dos parametros
   */
  int i = 0;
  while (i < MAX_PARAM) {
    char tipo_parametro = f.param[i];

    if (tipo_parametro == '\0') { // nao tem mais parametros
      break;
    } else { // parametro inteiro
      bytesPilha = getOffset(f, 'p', (i + 1));
      // imprime o offset em que o parametro sera armazenado se necessario
      printf("# p%c%d: -%d(%%rbp)\n", tipo_parametro, i + 1, bytesPilha);

      i++;
    }
  }

  /*
   * Imprime os offsets das variaveis locais
   */
  i = 0;
  while (i < MAX_VARS) {
    int tipo_var_loc = f.var_loc[i];

    if (tipo_var_loc == -1) { // nao tem mais variaveis locais
      break;
    } else if (tipo_var_loc == 1) { // variavel local de pilha
      bytesPilha = getOffset(f, 'v', (i + 1));
      // imprime o offset em que a variavel local de pilha será armazenada
      printf("# vi%d: -%d(%%rbp)\n", i + 1, bytesPilha);
    } else if (tipo_var_loc >= 1) { // variavel local array
      bytesPilha = getOffset(f, 'v', (i + 1));
      // imprime o offset em que a primeira posição do array será armazenada
      printf("# va%d: -%d(%%rbp)\n", i + 1, bytesPilha);
    }

    i++;
  }

  if (bytesPilha != 0) {
    // Calcula um tamanho que seja multiplo de 16 para reservar na pilha
    bytesPilha += 16 - (bytesPilha % 16);

    printf("subq $%d, %%rsp\n\n", bytesPilha);
  }
}

/*
 * Dado o numero de uma variavel local
 * Se ela for de registrador irá retornar em qual registrador ela esta
 * armazenada
 *
 * Como as variaveis locais de registrador são armazenadas nos registradores
 * r8 a r12 esta função retorna apenas o numero do registrador
 */
int getRegistrador(Func f, int variavel_local) {
  int i = 0;
  int regCount = 0; // Contagem de variaveis de registrador
  while (i < MAX_VARS) {
    int var = f.var_loc[i];
    if (var == -1) { // nao tem mais variaveis locais
      break;
    } else if (var == 0) { // variavel de registrador
      if (i + 1 == variavel_local) {
        return regCount + 8;
      }
      regCount++;
    }
    i++;
  }

  return 0;
}

/*
 * "Aloca" os registradores para as variaveis locais de registrador
 *
 * Como nenhuma alocação é realmente necessaria esta função apenas imprime um
 * comentario com o registrador em que será armazenada cada variavel local de
 * registrador
 */
void alocaRegistradores(Func f) {
  int i = 0;
  while (i < MAX_VARS) {
    int var = f.var_loc[i];
    if (var == -1) { // nao tem mais variaveis locais
      break;
    } else if (var == 0) { // variavel de registrador
      printf("# va%d: %%r%d\n", i + 1, getRegistrador(f, i + 1));
    }
    i++;
  }

  printf("\n");
}

/*
 * Imprime o inicio da definição de uma funcao em assembly:
 * Cria o registro de ativação da função
 * Aloca espaço na pilha para as variaveis locais se necessario
 */
void traduzInicioFuncao(Func f) {
  printf(".globl f%d\n", f.nome);
  printf("f%d:\n\n", f.nome);

  printf("pushq %%rbp\n");
  printf("movq %%rsp, %%rbp\n\n");

  alocaPilha(f);
  alocaRegistradores(f);
}

/*
 * Traduz a instrução de retorno da função
 */
void traduzRetorno(int ret) {
  // TODO
  // Traduz apenas retornos de constantes inteiras por enquanto
  printf("movl $%d, %%eax\n", ret);
  printf("jmp fim\n\n");
}

void traduzFimFuncao() {
  printf("fim:\n");
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
  int line_count = 0; // contagem de linhas lidas
  int r;              // quantidade de valores lidos pelo sscanf
  int var_loc_count;  // contagem do total de variaveis locais da funcao
  int ret;            // valor retornado pela funcao atual

  Func funcao;
  inicializaFuncao(&funcao);

  // Inicio do programa em assembly
  printf(".text\n\n");

  // Lê uma linha por vez
  while (fgets(line, LINESZ, stdin) != NULL) {
    line_count++;
    remove_newline(line);
    // printf("linha %d: %s\n", count, line);

    /*
     * Inicio da definição de uma função
     */
    // Tenta ler e ja armazena os parametros de uma definição de função
    r = sscanf(line, "function f%d p%c%*d p%c%*d p%c%*d", &funcao.nome,
               &funcao.param[0], &funcao.param[1], &funcao.param[2]);
    // Verifica se foi possivel ler a declaração da função
    if (r >= 1) {
      continue;
    }

    /*
     * Inicio do bloco de definição de variaveis locais da função
     */
    if (strncmp(line, "def", 3) == 0) {
      continue;
    }

    /*
     * Fim do bloco de definição de variaveis locais da função
     */
    if (strncmp(line, "enddef", 6) == 0) {
      // Traduz o inicio da função até alocação de pilhas e registradores
      traduzInicioFuncao(funcao);

      continue;
    }

    /*
     * Retorno da função
     */
    r = sscanf(line, "return ci%d", &ret);
    if (r == 1) {
      traduzRetorno(ret);
      continue;
    }

    /*
     * Fim da definição da função
     */
    if (strncmp(line, "end", 3) == 0) {
      traduzFimFuncao();

      // Reinicializa a struct da função para ler uma possivel proxima função
      inicializaFuncao(&funcao);

      continue;
    }

    /*
     * Definição de variaveis locais
     */
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
