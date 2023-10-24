#include <stdio.h>
#include <stdlib.h>

#include "utils.h"
#include "lexer.h"
#include "sintaxer.h"

int main() {
    char *buff = loadFile("q-src/def-data.q");
    tokenize(buff);
    showTokens();
    
    parse();

    return 0;
}