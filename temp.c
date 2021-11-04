# include <stdio.h>
# include <stdlib.h>
# include "defs.h"
//==================================================================================================
struct MIX
{
    unsigned char   rA      [6],
                    rX      [6],
                    rI      [6][3],
                    rJ      [3],
                    overflow,                 // флаг переполнения
                    compare,                  // флаг сравнения
                    vram    [4000][6];
    unsigned short  ip;
}   cpu;
# include "addfunc.c"
# include "instructions.c"
int main ()
{
    system ("color f0");
    zeromemory (&cpu);
    cpu.vram[100][5] = HLT;
    loader (&cpu);
    unsigned char* word;
    for (1;1;1)
    {
        if (cpu.ip > 3999) { setRJ (&cpu, 0); setIP(&cpu); }
        word = cpu.vram[cpu.ip];
        //word[5] = 8; word[4] = 3; word[3] = 2; word[2] = 0; word[1] = 20; word [0] = 0;
        switch( word[5] )
        {
            case NOP:  incRJ (&cpu); setIP (&cpu);                                            break;
            case ADD:  ariphmetic (&cpu, word);                                               break;
            case SUB:  ariphmetic (&cpu, word);                                               break;
            case MUL:  ariphmetic (&cpu, word);                                               break;
            case DIV:  ariphmetic (&cpu, word);                                               break;
            case HLT:  printf ("Mashine is halt\n"); system("pause"); memorydump (&cpu);   return 0;
            case SHFT: shift(&cpu, word);                                                     break;
            case MOVE: move(&cpu, word);                                                      break;
            case LDA:  mov(&cpu, word, cpu.rA   , 5);                                         break;
            case LD1:  mov(&cpu, word, cpu.rI[0], 2);                                         break;
            case LD2:  mov(&cpu, word, cpu.rI[1], 2);                                         break;
            case LD3:  mov(&cpu, word, cpu.rI[2], 2);                                         break;
            case LD4:  mov(&cpu, word, cpu.rI[3], 2);                                         break;
            case LD5:  mov(&cpu, word, cpu.rI[4], 2);                                         break;
            case LD6:  mov(&cpu, word, cpu.rI[5], 2);                                         break;
            case LDX:  mov(&cpu, word, cpu.rX   , 5);                                         break;
            case LDAN: inverse(&cpu,word); mov(&cpu, word, cpu.rA   , 5); inverse(&cpu,word); break;
            case LD1N: inverse(&cpu,word); mov(&cpu, word, cpu.rI[0], 2); inverse(&cpu,word); break;
            case LD2N: inverse(&cpu,word); mov(&cpu, word, cpu.rI[1], 2); inverse(&cpu,word); break;
            case LD3N: inverse(&cpu,word); mov(&cpu, word, cpu.rI[2], 2); inverse(&cpu,word); break;
            case LD4N: inverse(&cpu,word); mov(&cpu, word, cpu.rI[3], 2); inverse(&cpu,word); break;
            case LD5N: inverse(&cpu,word); mov(&cpu, word, cpu.rI[4], 2); inverse(&cpu,word); break;
            case LD6N: inverse(&cpu,word); mov(&cpu, word, cpu.rI[5], 2); inverse(&cpu,word); break;
            case LDXN: inverse(&cpu,word); mov(&cpu, word, cpu.rX   , 5); inverse(&cpu,word); break;
            case STA:  mov(&cpu, word, cpu.rA   , 5);                                         break;
            case ST1:  mov(&cpu, word, cpu.rI[0], 2);                                         break;
            case ST2:  mov(&cpu, word, cpu.rI[1], 2);                                         break;
            case ST3:  mov(&cpu, word, cpu.rI[2], 2);                                         break;
            case ST4:  mov(&cpu, word, cpu.rI[3], 2);                                         break;
            case ST5:  mov(&cpu, word, cpu.rI[4], 2);                                         break;
            case ST6:  mov(&cpu, word, cpu.rI[5], 2);                                         break;
            case STX:  mov(&cpu, word, cpu.rX   , 5);                                         break;
            case STJ:  mov(&cpu, word, cpu.rJ   , 2);                                         break;
            case STZ:  mov(&cpu, word, 0        , 0);                                         break;
            case JMP:  jump(&cpu, word, 0        , 0);                                        break;
            case JMPA: jump(&cpu, word, cpu.rA   , 5);                                        break;
            case JMP1: jump(&cpu, word, cpu.rI[0], 2);                                        break;
            case JMP2: jump(&cpu, word, cpu.rI[1], 2);                                        break;
            case JMP3: jump(&cpu, word, cpu.rI[2], 2);                                        break;
            case JMP4: jump(&cpu, word, cpu.rI[3], 2);                                        break;
            case JMP5: jump(&cpu, word, cpu.rI[4], 2);                                        break;
            case JMP6: jump(&cpu, word, cpu.rI[5], 2);                                        break;
            case JMPX: jump(&cpu, word, cpu.rX   , 5);                                        break;
            case AOPA: addressoperation(&cpu, word);                                          break;
            case AOP1: addressoperation(&cpu, word);                                          break;
            case AOP2: addressoperation(&cpu, word);                                          break;
            case AOP3: addressoperation(&cpu, word);                                          break;
            case AOP4: addressoperation(&cpu, word);                                          break;
            case AOP5: addressoperation(&cpu, word);                                          break;
            case AOP6: addressoperation(&cpu, word);                                          break;
            case AOPX: addressoperation(&cpu, word);                                          break;
            case CMPA: compare(&cpu, word, cpu.rA   , 5);                                     break;
            case CMP1: compare(&cpu, word, cpu.rI[0], 2);                                     break;
            case CMP2: compare(&cpu, word, cpu.rI[1], 2);                                     break;
            case CMP3: compare(&cpu, word, cpu.rI[2], 2);                                     break;
            case CMP4: compare(&cpu, word, cpu.rI[3], 2);                                     break;
            case CMP5: compare(&cpu, word, cpu.rI[4], 2);                                     break;
            case CMP6: compare(&cpu, word, cpu.rI[5], 2);                                     break;
            case CMPX: compare(&cpu, word, cpu.rX   , 5);                                     break;
            default:   incRJ (&cpu); setIP (&cpu);                                            break;
        }
        printf ("IP  [%5i]\n",cpu.ip);
        if (cpu.rA[0]) printf ("rA: [-]"); else printf ("rA: [+]"); printf ("[%2X][%2X][%2X][%2X][%2X]\n",cpu.rA[1],cpu.rA[2],cpu.rA[3],cpu.rA[4],cpu.rA[5]);
        if (cpu.rX[0]) printf ("rX: [-]"); else printf ("rX: [+]"); printf ("[%2X][%2X][%2X][%2X][%2X]\n",cpu.rX[1],cpu.rX[2],cpu.rX[3],cpu.rX[4],cpu.rX[5]);
        if (cpu.rI[0][0]) printf ("r1: [-]"); else printf ("r1: [+]"); printf ("[%2X][%2X]\n",cpu.rI[0][1],cpu.rI[0][2]);
        if (cpu.rI[1][0]) printf ("r2: [-]"); else printf ("r2: [+]"); printf ("[%2X][%2X]\n",cpu.rI[1][1],cpu.rI[1][2]);
        if (cpu.rI[2][0]) printf ("r3: [-]"); else printf ("r3: [+]"); printf ("[%2X][%2X]\n",cpu.rI[2][1],cpu.rI[2][2]);
        if (cpu.rI[3][0]) printf ("r4: [-]"); else printf ("r4: [+]"); printf ("[%2X][%2X]\n",cpu.rI[3][1],cpu.rI[3][2]);
        if (cpu.rI[4][0]) printf ("r5: [-]"); else printf ("r5: [+]"); printf ("[%2X][%2X]\n",cpu.rI[4][1],cpu.rI[4][2]);
        if (cpu.rI[5][0]) printf ("r6: [-]"); else printf ("r6: [+]"); printf ("[%2X][%2X]\n",cpu.rI[5][1],cpu.rI[5][2]);
        printf ("rJ: [+][%2X][%2X]\n",cpu.rJ[1],cpu.rJ[2]);
        if (cpu.overflow)           printf ("OVERFLOW = TRUE\n");
        else                        printf ("OVERFLOW = FALSE\n");
        if (!cpu.compare)           printf ("COMPARE FLAG = VOID\n");
        if (cpu.compare == EQUAL)   printf ("COMPARE FLAG = EQUAL\n");
        if (cpu.compare == LESS)    printf ("COMPARE FLAG = LESS\n");
        if (cpu.compare == GREATER) printf ("COMPARE FLAG = GREATER\n");
        system ("pause"); system ("cls");
        //printf ("rA\t%i %i %i %i %i\n",cpu.rA[0],cpu.rA[1],cpu.rA[2],cpu.rA[3],cpu.rA[4],cpu.rA[5]);
    }
    system ("pause");
    return 0;
}