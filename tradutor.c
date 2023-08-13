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
 * Os offsets para as variaveis locais de registradores são utilizados para
 * armazenar os valores previamente disponiveis nesses registradores ao entrar
 * numa função
 *
 * Os offsets para os parametros são utilizados para armazenar os valores nos
 * registradores %rdi, %rsi, %rdx conforme seja necessario chamar outras funções
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
    } else if (tipo_var_loc == 1 || // variavel local de pilha
               tipo_var_loc == 0) { // ou variavel local de registrador
      offset = offset + 4;          // soma 4 pois é o tamanho de uma variavel
                                    // local (todas são inteiro)
    } else if (tipo_var_loc > 1) {  // variavel local array
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
    } else if (tipo_var_loc == 0) { // variavel local de registrador
      bytesPilha = getOffset(f, 'v', (i + 1));

      // imprime o offset em que o valor previo do registrador será armazenado
      printf("# vr%d: -%d(%%rbp)\n", i + 1, bytesPilha);
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
 * Toda função será tratada como função interna:
 *  - Os valores temporarios serão armazenados em registradores caller-saved
 * (%rdi, %rsi, %rdx, %rcx, %r8, %r9, %r10, %11, %rax)
 *  - Os valores importantes (variaveis locais) são armazenadas em registradores
 * callee-saved (%rbx, %r12, %r13, %r14, %r15)
 *
 * Como as funções tem no maximo três parametros apenas os parametros %rdi, %rsi
 * e %rdx (e suas variações de 4 bytes) serão usados para parametros
 *
 * Como as funções tem no maximo quatro variaveis inteiras de registrador os
 * registradores %r12d, %r13d, %r14d, %r15d serão usados para essas variaveis
 * locais
 */
const char *getRegistrador(Func f, char tipo_item, char tipo_valor,
                           int posicao) {
  const char *registradores[] = {"%rdi", "%rsi",  "%rdx",  "%edi",  "%esi",
                                 "%edx", "%r12d", "%r13d", "%r14d", "%r15d"};
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
 * Traduz o codigo para salvar um item qualquer na pilha
 *
 * Pode ser usado para salvar os registradores das variaveis locais de
 * registrador %r12d a %r15d (callee-saved) ao entrar numa função ou para salvar
 * os parametros %rdi, %rsi, %rdx (caller-saved) antes de realizar a chamada de
 * uma função
 *
 * tipo_item
 *  - 'p' -> parametro
 *  - 'v' -> variavel local
 *
 *  tipo_valor
 *   - 'a' -> vetor
 *   - 'i' -> inteiro (se for variavel local será um inteiro de registrador)
 *   - 'r' -> variavel local de registrador
 *
 *   operacao
 *    - 'w' -> salvar (write) algum valor na pilha
 *    - 'r' -> resgatar (read) algum valor na pilha
 */
void traduzSalvamentoItem(Func f, char tipo_item, char tipo_valor, int posicao,
                          char operacao) {
  char registrador[16];

  // Traduz escrita ou leitura de parametros para/da a pilha
  if (tipo_item == 'p') {
    // armazena o registrador desse parametro numa string
    snprintf(registrador, sizeof(registrador), "%s",
             getRegistrador(f, tipo_item, tipo_valor, posicao));

    if (tipo_valor == 'a') {
      if (operacao == 'w') {
        printf("movq %s, -%d(%%rbp) # guarda o valor do parametro %c%c%d\n\n",
               registrador, getOffset(f, tipo_item, posicao), tipo_item,
               tipo_valor, posicao);
      } else if (operacao == 'r') {
        printf("movq -%d(%%rbp), %s # resgata o valor do parametro %c%c%d\n\n",
               getOffset(f, tipo_item, posicao), registrador, tipo_item,
               tipo_valor, posicao);
      }
    } else if (tipo_valor == 'i') {
      if (operacao == 'w') {
        printf("movl %s, -%d(%%rbp) # guarda o valor do parametro %c%c%d\n\n",
               registrador, getOffset(f, tipo_item, posicao), tipo_item,
               tipo_valor, posicao);
      } else if (operacao == 'r') {
        printf("movl -%d(%%rbp), %s # resgate o valor do parametro %c%c%d\n\n",
               getOffset(f, tipo_item, posicao), registrador, tipo_item,
               tipo_valor, posicao);
      }
    }
  }

  // Traduz escrita ou leitura de variaveis inteiras de registrador para/da a
  // pilha
  if (tipo_item == 'v') {
    snprintf(registrador, sizeof(registrador), "%s",
             getRegistrador(f, tipo_item, tipo_valor, posicao));

    if (tipo_valor == 'r') {
      if (operacao == 'w') {
        printf("movl %s, -%d(%%rbp) # guarda o valor previo do registrador \n",
               registrador, getOffset(f, tipo_item, posicao));
      } else if (operacao == 'r') {
        printf("movl -%d(%%rbp), %s # resgata o valor previo do registrador \n",
               getOffset(f, tipo_item, posicao), registrador);
      }
    }
  }
}

/*
 * "Aloca" os registradores para as variaveis locais de registrador
 *
 * Como essas variaveis são armazenadas em registradores callee-saved, essa
 * função ira armazenar os valores previamente contidos nesses registradores na
 * pilha de função, para que sejam resgatados antes da função retornar
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
        printf("# Guarda os valores previos dos registradores\n");
      }

      printf("# vr%d: %s\n", i + 1, getRegistrador(f, 'v', 'r', i + 1));
      traduzSalvamentoItem(f, 'v', 'r', i + 1, 'w');
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
void traduzRetorno(Func f, char tipo_item_retorno, char tipo_valor_retorno,
                   int posicao_retorno) {
  if (tipo_item_retorno == 'c') { // retorno é uma constante
    printf("movl $%d, %%eax\n", posicao_retorno);
  } else if (tipo_item_retorno == 'p' ||  // retorno é parametro
             tipo_valor_retorno == 'r') { // ou variavel local de registrador
    printf("movl %s, %%eax\n",
           getRegistrador(f, tipo_item_retorno, tipo_valor_retorno,
                          posicao_retorno));
  } else if (tipo_valor_retorno == 'i') { // retorno é variavel local de pilha
    printf("movl -%d(%%rbp), %%eax\n", getOffset(f, 'v', posicao_retorno));
  }
  printf("jmp fim_f%d\n\n", f.nome);
}

/*
 * Restaura os valores previos dos registradores callee-saved (%r12 a %r15) que
 * foram usados para as variaveis locais dessa função
 */
void desalocaRegistradores(Func f) {
  int i = 0;
  int countRegistradores = 0; // Conta a quantidade de variaveis de registrador
  while (i < MAX_VARS) {
    int tipo_valor = f.var_loc[i];

    if (tipo_valor == -1) { // nao tem mais variaveis locais
      if (countRegistradores > 0) {
        printf("\n");
      }

      break;
    } else if (tipo_valor == 0) { // variavel de registrador
      countRegistradores++;
      if (countRegistradores == 1) {
        printf(
            "# Restaura os valores previos dos registradores callee-saved\n");
      }

      traduzSalvamentoItem(f, 'v', 'r', i + 1, 'r');
    }

    i++;
  }
}

void traduzFimFuncao(Func f) {
  desalocaRegistradores(f);
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
  int if_count = 0;   // contagem de quantos ifs tem na funcao

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
      printf("# %s\n", line);
      // Imprime comentarios com os registradores de cada parametro
      int i = 0;
      while (i < MAX_PARAM) {
        char tipo_valor_parametro = funcao.param[i];
        if (tipo_valor_parametro == '\0') {
          break;
        } else if (tipo_valor_parametro != '\0') {
          printf("# p%c%d: %s\n", funcao.param[i], i + 1,
                 getRegistrador(funcao, 'p', tipo_valor_parametro, i + 1));
        }
        i++;
      }
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
    char tipo_item_retorno;
    char tipo_valor_retorno;
    int posicao_retorno;
    r = sscanf(line, "return %c%c%d", &tipo_item_retorno, &tipo_valor_retorno,
               &posicao_retorno);
    if (r == 3) {
      printf("# %s\n", line);
      traduzRetorno(funcao, tipo_item_retorno, tipo_valor_retorno,
                    posicao_retorno);
      continue;
    }

    /*
     * Fim da definição da função
     */
    if (strcmp(line, "end") == 0) {
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

    /*
     * Atribuição com chamada de função
     */
    int numero_funcao;
    char tipo_item_operando3;
    char tipo_valor_operando3;
    int posicao_operando3;
    r = sscanf(line, "v%c%d = call f%d %c%c%d %c%c%d %c%c%d",
               &tipo_valor_variavel, &posicao_variavel, &numero_funcao,
               &tipo_item_operando1, &tipo_valor_operando1, &posicao_operando1,
               &tipo_item_operando2, &tipo_valor_operando2, &posicao_operando2,
               &tipo_item_operando3, &tipo_valor_operando3, &posicao_operando3);

    // função com 1 ou mais parametros
    if (r >= 3) {
      int parametros;
      if (r == 3) { // nenhum parametro
        parametros = 0;
      } else if (r == 6) { // 1 parametro
        parametros = 1;
      } else if (r == 9) { // 2 parametros
        parametros = 2;
      } else if (r == 12) { // 3 parametros
        parametros = 3;
      }
      printf("# %s\n", line);

      // Guarda o valor dos registradores de parametro na pilha
      for (int i = 0; i < parametros; i++) {
        char tipo_valor = funcao.param[i];
        traduzSalvamentoItem(funcao, 'p', tipo_valor, i + 1, 'w');
      }

      // Move os valores necessarios para os registradores de parametros antes
      // de chamara função

      // Move o primeiro parametro
      if (parametros >= 1) {
        if (tipo_item_operando1 == 'p') {
          if (tipo_valor_operando1 == 'i') { // pi - parametro inteiro
            printf("movl -%d(%%rbp), %%edi\n",
                   getOffset(funcao, tipo_item_operando1, posicao_operando1));
          } else if (tipo_valor_operando1 == 'a') { // pa - parametro array
            printf("movq -%d(%%rbp), %%rdi\n",
                   getOffset(funcao, tipo_item_operando1, posicao_operando1));
          }
        } else if (tipo_item_operando1 == 'v') {
          if (tipo_valor_operando1 == 'i') { // vi - variavel inteira de pilha
            printf("movl -%d(%%rbp), %%edi\n",
                   getOffset(funcao, tipo_item_operando1, posicao_operando1));
          } else if (tipo_valor_operando1 ==
                     'a') { // va - variavel array de pilha
            printf("leaq -%d(%%rbp), %%rdi\n",
                   getOffset(funcao, tipo_item_operando1, posicao_operando1));
          } else if (tipo_valor_operando1 ==
                     'r') { // vr - variavel inteira de registrador
            printf("movl %s, %%edi\n",
                   getRegistrador(funcao, tipo_item_operando1,
                                  tipo_valor_operando1, posicao_operando1));
          }
        } else if (tipo_item_operando1 == 'c') { // ci - constante inteirad
          printf("movl $%d, %%edi\n", posicao_operando1);
        }
      }

      // Move o segundo parametro
      if (parametros >= 2) {
        if (tipo_item_operando2 == 'p') {
          if (tipo_valor_operando2 == 'i') { // pi - parametro inteiro
            printf("movl -%d(%%rbp), %%esi\n",
                   getOffset(funcao, tipo_item_operando2, posicao_operando2));
          } else if (tipo_valor_operando2 == 'a') { // pa - parametro array
            printf("movq -%d(%%rbp), %%rsi\n",
                   getOffset(funcao, tipo_item_operando2, posicao_operando2));
          }
        } else if (tipo_item_operando2 == 'v') {
          if (tipo_valor_operando2 == 'i') { // vi - variavel inteira de pilha
            printf("movl -%d(%%rbp), %%esi\n",
                   getOffset(funcao, tipo_item_operando2, posicao_operando2));
          } else if (tipo_valor_operando2 ==
                     'a') { // va - variavel array de pilha
            printf("leaq -%d(%%rbp), %%rsi\n",
                   getOffset(funcao, tipo_item_operando2, posicao_operando2));
          } else if (tipo_valor_operando2 ==
                     'r') { // vr - variavel inteira de registrador
            printf("movl %s, %%esi\n",
                   getRegistrador(funcao, tipo_item_operando2,
                                  tipo_valor_operando2, posicao_operando2));
          }
        } else if (tipo_item_operando2 == 'c') { // ci - constante inteirad
          printf("movl $%d, %%esi\n", posicao_operando2);
        }
      }

      // Move o terceiro parametro
      if (parametros >= 3) {
        if (tipo_item_operando3 == 'p') {
          if (tipo_valor_operando3 == 'i') { // pi - parametro inteiro
            printf("movl -%d(%%rbp), %%edx\n",
                   getOffset(funcao, tipo_item_operando3, posicao_operando3));
          } else if (tipo_valor_operando3 == 'a') { // pa - parametro array
            printf("movq -%d(%%rbp), %%rdx\n",
                   getOffset(funcao, tipo_item_operando3, posicao_operando3));
          }
        } else if (tipo_item_operando3 == 'v') {
          if (tipo_valor_operando3 == 'i') { // vi - variavel inteira de pilha
            printf("movl -%d(%%rbp), %%edx\n",
                   getOffset(funcao, tipo_item_operando3, posicao_operando3));
          } else if (tipo_valor_operando3 ==
                     'a') { // va - variavel array de pilha
            printf("leaq -%d(%%rbp), %%rdx\n",
                   getOffset(funcao, tipo_item_operando3, posicao_operando3));
          } else if (tipo_valor_operando3 ==
                     'r') { // vr - variavel inteira de registrador
            printf("movl %s, %%edx\n",
                   getRegistrador(funcao, tipo_item_operando3,
                                  tipo_valor_operando3, posicao_operando3));
          }
        } else if (tipo_item_operando3 == 'c') { // ci - constante inteirad
          printf("movl $%d, %%edx\n", posicao_operando3);
        }
      }

      printf("call f%d\n", numero_funcao);
      if (tipo_valor_variavel == 'i') {
        printf("movl %%eax, -%d(%%rbp)\n\n",
               getOffset(funcao, 'v', posicao_variavel));
      } else if (tipo_valor_variavel == 'r') {
        printf(
            "movl %%eax, %s\n\n",
            getRegistrador(funcao, 'v', tipo_valor_variavel, posicao_variavel));
      }

      // Resgata os valores dos registradores de parametro na pilha
      for (int i = 0; i < parametros; i++) {
        char tipo_valor = funcao.param[i];
        traduzSalvamentoItem(funcao, 'p', tipo_valor, i + 1, 'r');
      }
    }

    /*
     * Condicionais
     */

    char condicao[2];
    char var1[16];
    char var2[16];

    /*
     * Verifica se é um IF
     */

    r = sscanf(line, "if %c%c%d %c%c %c%c%d", &tipo_item_operando1,
               &tipo_valor_operando1, &posicao_operando1, &condicao[0],
               &condicao[1], &tipo_item_operando2, &tipo_valor_operando2,
               &posicao_operando2);
    if (r == 8) {
      if_count++;
      printf("# _%s\n", line);
      // Verifica se o primeiro dado é uma a variável
      if (tipo_item_operando1 == 'v') {
        // Verifica se é uma variável inteira
        if (tipo_valor_operando1 == 'i') {
          snprintf(var1, sizeof(var1), "-%d(%%rbp)",
                   getOffset(funcao, tipo_item_operando1, posicao_operando1));
        }
        // Verifica se é uma variável de registrador
        else if (tipo_valor_operando1 == 'r') {
          snprintf(var1, sizeof(var1), "%s",
                   getRegistrador(funcao, tipo_item_operando1,
                                  tipo_valor_operando1, posicao_operando1));
        }
      }

      // Verifica se o primeiro dado é um parâmetro
      if (tipo_item_operando1 == 'p') {
        printf("# var = %c%c%d\n", tipo_item_operando1, tipo_valor_operando1,
               posicao_operando1);
        snprintf(var1, sizeof(var1), "%s",
                 getRegistrador(funcao, tipo_item_operando1,
                                tipo_valor_operando1, posicao_operando1));
      }

      // Verifica se o primeiro dado é uma constante
      if (tipo_item_operando1 == 'c') {
        snprintf(var1, sizeof(var1), "$%d", posicao_operando1);
      }

      // Verifica se o segundo dado é uma a variável
      if (tipo_item_operando2 == 'v') {
        // Verifica se é uma variável inteira
        if (tipo_valor_operando2 == 'i') {
          snprintf(var2, sizeof(var2), "-%d(%%rbp)",
                   getOffset(funcao, tipo_item_operando2, posicao_operando2));
        }
        // Verifica se é uma variável de registrador
        else if (tipo_valor_operando2 == 'r') {
          snprintf(var2, sizeof(var2), "%s",
                   getRegistrador(funcao, tipo_item_operando2,
                                  tipo_valor_operando2, posicao_operando2));
        }
      }

      // Verifica se o segundo dado é um parâmetro
      if (tipo_item_operando2 == 'p') {
        snprintf(var2, sizeof(var2), "%s",
                 getRegistrador(funcao, tipo_item_operando2,
                                tipo_valor_operando2, posicao_operando2));
      }

      // Verifica se o segundo dado é uma constante
      if (tipo_item_operando2 == 'c') {
        snprintf(var2, sizeof(var2), "$%d", posicao_operando2);
      }

      printf("cmpl %s, %s \n", var2, var1);

      switch (condicao[0]) {
      // eq
      case 'e':
        printf("jne end_if%d\n\n", if_count);
        break;

      // ne
      case 'n':
        printf("je end_if%d\n\n", if_count);
        break;

      // lt ou le
      case 'l':
        if (condicao[1] == 't') { // lt
          printf("jge end_if%d\n\n", if_count);
        } else if (condicao[1] == 'e') { // le
          printf("jg end_if%d\n\n", if_count);
        }
        break;

        // gt ou ge
      case 'g':
        if (condicao[1] == 't') { // gt
          printf("jle end_if%d\n\n", if_count);
        } else if (condicao[1] == 'e') { // ge
          printf("jl end_if%d\n\n", if_count);
        }
        break;

      default:
        perror("Condição inválida\n\n");
        break;
      }
    }

    if (strncmp(line, "endif", 5) == 0) {
      printf("end_if%d:\n\n", if_count);
      continue;
    }

    if (strncmp(line, "set", 3) == 0 || strncmp(line, "get", 3) == 0) {
      printf("# %s\n", line);
      printf("# TODO: Implementação do set e do get\n\n");
    }
  }

  return 0;
}
