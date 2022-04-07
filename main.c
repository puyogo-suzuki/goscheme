#include "token.h"
#include <stdio.h>
int main(void) {
    string_t str;
    string_new_deep2(&str, "(hoge piyo 3283 \"gehiog\" (a b))");
    tokenizer_t t = tokenizer_new(str);
    token_t ot;
    while(tokenizer_next(&t, &ot)) {
        printf("%d ", ot.tokenKind);
        if (ot.value.strValue.buffer == NULL) {
            putchar('\n');
            continue;
        }
        for(int i = 0; i < ot.value.strValue.length; ++i) {
            putchar(ot.value.strValue.buffer[i]);
        }
        putchar('\n');
    }
    printf("end\n");
    return 0;
}