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
  int count = 0;
  int r;
  int f;
  char tipos_parametros[3];
  int ret;


  // Lê uma linha por vez
  while (fgets(line, LINESZ, stdin) != NULL) {
    count++;
    remove_newline(line);

    // Verifica se a linha começa com function e quantos parametros possui
    r = sscanf(line, "function f%d p%c%*d p%c%*d p%c%*d", &f, &tipos_parametros[0], &tipos_parametros[1], &tipos_parametros[2]); // Função com 3 parametros
    if (r == 4) {
      printf("Definição da função f%d, com parametros p%c1, p%c2, p%c3\n", f, tipos_parametros[0], tipos_parametros[1], tipos_parametros[2]);
      continue;
    }
    r = sscanf(line, "function f%d p%c%*d p%c%*d", &f, &tipos_parametros[0], &tipos_parametros[1]); // Função com 2 parametros
    if (r == 3) {
      printf("Definição da função f%d, com parametros p%c1, p%c2\n", f, tipos_parametros[0], tipos_parametros[1]);
      continue;
    }
    r = sscanf(line, "function f%d p%c%*d", &f, &tipos_parametros[0]); // Função com 1 parametro
    if (r == 2) {
      printf("Definição da função f%d, com parametros p%c1\n", f, tipos_parametros[0]);
      continue;
    }
    r = sscanf(line, "function f%d", &f); // Função sem parametros
    if (r == 1) {
      printf("Definição da função f%d, sem parametros\n", f);
      continue;
    }

    // Verifica se a linha começa com def
    if (strncmp(line, "def", 3) == 0) {
      printf("inicio do bloco de variaveis locais da função f%d\n", f);
      continue;
    }

    // Verifica se a linha começa com enddef
    if (strncmp(line, "enddef", 6) == 0) {
      printf("fim do bloco de variaveis locais e inicio do corpo da função f%d\n", f);
      continue;
    }

    // Verifica se a linha começa return e qual valor retornado
    r = sscanf(line, "return ci%d", &ret);
    if(r == 1) {
      printf("a função f%d retornou o valor %d\n", f, ret);
    }

    // Verifica se a linha começa com end
    if (strncmp(line, "end", 3) == 0) {
      printf("fim da função f%d\n", f);
      printf("\n----------------------------------------\n\n");
    }
  }

  return 0;
}
