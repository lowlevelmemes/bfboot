/** BFBoot is a full brainfuck to bootable OS converter **/

/* it depends on nasm */

#include <stdio.h>
#include <stdlib.h>

#define MAX_STACK_SIZE  (65536 * 8)

FILE *src;
FILE *dest;

void transpile(int op, int count) {

    switch (op) {
        case '+':
            if (count > 1)
                fprintf(dest, "add byte [ebx], %d\n", count);
            else
                fprintf(dest, "inc byte [ebx]\n");
            break;
        case '-':
            if (count > 1)
                fprintf(dest, "sub byte [ebx], %d\n", count);
            else
                fprintf(dest, "dec byte [ebx]\n");
            break;
        case '<':
            if (count > 1)
                fprintf(dest, "sub ebx, %d\n", count);
            else
                fprintf(dest, "dec ebx\n");
            break;
        case '>':
            if (count > 1)
                fprintf(dest, "add ebx, %d\n", count);
            else
                fprintf(dest, "inc ebx\n");
            break;
        default:
            break;
    }

}

void usage(char *exec_name) {
    fprintf(stderr,
        "Usage: %s <source> <output>\n", exec_name
    );
    return;
}

int main(int argc, char **argv) {
    char tmpcmd[2048];

    if (argc < 2) {
        fprintf(stderr, "BFBoot: no source file.\n");
        usage(argv[0]);
        return EXIT_FAILURE;
    }
    if (argc < 3) {
        fprintf(stderr, "BFBoot: no output filename.\n");
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    if (!(src = fopen(argv[1], "r"))) {
        fprintf(stderr, "BFBoot: Failed to open %s.\n", argv[1]);
        return EXIT_FAILURE;
    }
    if (!(dest = fopen("bfboot.tmp", "w"))) {
        fprintf(stderr, "BFBoot: Failed to open bfboot.tmp.\n");
        fclose(src);
        return EXIT_FAILURE;
    }

    int cur_label = 0;
    int *label_stack = malloc(MAX_STACK_SIZE);
    if (!label_stack) {
        fprintf(stderr, "BFBoot: malloc fail.\n");
        fclose(src);
        fclose(dest);
        return EXIT_FAILURE;
    }
    int label_sp = 0;
    int last_operand = 0;
    int operand_count = 0;

    fprintf(dest,
        "org 0x100000\n"
        "bits 32\n"
    );

    /* get operands and repeat count */
    for (;;) {
        switch (fgetc(src)) {
            case EOF:
                fprintf(dest, "jmp $\n");
                fclose(src);
                fclose(dest);
                free(label_stack);
                if (system("nasm bfboot.tmp -f bin -o bfboot.tmp.bin")) {
                    fprintf(stderr, "BFBoot: nasm call error.\n");
                    return EXIT_FAILURE;
                }
                sprintf(tmpcmd, "nasm loader.asm -f bin -o %s", argv[2]);
                if (system(tmpcmd)) {
                    fprintf(stderr, "BFBoot: nasm call error.\n");
                    return EXIT_FAILURE;
                }
                return 0;
            case '+':
                if (last_operand != '+') {
                    transpile(last_operand, operand_count);
                    operand_count = 1;
                    last_operand = '+';
                } else {
                    operand_count++;
                }
                break;
            case '-':
                if (last_operand != '-') {
                    transpile(last_operand, operand_count);
                    operand_count = 1;
                    last_operand = '-';
                } else {
                    operand_count++;
                }
                break;
            case '>':
                if (last_operand != '>') {
                    transpile(last_operand, operand_count);
                    operand_count = 1;
                    last_operand = '>';
                } else {
                    operand_count++;
                }
                break;
            case '<':
                if (last_operand != '<') {
                    transpile(last_operand, operand_count);
                    operand_count = 1;
                    last_operand = '<';
                } else {
                    operand_count++;
                }
                break;
            case '.':
                transpile(last_operand, operand_count);
                last_operand = 0;
                fprintf(dest,
                    "mov al, byte [ebx]\n"
                    "call ecx\n"
                );
                break;
            case ',':
                transpile(last_operand, operand_count);
                last_operand = 0;
                fprintf(dest,
                    "call edx\n"
                    "mov byte [ebx], al\n"
                );
                break;
            case '[':
                transpile(last_operand, operand_count);
                last_operand = 0;
                fprintf(dest, "label%d:\n", cur_label);
                fprintf(dest,
                    "cmp byte [ebx], 0\n"
                    "je label%d_out\n", cur_label
                );
                label_stack[label_sp++] = cur_label++;
                break;
            case ']':
                transpile(last_operand, operand_count);
                last_operand = 0;
                fprintf(dest,
                    "cmp byte [ebx], 0\n"
                    "jne label%d\n", label_stack[label_sp-1]
                );
                fprintf(dest, "label%d_out:\n", label_stack[label_sp-1]);
                label_sp--;
                break;
            default:
                break;
        }

    }

}
