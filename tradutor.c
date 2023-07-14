#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void processFunctionDeclaration() {
    printf("Função declarada\n");
}

void processDefinition() {
    printf("Definição encontrada\n");
}

void processEndDefinition() {
    printf("Fim da definição\n");
}

void processReturn() {
    printf("Instrução de retorno\n");
}

void processInstructionCi0() {
    printf("Instrução Ci0\n");
}

int main() {
    char line[1000];

    // Ler o arquivo linha por linha usando scanf
    while (scanf("%[^\n]%*c", line) != EOF) {
        // Analisar a linha e executar as ações correspondentes
        if (strcmp(line, "function f1") == 0) {
            processFunctionDeclaration();
        } else if (strcmp(line, "def") == 0) {
            processDefinition();
        } else if (strcmp(line, "enddef") == 0) {
            processEndDefinition();
        } else if (strcmp(line, "return") == 0) {
            processReturn();
        } else if (strcmp(line, "ci0") == 0) {
            processInstructionCi0();
        }
    }

    return 0;
}