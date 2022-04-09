#include <stdio.h>
#include "token.h"
#include "io.h"

int main(void) {
    string_t str;
    string_new_deep2(&str, "(hoge piyo 3283 \"  gehiog\" (a b))");
    tokenizer_t t = tokenizer_new(str);
    token_t ot;
    while(tokenizer_next(&t, &ot)) {
        string_t s;
        token_toString(&ot, &s);
        string_writeLine(stdout, &s);
    }
    printf("end\n");
    return 0;
}