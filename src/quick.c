#include <stdio.h>
#include <stdlib.h>

#include "utils.h"
#include "lexer.h"
#include "sintaxer.h"

int main() {
    char *buff = loadFile("q-src/1.q");
    tokenize(buff);
    showTokens();
    
    parse();

    return 0;
}