#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INIT_ADDRESS 16 // the minimum address of variable
#define LABLE 0         // type of variable
#define SYMBOL 1
#define MAX_LENGTH_OF_A_INSTRUCTION 64 + 1 // We need an extra char to save '\0', so we plus 1.
#define MAX_LENGTH_OF_C_INSTRUCTION 11 + 1 // without blank, the maximum length is 3 + 1 + 3 + 1 + 3 = 11
#define MAX_LENGTH_OF_FILE_NAME 64

// a table to store all lables and symbols
typedef struct table
{
    char name[MAX_LENGTH_OF_A_INSTRUCTION];
    int address;
    unsigned char type; // SYMBOL or LABLE
    struct table *next; // This is a simple linked list.
} table;

table *table_init(table *head);
table *first_read(table *head, FILE *fr, FILE *fw);                           // the first time to search the whole text file, to get all lables
table *read_lable(table *head, FILE *fr, FILE *fw, int address);              // Get the name of lable
table *table_add(table *head, char *string, int address, unsigned char type); // Remember the instruction number, and store all things into the table
table *second_read(table *head, FILE *fr, FILE *fw);                          // Judge C-instruction or A-instruction, ignore all labels, and print binary code
table *read_symbol(table *head, FILE *fr, FILE *fw);                          // Get the string of A-instruction.
table *symbol_or_lable(table *head, FILE *fw, char *string);                  // Check the string of A-instruction we get is lable or symbol. If it is a symbol, add it to the table. At last, print the A-instruction.
void c_instruction_split(char *string);                                       // All possibilities of C-instruction. At last, print binary code.
void comp_check(FILE *fw, char *string);                                      // All possibilities of computation type
void dest_check(FILE *fw, char *string);                                      // All possibilities of destination
void jump_check(FILE *fw, char *string);                                      // All possibilities of jump
void binary_fprintf(FILE *fw, int num);
void output_string(char *string);
void free_table(table *head);

int main(void)
{
    printf("Please input the name of asm file: ");

    char string[MAX_LENGTH_OF_FILE_NAME];
    scanf("%s", string);

    FILE *fr = fopen(string, "r");

    output_string(string);
    FILE *fw = fopen(string, "a");

    table *head = (table *)malloc(sizeof(table));
    head->name[0] = '\0';
    head->address = 0;
    head->type = SYMBOL;
    head->next = NULL;

    head = table_init(head);
    head = first_read(head, fr, fw);

    fseek(fr, 0, SEEK_SET);
    head = second_read(head, fr, fw);

    fclose(fr);
    fclose(fw);

    free_table(head);

    return EXIT_SUCCESS;
}

table *table_init(table *head)
{
    head = table_add(head, "R0", 0, LABLE);
    head = table_add(head, "R1", 1, LABLE);
    head = table_add(head, "R2", 2, LABLE);
    head = table_add(head, "R3", 3, LABLE);
    head = table_add(head, "R4", 4, LABLE);
    head = table_add(head, "R5", 5, LABLE);
    head = table_add(head, "R6", 6, LABLE);
    head = table_add(head, "R7", 7, LABLE);
    head = table_add(head, "R8", 8, LABLE);
    head = table_add(head, "R9", 9, LABLE);
    head = table_add(head, "R10", 10, LABLE);
    head = table_add(head, "R11", 11, LABLE);
    head = table_add(head, "R12", 12, LABLE);
    head = table_add(head, "R13", 13, LABLE);
    head = table_add(head, "R14", 14, LABLE);
    head = table_add(head, "R15", 15, LABLE);
    head = table_add(head, "SCREEN", 16384, LABLE);
    head = table_add(head, "KBD", 24576, LABLE);
    head = table_add(head, "SP", 0, LABLE);
    head = table_add(head, "LCL", 1, LABLE);
    head = table_add(head, "ARG", 2, LABLE);
    head = table_add(head, "THIS", 3, LABLE);
    head = table_add(head, "THAT", 4, LABLE);

    return head;
}

table *first_read(table *head, FILE *fr, FILE *fw)
{
    int i = -1; // Track the instruction next to the lable
    int c;      // Check the asm file by charactor
    while ((c = fgetc(fr)) != EOF)
    {
        if (c == '(')
        {
            head = read_lable(head, fr, fw, i + 1); // We will read the lable, and we know the instruction number is i + 1.
        }
        else if (c == '\n' || c == '\t' || c == ' ') // We ignore all these things
        {
            continue;
        }
        else if (c == '/')
        {
            while ((c = fgetc(fr)) != '\n' && c != '\t')
            {
                if (c == EOF)
                {
                    break;
                }
            }
        }
        else // If we find the charactor is not the above, we know there is a instruction. We add the instruction number, and we ignore that line until we find \n or \t.
        {
            i++;
            do
            {
                c = fgetc(fr);
                if (c == EOF)
                {
                    break;
                }
            } while (c != '\n' && c != '\t');
        }
    }
    return head;
}

table *read_lable(table *head, FILE *fr, FILE *fw, int address)
{
    char string[MAX_LENGTH_OF_A_INSTRUCTION];
    int i = 0;
    char c;
    while ((c = fgetc(fr)) != ')')
    {
        string[i] = c; // We get each charactor of the lable
        i++;
    }

    string[i] = '\0';                               // A flag means that the string is over.
    head = table_add(head, string, address, LABLE); // We add the lable to the table
    return head;
}

table *table_add(table *head, char *string, int address, unsigned char type)
{
    table *newnode = (table *)malloc(sizeof(table));
    newnode->next = head;
    strcpy(newnode->name, string);
    newnode->address = address;
    newnode->type = type;

    head = newnode;

    return head;
}

table *second_read(table *head, FILE *fr, FILE *fw)
{
    char c;
    while ((c = fgetc(fr)) != EOF)
    {
        if (c == '@') // We find an A-instruction.
        {
            head = read_symbol(head, fr, fw); // We read the address, and judge whether it is a lable or symbol. Finally, we print.
            continue;
        }
        else if (c == '(')
        {
            while (fgetc(fr) != ')')
                ; // We ignore the lable.
            continue;
        }
        else if (c == '/')
        {
            while ((c = fgetc(fr)) != '\n' && c != '\t')
            {
                if (c == EOF)
                {
                    break;
                }
            }
            continue;
        }
        else if (c == '\n' || c == '\t')
        {
            continue;
        }
        else if (c != '\n' && c != '\t' && c != ' ' && c != '/' && c != '@') // This is a C-instruction.
        {                                                                    // Pay attention that the first item will always be executed.
            if (c == EOF)
            {
                return head;
            }
            char string[MAX_LENGTH_OF_C_INSTRUCTION];
            string[0] = c;
            int i = 1;
            while ((c = fgetc(fr)) != '\n' && c != '\t' && c != EOF)
            {
                if (c == ' ')
                {
                    continue; // We ignore all blanks.
                }
                string[i] = c;
                i++;
            }
            string[i] = '\0'; // The string is over.
            fprintf(fw, "111");
            comp_check(fw, string);
            dest_check(fw, string);
            jump_check(fw, string);
            fprintf(fw, "\n");
        }
    }
    return head;
}

table *read_symbol(table *head, FILE *fr, FILE *fw)
{
    int i = 0;
    unsigned char string[MAX_LENGTH_OF_A_INSTRUCTION];
    unsigned char c;
    while ((c = fgetc(fr)) != '\n' && c != '\t' && c != ' ')
    {
        string[i] = c;
        i++;
    }
    string[i] = '\0';
    if (string[0] <= '9' && string[0] >= '0')
    {
        fprintf(fw, "0");
        int sum = 0;
        int times = 1;
        for (int j = 0; j < i; j++)
        {
            times = 1;
            for (int m = 0; m < i - j - 1; m++)
            {
                times *= 10;
            }
            sum += (string[j] - '0') * times;
        }
        binary_fprintf(fw, sum);
        fprintf(fw, "\n");
    }
    else
    {
        head = symbol_or_lable(head, fw, string); // Check the string is symbol or lable //Check the string is symbol or lable.
    }
    return head;
}

table *symbol_or_lable(table *head, FILE *fw, char *string)
{
    table *tmp = head;
    int max_address = INIT_ADDRESS;
    fprintf(fw, "0");
    int i = 0;
    while (tmp->next != NULL)
    {
        if ((i = strcmp(tmp->name, string)) == 0)
        {
            binary_fprintf(fw, tmp->address);
            fprintf(fw, "\n");
            return head;
        }
        if (max_address <= tmp->address && tmp->type == SYMBOL)
        {
            max_address = tmp->address + 1;
        }
        tmp = tmp->next;
    }
    binary_fprintf(fw, max_address);
    fprintf(fw, "\n");
    head = table_add(head, string, max_address, SYMBOL);

    return head;
}

void comp_check(FILE *fw, char *string)
{
    int i = 0; // First we find the position of '='.
    while (string[i] != '=' && string[i] != '\0')
    {
        i++;
    }

    int j = 0;
    while (string[j] != ';' && string[j] != '\0')
    {
        j++;
    }
    // 3 cases
    if (string[i] == '\0')
    {
        // case 1: no destination and but jump
        if (j == 1)
        {
            switch (string[0])
            {
            case '0':
                fprintf(fw, "0101010");
                return;
                ;
            case '1':
                fprintf(fw, "0111111");
                return;
            case 'D':
                fprintf(fw, "0001100");
                return;
            case 'A':
                fprintf(fw, "0110000");
                return;
            case 'M':
                fprintf(fw, "1110000");
                return;
            default:
                exit(1);
                return;
            }
            return;
        }
        else if (j == 2)
        {
            if (string[0] == '-' && string[1] == '1')
            {
                fprintf(fw, "0111010");
                return;
            }
            else if (string[0] == '!' && string[1] == 'D')
            {
                fprintf(fw, "0001101");
                return;
            }
            else if (string[0] == '!' && string[1] == 'A')
            {
                fprintf(fw, "0110001");
                return;
            }
            else if (string[0] == '!' && string[1] == 'M')
            {
                fprintf(fw, "1110001");
                return;
            }
            else if (string[0] == '-' && string[1] == 'D')
            {
                fprintf(fw, "0001111");
                return;
            }
            else if (string[0] == '-' && string[1] == 'A')
            {
                fprintf(fw, "0110011");
                return;
            }
            else if (string[0] == '-' && string[1] == 'M')
            {
                fprintf(fw, "1110011");
                return;
            }
        }
        else if (j == 3)
        {
            if (string[0] == 'D' && string[1] == '+' && string[2] == '1')
            {
                fprintf(fw, "0011111");
                return;
            }
            else if (string[0] == 'A' && string[1] == '+' && string[2] == '1')
            {
                fprintf(fw, "0110111");
                return;
            }
            else if (string[0] == 'M' && string[1] == '+' && string[2] == '1')
            {
                fprintf(fw, "1110111");
                return;
            }
            else if (string[0] == 'D' && string[1] == '-' && string[2] == '1')
            {
                fprintf(fw, "0001110");
                return;
            }
            else if (string[0] == 'A' && string[1] == '-' && string[2] == '1')
            {
                fprintf(fw, "0110010");
                return;
            }
            else if (string[0] == 'M' && string[1] == '-' && string[2] == '1')
            {
                fprintf(fw, "1110010");
                return;
            }
            else if (string[0] == 'D' && string[1] == '+' && string[2] == 'A')
            {
                fprintf(fw, "0000010");
                return;
            }
            else if (string[0] == 'D' && string[1] == '+' && string[2] == 'M')
            {
                fprintf(fw, "1000010");
                return;
            }
            else if (string[0] == 'D' && string[1] == '-' && string[2] == 'A')
            {
                fprintf(fw, "0010011");
                return;
            }
            else if (string[0] == 'D' && string[1] == '-' && string[2] == 'M')
            {
                fprintf(fw, "1010011");
                return;
            }
            else if (string[0] == 'A' && string[1] == '-' && string[2] == 'D')
            {
                fprintf(fw, "0000111");
                return;
            }
            else if (string[0] == 'M' && string[1] == '-' && string[2] == 'D')
            {
                fprintf(fw, "1000111");
                return;
            }
            else if (string[0] == 'D' && string[1] == '&' && string[2] == 'A')
            {
                fprintf(fw, "0000000");
                return;
            }
            else if (string[0] == 'D' && string[1] == '&' && string[2] == 'M')
            {
                fprintf(fw, "1000000");
                return;
            }
            else if (string[0] == 'D' && string[1] == '|' && string[2] == 'A')
            {
                fprintf(fw, "0010101");
                return;
            }
            else if (string[0] == 'D' && string[1] == '|' && string[2] == 'M')
            {
                fprintf(fw, "1010101");
                return;
            }

            return;
        }
    }
    else
    {
        // case 2: no jump but destination && case 3: jump and destination
        if (j - i == 2)
        {
            switch (string[i + 1])
            {
            case '0':
                fprintf(fw, "0101010");
                return;
            case '1':
                fprintf(fw, "0111111");
                return;
            case 'D':
                fprintf(fw, "0001100");
                return;
            case 'A':
                fprintf(fw, "0110000");
                return;
            case 'M':
                fprintf(fw, "1110000");
                return;
            default:
                exit(1);
                return;
            }
            return;
        }
        else if (j - i == 3)
        {
            if (string[i + 1] == '-' && string[i + 2] == '1')
            {
                fprintf(fw, "0111010");
                return;
            }
            else if (string[i + 1] == '!' && string[i + 2] == 'D')
            {
                fprintf(fw, "0001101");
                return;
            }
            else if (string[i + 1] == '!' && string[i + 2] == 'A')
            {
                fprintf(fw, "0110001");
                return;
            }
            else if (string[i + 1] == '!' && string[i + 2] == 'M')
            {
                fprintf(fw, "1110001");
                return;
            }
            else if (string[i + 1] == '-' && string[i + 2] == 'D')
            {
                fprintf(fw, "0001111");
                return;
            }
            else if (string[i + 1] == '-' && string[i + 2] == 'A')
            {
                fprintf(fw, "0110011");
                return;
            }
            else if (string[i + 1] == '-' && string[i + 2] == 'M')
            {
                fprintf(fw, "1110011");
                return;
            }
        }
        else if (j - i == 4)
        {
            if (string[i + 1] == 'D' && string[i + 2] == '+' && string[i + 3] == '1')
            {
                fprintf(fw, "0011111");
                return;
            }
            else if (string[i + 1] == 'A' && string[i + 2] == '+' && string[i + 3] == '1')
            {
                fprintf(fw, "0110111");
                return;
            }
            else if (string[i + 1] == 'M' && string[i + 2] == '+' && string[i + 3] == '1')
            {
                fprintf(fw, "1110111");
                return;
            }
            else if (string[i + 1] == 'D' && string[i + 2] == '-' && string[i + 3] == '1')
            {
                fprintf(fw, "0001110");
                return;
            }
            else if (string[i + 1] == 'A' && string[i + 2] == '-' && string[i + 3] == '1')
            {
                fprintf(fw, "0110010");
                return;
            }
            else if (string[i + 1] == 'M' && string[i + 2] == '-' && string[i + 3] == '1')
            {
                fprintf(fw, "1110010");
                return;
            }
            else if (string[i + 1] == 'D' && string[i + 2] == '+' && string[i + 3] == 'A')
            {
                fprintf(fw, "0000010");
                return;
            }
            else if (string[i + 1] == 'D' && string[i + 2] == '+' && string[i + 3] == 'M')
            {
                fprintf(fw, "1000010");
                return;
            }
            else if (string[i + 1] == 'D' && string[i + 2] == '-' && string[i + 3] == 'A')
            {
                fprintf(fw, "0010011");
                return;
            }
            else if (string[i + 1] == 'D' && string[i + 2] == '-' && string[i + 3] == 'M')
            {
                fprintf(fw, "1010011");
                return;
            }
            else if (string[i + 1] == 'A' && string[i + 2] == '-' && string[i + 3] == 'D')
            {
                fprintf(fw, "0000111");
                return;
            }
            else if (string[i + 1] == 'M' && string[i + 2] == '-' && string[i + 3] == 'D')
            {
                fprintf(fw, "1000111");
                return;
            }
            else if (string[i + 1] == 'D' && string[i + 2] == '&' && string[i + 3] == 'A')
            {
                fprintf(fw, "0000000");
                return;
            }
            else if (string[i + 1] == 'D' && string[i + 2] == '&' && string[i + 3] == 'M')
            {
                fprintf(fw, "1000000");
                return;
            }
            else if (string[i + 1] == 'D' && string[i + 2] == '|' && string[i + 3] == 'A')
            {
                fprintf(fw, "0010101");
                return;
            }
            else if (string[i + 1] == 'D' && string[i + 2] == '|' && string[i + 3] == 'M')
            {
                fprintf(fw, "1010101");
                return;
            }

            return;
        }
    }
}

void dest_check(FILE *fw, char *string)
{
    int i = 0;
    while (string[i] != '=' && string[i] != '\0')
    {
        i++;
    }

    if (string[i] == '\0') // no destination
    {
        fprintf(fw, "000");
        return;
    }
    else if (i == 1)
    {
        if (string[0] == 'M')
        {
            fprintf(fw, "001");
            return;
        }
        else if (string[0] == 'D')
        {
            fprintf(fw, "010");
        }
        else if (string[0] == 'A')
        {
            fprintf(fw, "100");
        }
        return;
    }
    else if (i == 2)
    {
        if (string[0] == 'M' && string[1] == 'D')
        {
            fprintf(fw, "011");
            return;
        }
        else if (string[0] == 'A' && string[1] == 'M')
        {
            fprintf(fw, "101");
            return;
        }
        else if (string[0] == 'A' && string[1] == 'D')
        {
            fprintf(fw, "110");
            return;
        }
        else
        {
            exit(1);
        }

        return;
    }
    else if (i == 3)
    {
        if (string[0] == 'A' && string[1] == 'M' && string[2] == 'D')
        {
            fprintf(fw, "111");
            return;
        }
        else
        {
            exit(1);
        }

        return;
    }
    return;
}

void jump_check(FILE *fw, char *string)
{
    int i = 0;
    while (string[i] != ';' && string[i] != '\0')
    {
        i++;
    }

    if (string[i] == '\0')
    {
        // no jump
        fprintf(fw, "000");
        return;
    }
    else if (string[i + 2] == 'G' && string[i + 3] == 'T') // The first charactor is J for all cases. So we ignore it.
    {
        fprintf(fw, "001");
        return;
    }
    else if (string[i + 2] == 'E' && string[i + 3] == 'Q')
    {
        fprintf(fw, "010");
        return;
    }
    else if (string[i + 2] == 'G' && string[i + 3] == 'E')
    {
        fprintf(fw, "011");
        return;
    }
    else if (string[i + 2] == 'L' && string[i + 3] == 'T')
    {
        fprintf(fw, "100");
        return;
    }
    else if (string[i + 2] == 'N' && string[i + 3] == 'E')
    {
        fprintf(fw, "101");
        return;
    }
    else if (string[i + 2] == 'L' && string[i + 3] == 'E')
    {
        fprintf(fw, "110");
        return;
    }
    else if (string[i + 2] == 'M' && string[i + 3] == 'P')
    {
        fprintf(fw, "111");
        return;
    }

    return;
}

void binary_fprintf(FILE *fw, int num)
{
    char binary[15];
    for (int i = 0; i < 15; i++)
    {
        binary[i] = 0;
    }

    int remainder = 0;
    int j = 14;
    do
    {
        remainder = num % 2;
        num = num / 2;
        binary[j] = remainder;
        j--;
    } while (num != 0);

    for (int i = 0; i < 15; i++)
    {
        fprintf(fw, "%d", binary[i]);
    }
    return;
}

void output_string(char *string)
{
    int i = 0;
    while (string[i] != '.')
    {
        i++;
    }
    string[i + 1] = 'h';
    string[i + 2] = 'a';
    string[i + 3] = 'c';
    string[i + 4] = 'k';
}

void free_table(table *head)
{
    if (head->next == NULL)
    {
        free(head);
        return;
    }

    free_table(head->next);

    free(head);
    return;
}
