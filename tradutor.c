#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINESZ 256

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
  int count = 0;            // contagem de linhas lidas
  int r;                    // quantidade de valores lidos pelo sscanf
  int f;                    // nome (numero) da funcao atual
  char tipos_parametros[3]; // cada funcao pode ter no maximo 3 parametros
  int parametro_count = 0;  // quantidade de parametros
  int ret;                  // valor retornado pela funcao atual

  /* por enquanto estou salvando todas as variaveis locais num unico vetor
   * como as definições de variavel só define os tipos sem inicializar com
   * nenhum valor entao eu guardo apenas os tipos
   * se o valor for -2 é variavel inteira de pilha
   * se o valor for -1 é variavel inteira de registrador
   * se o valor for maior ou igual a 0 então é um vetor e o valor é o tamanho do
   * vetor
   */
  int vl[12];   // tipos das variaveis locais
  int vl_count; // contagem de variaveis locais

  // Lê uma linha por vez
  while (fgets(line, LINESZ, stdin) != NULL) {
    count++;
    remove_newline(line);
    printf("linha %d: %s\n", count, line);

    // Verifica se a linha começa com function e quantos parametros possui
    r = sscanf(line, "function f%d p%c%*d p%c%*d p%c%*d", &f,
               &tipos_parametros[0], &tipos_parametros[1],
               &tipos_parametros[2]); // Função com 3 parametros
    if (r == 4) {
      parametro_count = 3;
      printf("Definição da função f%d, com parametros p%c1, p%c2, p%c3\n", f,
             tipos_parametros[0], tipos_parametros[1], tipos_parametros[2]);
      printf("\n");
      continue;
    }
    r = sscanf(line, "function f%d p%c%*d p%c%*d", &f, &tipos_parametros[0],
               &tipos_parametros[1]); // Função com 2 parametros
    if (r == 3) {
      parametro_count = 2;
      printf("Definição da função f%d, com parametros p%c1, p%c2\n", f,
             tipos_parametros[0], tipos_parametros[1]);
      printf("\n");
      continue;
    }
    r = sscanf(line, "function f%d p%c%*d", &f,
               &tipos_parametros[0]); // Função com 1 parametro
    if (r == 2) {
      parametro_count = 1;
      printf("Definição da função f%d, com parametros p%c1\n", f,
             tipos_parametros[0]);
      printf("\n");
      continue;
    }
    r = sscanf(line, "function f%d", &f); // Função sem parametros
    if (r == 1) {
      parametro_count = 0;
      printf("Definição da função f%d, sem parametros\n", f);
      printf("\n");
      continue;
    }

    // Verifica se a linha começa com def
    if (strncmp(line, "def", 3) == 0) {
      printf("inicio do bloco de variaveis locais da função f%d\n", f);
      printf("\n");
      continue;
    }

    // Verifica se a linha começa com enddef
    if (strncmp(line, "enddef", 6) == 0) {
      printf(
          "fim do bloco de variaveis locais e inicio do corpo da função f%d\n",
          f);
      printf("\n");
      continue;
    }

    // Verifica se a linha começa return e qual valor retornado
    r = sscanf(line, "return ci%d", &ret);
    if (r == 1) {
      printf("a função f%d retornou o valor %d\n", f, ret);
      printf("\n");
      continue;
    }

    // Verifica se a linha começa com end
    if (strncmp(line, "end", 3) == 0) {
      printf("fim da função f%d\n", f);
      printf("\n----------------------------------------\n\n");
      // Imprime um sumario da funcao atual
      printf("sumario de f%d\n", f);
      // Imprime os tipos de parametros
      printf("%d parametros\n", parametro_count);
      for (int i = 0; i < parametro_count; i++) {
        if (tipos_parametros[i] == 'a') {
          printf("parametro va%d do tipo array\n", i + 1);
        } else if (tipos_parametros[i] == 'i') {
          printf("parametro vi%d do tipo inteiro\n", i + 1);
        }
      }

      // Imprime os tipos das variaveis locais
      printf("%d variaveis locais\n", vl_count);
      for (int i = 0; i < vl_count; i++) {
        if (vl[i] == -2) {
          printf("variavel local vi%d de pilha\n", i + 1);
        } else if (vl[i] == -1) {
          printf("variavel local ri%d de registrador\n", i + 1);
        } else if (vl[i] >= 0) {
          printf("variavel local va%d array de tamanho %d\n", i + 1, vl[i]);
        }
      }

      printf("\n----------------------------------------\n\n");
      continue;
    }

    // Verifica se a linha define uma variavel inteira de pilha
    r = sscanf(line, "var vi%d", &vl_count);
    if (r == 1) {
      vl[vl_count - 1] = -2;
      printf("lida uma variavel inteira de pilha vi%d\n", vl_count);
      printf("\n");
      continue;
    }
    // Verifica se a linha define uma variavel inteira de registrador
    r = sscanf(line, "reg ri%d", &vl_count);
    if (r == 1) {
      vl[vl_count - 1] = -1;
      printf("lida uma variavel inteira de registrador ri%d\n", vl_count);
      printf("\n");
      continue;
    }
    // Verifica se a linha define um array e qual o seu tamanho
    unsigned int tamanho_vetor;
    r = sscanf(line, "vet va%d size ci%d", &vl_count, &tamanho_vetor);
    if (r == 2) {
      vl[vl_count - 1] = tamanho_vetor;
      printf("lida uma variavel vetor va%d de tamanho %d\n", vl_count,
             vl[vl_count - 1]);
      printf("\n");
      continue;
    }
  }

  return 0;
}
