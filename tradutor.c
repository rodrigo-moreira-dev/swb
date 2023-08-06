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
 * tipo_item:
 *  'p' - parametro
 *  'v' - variavel local
 *
 * posicao:
 *  a posição da variavel local ou do parametro por exemplo
 *  1 para pi1 ou vi1
 *  2 para pi2 ou vi2
 */
int getOffset(Func f, char tipo_item, int posicao) {
  int offset = 0;

  /*
   * Conta a quantidade de variaveis locais e os tipos de cada uma delas
   * Para calcular a quantidade de bytes necessaria para o offset
   *
   * Caso a variavel local seja um array, será levado em conta o seu tamanho
   */
  int i = 0;
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

    // retorna o offset atual pois é o do item desejado
    if (tipo_item == 'v' && posicao == (i + 1)) {
      return offset;
    }

    i++;
  }

  /*
   * Conta a quantidade de parametros e os tipos de cada um deles
   * Para calcular a quantidade de bytes necessaria para o offset
   */
  i = 0;
  while (i < MAX_PARAM) {
    char tipo_parametro = f.param[i];

    if (tipo_parametro == '\0') { // nao tem mais parametros
      break;
    } else if (tipo_parametro == 'i') { // parametro inteiro
      offset = (offset + 4); // soma 4 no offset pois é o tamanho do inteiro
    } else if (tipo_parametro == 'a') { // parametro array
      // Verifica e garante que o offset de um ponteiro (tamanho 8) seja
      // multiplo de 8
      if (offset != 0 && offset % 8 != 0) {
        offset += 8 - (offset % 8);
      }

      offset = (offset + 8); // soma 8 pois é o tamanho de um ponteiro
    }

    // retorna o offset atual pois é o da variavel desejada
    if (tipo_item == 'p' && posicao == (i + 1)) {
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
   * Imprime os offsets das variaveis locais
   */
  int i = 0;
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

  /*
   * Imprime os offsets dos parametros
   */
  i = 0;
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

  if (bytesPilha != 0) {
    // Verifica e garante que o espaço alocado para a pilha seja multiplo de 16
    if (bytesPilha % 16 != 0) {
      bytesPilha += 16 - (bytesPilha % 16);
    }

    printf("subq $%d, %%rsp\n\n", bytesPilha);
  }
}

/*
 * Retorna o registrador em que um determinado item está armazenado
 *
 * tipo_item:
 *  'p' - parametro
 *  'v' - variavel local
 *
 * tipo_valor:
 *  'i' - inteiro
 *  'a' - array
 *
 * Como as variaveis locais de registrador são armazenadas nos registradores
 * r8 a r11 esta função retorna apenas o numero do registrador
 */
const char *getRegistrador(Func f, char tipo_item, char tipo_valor,
                           int posicao) {
  const char *registradores[] = {"%rdi", "%rsi", "%rdx", "%edi",  "%esi",
                                 "%edx", "%r8d", "%r9d", "%r10d", "%r11d"};
  if (tipo_item == 'p') {
    if (tipo_valor == 'i') {
      return registradores[posicao - 1 +
                           3]; // Pula os registradores %rdi até %rsi
    }
    return registradores[posicao - 1];
  }

  int i = 0;
  int regCount = 0; // Contagem de variaveis de registrador
  while (i < MAX_VARS) {
    int var = f.var_loc[i];
    if (var == -1) { // nao tem mais variaveis locais
      break;
    } else if (var == 0) { // variavel de registrador
      if (i + 1 == posicao) {
        return registradores[regCount + 6];
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
  int countRegistradores = 0; // Conta a quantidade de variaveis de registrador
  while (i < MAX_VARS) {
    int var = f.var_loc[i];

    if (var == -1) { // nao tem mais variaveis locais
      if (countRegistradores > 0) {
        printf("\n");
      }

      break;
    } else if (var == 0) { // variavel de registrador
      countRegistradores++;
      if (countRegistradores == 1) {
        printf("# Variaveis locais de registrador\n");
      }

      printf("# vr%d: %s\n", i + 1, getRegistrador(f, 'v', 'i', i + 1));
    }

    i++;
  }
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
void traduzRetorno(Func f, int ret) {
  // TODO
  // Traduz apenas retornos de constantes inteiras por enquanto
  printf("movl $%d, %%eax\n", ret);
  printf("jmp fim_f%d\n\n", f.nome);
}

void traduzFimFuncao(Func f) {
  printf("fim_f%d:\n", f.nome);
  printf("leave\n");
  printf("ret\n\n");
}

/*
 * Traduz uma atribuição simples de uma variavel local ou constante ou
 * parametro inteiro para uma variavel local de registrador ou de pilha
 *
 * tipo_valor_variavel:
 *  'i' -> variavel de pilha
 *  'r' -> variavel de registrador
 *
 * tipo_item_operando:
 *  'c' -> constante
 *  'p' -> parametro da função
 *  'v' -> variavel local da função
 *
 *  tipo_valor_operando:
 *   'i' -> inteiro
 *   'a' -> array
 */
void traduzAtribuicaoSimples(Func f, char tipo_valor_variavel,
                             int posicao_variavel, char tipo_item_operando,
                             char tipo_valor_operando, int posicao_operando) {

  char source[16];
  char dest[16];

  /*
   * Determina a string para o destino da atribuição
   *
   * se for de pilha será %d(%rbp) sendo que %d será o offset que será
   * calculado
   *
   * se for de registrador será %r%d sendo que %d será o numero do registrador
   * que será calculado
   */
  if (tipo_valor_variavel == 'i') { // variavel de pilha
    snprintf(dest, sizeof(dest), "-%d(%%rbp)",
             getOffset(f, 'v', posicao_variavel));
  } else if (tipo_valor_variavel == 'r') { // variavel de registrador
    snprintf(dest, sizeof(dest), "%s",
             getRegistrador(f, 'v', tipo_valor_variavel, posicao_variavel));
  }

  /*
   * Determina a string para a fonte da atribuição
   *
   * se for variavel local de registrador será %d(%rbp) sendo que %d será o
   * offset que será calculado
   *
   * se for variavel local de registrador será %r%d sendo que %d será o numero
   * do registrador que será calculado
   *
   * se for parametro sera um dos registradores de parametro
   *
   * se for constante será $%d sendo %d a propria constante
   */
  if (tipo_item_operando == 'c') {
    snprintf(source, sizeof(source), "$%d", posicao_operando);
  } else if (tipo_item_operando == 'p' ||  // operando é parametro
             tipo_valor_operando == 'r') { // ou variavel local de registrador
    snprintf(source, sizeof(source), "%s",
             getRegistrador(f, tipo_item_operando, tipo_valor_operando,
                            posicao_operando));
  } else if (tipo_valor_operando == 'i') { // operando é variavel local de pilha
    snprintf(source, sizeof(source), "-%d(%%rbp)",
             getOffset(f, 'v', posicao_operando));
  }

  printf("movl %s, %s\n\n", source, dest);
}

/*
 * Traduz uma atribuição com operação
 *
 * tipo_valor_variavel:
 *  'i' -> variavel de pilha
 *  'r' -> variavel de registrador
 *
 * tipo_item_operando:
 *  'c' -> constante
 *  'p' -> parametro da função
 *  'v' -> variavel local da função
 *
 *  tipo_valor_operando:
 *   'i' -> inteiro
 *   'a' -> array
 */
void traduzAtribuicaoOperacao(Func f, char tipo_valor_variavel,
                              int posicao_variavel, char tipo_item_operando1,
                              char tipo_valor_operando1, int posicao_operando1,
                              char operacao, char tipo_item_operando2,
                              char tipo_valor_operando2,
                              int posicao_operando2) {
  const char *operacoes[] = {"addl", "subl", "imull", "idivl"};

  // Primeiro move o primeiro operando para %eax onde será armazenado o
  // resultado final da operação
  if (tipo_item_operando1 == 'c') { // operando é constante
    printf("movl $%d, %%eax\n", posicao_operando1);
  } else if (tipo_item_operando1 == 'p') { // operando é parametro
    printf("movl %s, %%eax\n",
           getRegistrador(f, tipo_item_operando1, tipo_valor_operando1,
                          posicao_operando1));
  } else if (tipo_valor_operando1 ==
             'r') { // operando é variavel local de registrador
    printf("movl %s, %%eax\n",
           getRegistrador(f, tipo_item_operando1, tipo_valor_operando1,
                          posicao_operando1));
  } else if (tipo_valor_operando1 ==
             'i') { // operando é variavel local de pilha
    printf("movl -%d(%%rbp), %%eax\n", getOffset(f, 'v', posicao_operando1));
  }

  // Define qual operacao será traduzida com base no caracter da operacao
  int i;
  switch (operacao) {
  case '+':
    i = 0;
    break;
  case '-':
    i = 1;
    break;
  case '*':
    i = 2;
    break;
  case '/':
    i = 3;
    break;
  }

  // Traduz a operação
  // Como a tradução da divisão é levemente diferente ela é feita separada
  if (tipo_item_operando2 == 'c') { // operando é constante
    if (operacao == '/') {
      printf("cdq\n"); // operacoes necessarias para se usar o idiv
      printf("movl $%d, %%ebx\n", posicao_operando2);
      printf("%s %%ebx\n", operacoes[i]);
    } else {
      printf("%s $%d, %%eax\n", operacoes[i], posicao_operando2);
    }
  } else if (tipo_item_operando2 == 'p' ||  // operando é parametro
             tipo_valor_operando2 == 'r') { // ou variavel local de registrador
    if (operacao == '/') {
      printf("cdq\n"); // operacoes necessarias para se usar o idiv
      printf("%s %s\n", operacoes[i],
             getRegistrador(f, tipo_item_operando2, tipo_valor_operando2,
                            posicao_operando2));
    } else {
      printf("%s %s, %%eax\n", operacoes[i],
             getRegistrador(f, tipo_item_operando2, tipo_valor_operando2,
                            posicao_operando2));
    }
  } else if (tipo_valor_operando2 ==
             'i') { // operando é variavel local de pilha
    if (operacao == '/') {
      printf("cdq\n"); // operacoes necessarias para se usar o idiv
      printf("%s -%d(%%rbp)\n", operacoes[i],
             getOffset(f, 'v', posicao_operando2));
    } else {
      printf("%s -%d(%%rbp), %%eax\n", operacoes[i],
             getOffset(f, 'v', posicao_operando2));
    }
  }

  // Move o o resultado da operação para a pilha ou o registrador em questão
  if (tipo_valor_variavel == 'i') { // variavel de pilha
    printf("movl %%eax, -%d(%%rbp)\n\n", getOffset(f, 'v', posicao_variavel));
  } else if (tipo_valor_variavel == 'r') { // variavel de registrador
    printf("movl %%eax, %s\n\n",
           getRegistrador(f, 'v', tipo_valor_variavel, posicao_variavel));
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
      traduzRetorno(funcao, ret);
      continue;
    }

    /*
     * Fim da definição da função
     */
    if (strncmp(line, "end", 3) == 0) {
      traduzFimFuncao(funcao);

      // Reinicializa a struct da função para ler uma possivel proxima função
      inicializaFuncao(&funcao);

      continue;
    }

    /*
     * Definição de variaveis locais
     */
    // Verifica se a linha define uma variavel inteira de registrador
    r = sscanf(line, "reg vr%d", &var_loc_count);
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

    /*
     * Operações de atribuição
     */
    char tipo_valor_variavel;
    int posicao_variavel;
    char tipo_item_operando1;
    char tipo_valor_operando1;
    int posicao_operando1;
    char operacao;
    char tipo_item_operando2;
    char tipo_valor_operando2;
    int posicao_operando2;
    r = sscanf(line, "v%c%d = %c%c%d %c %c%c%d", &tipo_valor_variavel,
               &posicao_variavel, &tipo_item_operando1, &tipo_valor_operando1,
               &posicao_operando1, &operacao, &tipo_item_operando2,
               &tipo_valor_operando2, &posicao_operando2);

    // Atribuição Simples
    if (r == 5) {
      printf("# %s\n", line);
      traduzAtribuicaoSimples(funcao, tipo_valor_variavel, posicao_variavel,
                              tipo_item_operando1, tipo_valor_operando1,
                              posicao_operando1);
      continue;
    }

    // Atribuição com operação
    if (r == 9) {
      printf("# %s\n", line);
      traduzAtribuicaoOperacao(funcao, tipo_valor_variavel, posicao_variavel,
                               tipo_item_operando1, tipo_valor_operando1,
                               posicao_operando1, operacao, tipo_item_operando2,
                               tipo_valor_operando2, posicao_operando2);
      continue;
    }
  }

  return 0;
}
