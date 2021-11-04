#include <stdint.h>
void mov (struct MIX* cpuunit, const unsigned char miw[6], unsigned char* reg, int rsiz)
{
    char l = miw[4]/8, r = miw[4]%8; // вычисляем значение L и R из поля F
    long long adds = makeaddss (cpuunit, miw);
    if (adds<0) adds = -adds;
    if (adds>3999) { incRJ (cpuunit); setIP(cpuunit); return; }
    if (!reg) // если указатель на регистр нулевой, то это команда STZ - обнуляем память по адресу
    {
        for (int i=l; i<=r; ++i) cpuunit->vram[adds][i] = 0;
        incRJ (cpuunit); setIP(cpuunit); return;
    }
    if (miw[5] < 24) // инструкция LOAD
    {
        unsigned char buffreg[6] = {0};
        if (rsiz == 2)
        {
            buffreg[0] = reg[0]; buffreg[4] = reg[1]; buffreg[5] = reg[2];
            for (int i=l; i<=r; ++i)    buffreg[i] = cpuunit->vram[adds][i];
            reg[0] = buffreg[0]; reg[1] = buffreg[4]; reg[2] = buffreg[5];
        }
        else
        {
            buffreg[0] = reg[0]; buffreg[1] = reg[1]; buffreg[2] = reg[2];
            buffreg[3] = reg[3]; buffreg[4] = reg[4]; buffreg[5] = reg[5];
            for (int i=l; i<=r; ++i)    buffreg[i] = cpuunit->vram[adds][i];
            reg[0] = buffreg[0]; reg[1] = buffreg[1]; reg[2] = buffreg[2];
            reg[3] = buffreg[3]; reg[4] = buffreg[4]; reg[5] = buffreg[5];
        }
    }
    else // инструкция STORE
    {
        unsigned char buffreg[6] = {0};
        if (rsiz == 2)
        {
            buffreg[0] = reg[0]; buffreg[4] = reg[1]; buffreg[5] = reg[2];
            for (int i=l; i<=r; ++i)    cpuunit->vram[adds][i] = buffreg[i];
        }
        else
        {
            buffreg[0] = reg[0]; buffreg[1] = reg[1]; buffreg[2] = reg[2];
            buffreg[3] = reg[3]; buffreg[4] = reg[4]; buffreg[5] = reg[5];
            for (int i=l; i<=r; ++i)    cpuunit->vram[adds][i] = buffreg[i];
            //reg[0] = buffreg[0]; reg[1] = buffreg[1]; reg[2] = buffreg[2];
            //reg[3] = buffreg[3]; reg[4] = buffreg[4]; reg[5] = buffreg[5];
        }
    }
    incRJ (cpuunit);
    setIP (cpuunit);
    return ;
}
//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::://
void ariphmetic (struct MIX* cpuunit, const unsigned char miw[6])
{
    char l = miw[4]/8, r = miw[4]%8; // вычисляем значение L и R из поля F
    long long adds = makeaddss (cpuunit, miw);
    if (adds<0) adds = -adds;
    if (adds>3999) { incRJ (cpuunit); setIP(cpuunit); return; }
    long long tmpmem = 0, tmpreg = 0;
    for (int i = l; i<=r; ++i) // сложением и сдвигами в tmpmem формируем содержимое ячейки памяти
    {
        tmpmem = tmpmem << 8;
        tmpmem += cpuunit->vram[adds][i];
    }
    if (!l) // если поле L = 0
        if (cpuunit->vram[adds][0]) // если знак числа - '-'
        {
            long long signbyte = cpuunit->vram[adds][0];
            signbyte = signbyte << 40;
            tmpmem -= signbyte; // удаляем знаковый байт из сформированного числа
            tmpmem = -tmpmem; // меняем реальный знак сформированного числа
        }
    for (int i = 1; i<=5; ++i) // в tmpreg формируем значение регистра
    {
        tmpreg = tmpreg << 8;
        tmpreg += cpuunit->rA[i];
    }
    if (cpuunit->rA[0] && !l) tmpreg = -tmpreg; // если необходимо, то меняем реальный знак числа
    switch (miw[5])
    {
        case ADD:
            //printf (">>>ADD DEBUG PRINT<<<\n");
            //printf ("%X + %X",tmpreg, tmpmem);
            tmpreg += tmpmem;
            //printf (" = %X\n",tmpreg);
            if (tmpreg<0)
            {
                cpuunit->rA[0] = 1;
                tmpreg = -tmpreg;
            }
            else cpuunit->rA[0] = 0;
            if (tmpreg>0x000000FFFFFFFFFF) // проверка переполнения
            {
                cpuunit->overflow = 1;
                tmpreg = tmpreg << 24 >> 24;
            }
            cpuunit->rA[1] = tmpreg >> 32;
            cpuunit->rA[2] = (tmpreg << 32) >> 56;  cpuunit->rA[3] = (tmpreg << 40) >> 56;
            cpuunit->rA[4] = (tmpreg << 48) >> 56;  cpuunit->rA[5] = (tmpreg << 56) >> 56;
            break;
//------------------------------------------------------------------------------------------------//
        case SUB:
            tmpreg -= tmpmem;
            if (tmpreg<0)
            {
                cpuunit->rA[0] = 1;
                tmpreg = -tmpreg;
            }
            else cpuunit->rA[0] = 0;
            if (tmpreg>0x000000FFFFFFFFFF) // проверка переполнения
            {
                cpuunit->overflow = 1;
                tmpreg = tmpreg << 24 >> 24;
            }
            cpuunit->rA[1] = tmpreg >> 32;
            cpuunit->rA[2] = (tmpreg << 32) >> 56;  cpuunit->rA[3] = (tmpreg << 40) >> 56;
            cpuunit->rA[4] = (tmpreg << 48) >> 56;  cpuunit->rA[5] = (tmpreg << 56) >> 56;
            break;
//------------------------------------------------------------------------------------------------//
        case MUL:
        {
            long long tmpa = 0, tmpx = 0, tmpmul[5] = {0};
            if ((tmpreg<0 && tmpmem>0)||(tmpreg>0 && tmpmem<0))
            {
                if (tmpreg<0) tmpreg = -tmpreg;
                if (tmpmem<0) tmpmem = -tmpmem;
                cpuunit->rA[0] = cpuunit->rX[0] = 1;
            }
            int j = 0;
            for (int i=r; i>=l; --i)
                if (i) { tmpmul[j] = tmpreg * cpuunit->vram [adds] [i]; ++j; }
            //tmpmul[0] = tmpreg * cpuunit->vram [adds] [5];
            //tmpmul[1] = tmpreg * cpuunit->vram [adds] [4];
            //tmpmul[2] = tmpreg * cpuunit->vram [adds] [3];
            //tmpmul[3] = tmpreg * cpuunit->vram [adds] [2];
            //tmpmul[4] = tmpreg * cpuunit->vram [adds] [1];
            tmpx =  tmpmul[0];
            tmpx += ((tmpmul[1] << 8 ) << 24) >> 24;    tmpa += tmpmul[1] >> 32;
            tmpx += ((tmpmul[2] << 16) << 24) >> 24;    tmpa += tmpmul[2] >> 24;
            tmpx += ((tmpmul[3] << 24) << 24) >> 24;    tmpa += tmpmul[3] >> 16;
            tmpx += ((tmpmul[4] << 32) << 24) >> 24;    tmpa += tmpmul[4] >> 8;
            tmpa += tmpx >> 40;
            tmpx =  tmpx << 24 >> 24;
            cpuunit->rA[1] =  tmpa >> 32;           cpuunit->rX[1] =  tmpx >> 32;
            cpuunit->rA[2] = (tmpa << 32) >> 56;    cpuunit->rX[2] = (tmpx << 32) >> 56;
            cpuunit->rA[3] = (tmpa << 40) >> 56;    cpuunit->rX[3] = (tmpx << 40) >> 56;
            cpuunit->rA[4] = (tmpa << 48) >> 56;    cpuunit->rX[4] = (tmpx << 48) >> 56;
            cpuunit->rA[5] = (tmpa << 56) >> 56;    cpuunit->rX[5] = (tmpx << 56) >> 56;
            break;
        }
//------------------------------------------------------------------------------------------------//
        case DIV:
        {
            unsigned long long num = 0, tmpa = 0, tmpx = 0;
            if ((tmpreg<0 && tmpmem>0)||(tmpreg>0 && tmpmem<0))
            {
                if (tmpreg<0) tmpreg = -tmpreg;
                if (tmpmem<0) tmpmem = -tmpmem;
                cpuunit->rA[0] = cpuunit->rX[0] = 1;
            }
            num += cpuunit->rA[3];  num = num << 8; num += cpuunit->rA[4];  num = num << 8;
            num += cpuunit->rA[5];  num = num << 8; num += cpuunit->rX[0];  num = num << 8;
            num += cpuunit->rX[1];  num = num << 8; num += cpuunit->rX[2];  num = num << 8;
            num += cpuunit->rX[3];  num = num << 8; num += cpuunit->rX[4];  num = num << 8;
            num += cpuunit->rX[5];  num = num << 8;
            tmpa = num / tmpmem;     tmpa = tmpa << 24 >> 24;
            tmpx = num % tmpmem;     tmpx = tmpx << 24 >> 24;
            cpuunit->rA[1] =  tmpa >> 32;           cpuunit->rX[1] =  tmpx >> 32;
            cpuunit->rA[2] = (tmpa << 32) >> 56;    cpuunit->rX[2] = (tmpx << 32) >> 56;
            cpuunit->rA[3] = (tmpa << 40) >> 56;    cpuunit->rX[3] = (tmpx << 40) >> 56;
            cpuunit->rA[4] = (tmpa << 48) >> 56;    cpuunit->rX[4] = (tmpx << 48) >> 56;
            cpuunit->rA[5] = (tmpa << 56) >> 56;    cpuunit->rX[5] = (tmpx << 56) >> 56;
            break;
        }
//------------------------------------------------------------------------------------------------//
        default : break;
    }
    incRJ (cpuunit);
    setIP (cpuunit);
    return ;
}
//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::://
void addressoperation (struct MIX* cpuunit, const unsigned char miw[6])
{
    //printf (">>>AOP DEBUG PRINT<<<\n");
    long long adds = makeaddss (cpuunit, miw); //printf (">>>ADDS = %X\n",adds);
    long long tmpreg = 0;
    int index = miw[5] - 48, rsiz = 0;
    unsigned char* reg = 0;
    switch (index) // в зависимости от значения опкода выбираем регистр, с которым дальше работаем
    {
        case 0: reg = cpuunit->rA;    rsiz = 5; break;
        case 1: reg = cpuunit->rI[0]; rsiz = 2; break;
        case 2: reg = cpuunit->rI[1]; rsiz = 2; break;
        case 3: reg = cpuunit->rI[2]; rsiz = 2; break;
        case 4: reg = cpuunit->rI[3]; rsiz = 2; break;
        case 5: reg = cpuunit->rI[4]; rsiz = 2; break;
        case 6: reg = cpuunit->rI[5]; rsiz = 2; break;
        case 7: reg = cpuunit->rX;    rsiz = 5; break;
    }
    for (int i = 1; i<=rsiz; ++i) // в tmpreg формируем значение регистра
    {// ошибка аккумуляции (исправлено)
        tmpreg = tmpreg << 8;
        tmpreg += reg[i];
    }//printf (">>>REGA = %X\n",tmpreg);
    if (reg[0]) tmpreg = -tmpreg;
    switch(miw[4]) // в зависимости от значения поля F выбираем необходимую инструкцию
    {
        case INC:   tmpreg += adds; break;
        case DEC:   tmpreg -= adds; break;
        case ENT:   tmpreg =  adds; break;
        case ENN:   tmpreg = -adds; break;
        default: break;
    }
    if (tmpreg < 0) { reg[0] = 1; tmpreg = -tmpreg; }
    if (rsiz == 2 && tmpreg > 0x000000000000FFFF) // проверка переполнения для 2-байтного регистра
    {
        cpuunit->overflow = 1;
        tmpreg = tmpreg << 48 >> 48;
    }
    if (rsiz == 5 && tmpreg > 0x000000FFFFFFFFFF) // проверка переполнения для 5-байтного регистра
    {
        cpuunit->overflow = 1;
        tmpreg = tmpreg << 24 >> 24;
    }
    if (rsiz == 2)
    {
        reg[1] = tmpreg >> 8;
        reg[2] = tmpreg - ((long long)reg[1] << 8);
    }
    else
    {//printf (">>>REGP = %X\n",tmpreg);
        reg[1] = tmpreg >> 32;                  reg[2] = (tmpreg << 32 >> 32) >> 24;
        reg[3] = (tmpreg << 40 >> 40) >> 16;    reg[4] = (tmpreg << 48 >> 48) >> 8;
        reg[5] = (tmpreg << 56 >> 56);
    }
    incRJ (cpuunit);
    setIP (cpuunit);
    return ;
}
//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::://
void compare (struct MIX* cpuunit, const unsigned char miw[6], unsigned char* reg, int rsiz)
{
    char l = miw[4]/8, r = miw[4]%8; // вычисляем значение L и R из поля F
    long long adds = makeaddss (cpuunit, miw);
    if (adds<0) adds = -adds;
    if (adds>3999) { incRJ (cpuunit); setIP(cpuunit); return; }
    long long tmpreg = 0, tmpmem = 0;
    for (int i = l; i<=r; ++i)
    {
        if (rsiz == 2)
            if (i >= 4)
                tmpreg += ((long long)reg[i-3]) << 8;
        if (rsiz == 5)
            if (i)
                tmpreg += ((long long)reg[i]) << 8;
        if (i)
            tmpmem += ((long long)cpuunit->vram[adds][i]) << 8;
    }
    if (l) { tmpreg = -tmpreg; tmpmem = -tmpmem; }
    if (tmpreg >  tmpmem)     cpuunit->compare = GREATER;
    if (tmpreg == tmpmem)     cpuunit->compare = EQUAL;
    if (tmpreg <  tmpmem)     cpuunit->compare = LESS;
    incRJ (cpuunit);
    setIP (cpuunit);
    return ;
}
//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::://
void jump (struct MIX* cpuunit, const unsigned char miw[6], unsigned char* reg, int rsiz)
{
    long long adds = makeaddss (cpuunit, miw);
    if (adds<0) adds = -adds;
    if (adds>3999) { incRJ (cpuunit); setIP(cpuunit); return; }
    if (!reg)
    {
        switch (miw[4])
        {
/*JMP  */   case 0: setRJ (cpuunit, adds); setIP(cpuunit);                                  return ;
/*JSJ  */   case 1: /*incRJ (cpuunit);*/ cpuunit->ip = adds;                                return ;
/*JOV  */   case 2:
                if (cpuunit->overflow)
                {
                    cpuunit->overflow = 0; setRJ(cpuunit, adds); setIP(cpuunit);
                }
                else { incRJ (cpuunit); setIP (cpuunit); }
                return ;
/*JNOV */   case 3:
                if (!cpuunit->overflow)
                {
                    setRJ (cpuunit, adds);  setIP(cpuunit);
                }
                else { incRJ (cpuunit); setIP (cpuunit); }
                return ;
/*JL   */   case 4:
                if (cpuunit->compare == LESS)
                {
                    setRJ (cpuunit, adds);  setIP(cpuunit);
                }
                else { incRJ (cpuunit); setIP (cpuunit); }
                return ;
/*JE   */   case 5:
                if (cpuunit->compare == EQUAL)
                {
                    setRJ (cpuunit, adds);  setIP(cpuunit);
                }
                else { incRJ (cpuunit); setIP (cpuunit); }
                return ;
/*JG   */   case 6:
                if (cpuunit->compare == GREATER)
                {
                   setRJ (cpuunit, adds);   setIP(cpuunit);
                }
                else { incRJ (cpuunit); setIP (cpuunit); }
                return ;
/*JGE  */   case 7:
                if (cpuunit->compare == EQUAL || cpuunit->compare == GREATER)
                {
                    setRJ (cpuunit, adds);  setIP(cpuunit);
                }
                else { incRJ (cpuunit); setIP (cpuunit); }
                return ;
/*JNE  */   case 8:
                if (cpuunit->compare == LESS || cpuunit->compare == GREATER)
                {
                    setRJ (cpuunit, adds);  setIP(cpuunit);
                }
                else { incRJ (cpuunit); setIP (cpuunit); }
                return ;
/*JLE  */   case 9:
                if (cpuunit->compare == EQUAL || cpuunit->compare == LESS)
                {
                    setRJ (cpuunit, adds);  setIP(cpuunit);
                }
                else { incRJ (cpuunit); setIP (cpuunit); }
                return ;
            default : incRJ (cpuunit); setIP (cpuunit); return ;
        }
    }
//------------------------------------------------------------------------------------------------//
    else
    {
        long long tmpreg = 0;
        for (int i=1; i<=rsiz; ++i)
        {
            tmpreg = tmpreg << 8;
            tmpreg += reg[i];
        }
        if (reg[0]) tmpreg = -tmpreg;
        switch (miw[4])
        {
/*JRN  */   case 0:
                if (tmpreg<0)   { setRJ (cpuunit, adds);  setIP(cpuunit);   }
                else            { incRJ (cpuunit); setIP (cpuunit);         }
                return ;
/*JRZ  */   case 1:
                if (tmpreg==0)  { setRJ (cpuunit, adds);  setIP(cpuunit);   }
                else            { incRJ (cpuunit); setIP (cpuunit);         }
                return ;
/*JRP  */   case 2:
                if (tmpreg>0)   { setRJ (cpuunit, adds);  setIP(cpuunit);   }
                else            { incRJ (cpuunit); setIP (cpuunit);         }
                return ;
/*JRNN */   case 3:
                if (tmpreg>=0)  { setRJ (cpuunit, adds);  setIP(cpuunit);   }
                else            { incRJ (cpuunit); setIP (cpuunit);         }
                return ;
/*JRNZ */   case 4:
                if (tmpreg!=0)  { setRJ (cpuunit, adds);  setIP(cpuunit);   }
                else            { incRJ (cpuunit); setIP (cpuunit);         }
                return ;
/*JRNP */   case 5:
                if (tmpreg<=0)  { setRJ (cpuunit, adds);  setIP(cpuunit);   }
                else            { incRJ (cpuunit); setIP (cpuunit);         }
                return ;
            default : incRJ (cpuunit); setIP (cpuunit); return ;
        }
    }
    return ;
}
//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::://
void shift (struct MIX* c, const unsigned char miw[6])
{
    long long adds = makeaddss (c, miw);
    int i, j;
    if (adds<0) adds = -adds;
    switch (miw[4])
    {
        case SLA:
            for (i=0; i<adds; ++i)
            {
                for (j=1; j<5; ++j) c->rA[j] = c->rA[j+1];
                c->rA[j] = 0;
            }
            break;
        case SRA:
            for (i=1; i<adds; ++i)
            {
                for (j=5; j>1; --j) c->rA[j] = c->rA[j-1];
                c->rA[j] = 0;
            }
            break;
        case SLAX:
        {
            unsigned char ax[10] = {c->rA[1],c->rA[2],c->rA[3],c->rA[4],c->rA[5],
                                    c->rX[1],c->rX[2],c->rX[3],c->rX[4],c->rX[5]};
            for (i=0; i<adds; ++i)
            {
                for (j=0; j<9; ++j) ax[j] = ax[j+1];
                ax[j] = 0;
            }
            c->rA[1]=ax[0]; c->rA[2]=ax[1]; c->rA[3]=ax[2]; c->rA[4]=ax[3]; c->rA[5]=ax[4];
            c->rX[1]=ax[5]; c->rX[2]=ax[6]; c->rX[3]=ax[7]; c->rX[4]=ax[8]; c->rX[5]=ax[9];
            break;
        }
        case SRAX:
        {
            unsigned char ax[10] = {c->rA[1],c->rA[2],c->rA[3],c->rA[4],c->rA[5],
                                    c->rX[1],c->rX[2],c->rX[3],c->rX[4],c->rX[5]};
            for (i=0; i<adds; ++i)
            {
                for (j=9; j>0; --j) ax[j] = ax[j-1];
                ax[j] = 0;
            }
            c->rA[1]=ax[0]; c->rA[2]=ax[1]; c->rA[3]=ax[2]; c->rA[4]=ax[3]; c->rA[5]=ax[4];
            c->rX[1]=ax[5]; c->rX[2]=ax[6]; c->rX[3]=ax[7]; c->rX[4]=ax[8]; c->rX[5]=ax[9];
            break;
        }
        case SLC:
        {
            unsigned char ax[10] = {c->rA[1],c->rA[2],c->rA[3],c->rA[4],c->rA[5],
                                    c->rX[1],c->rX[2],c->rX[3],c->rX[4],c->rX[5]};
            for (i=0; i<adds; ++i)
            {
                unsigned char tmp = ax[0];
                for (j=0; j<9; ++j) ax[j] = ax[j+1];
                ax[j] = tmp;
            }
            c->rA[1]=ax[0]; c->rA[2]=ax[1]; c->rA[3]=ax[2]; c->rA[4]=ax[3]; c->rA[5]=ax[4];
            c->rX[1]=ax[5]; c->rX[2]=ax[6]; c->rX[3]=ax[7]; c->rX[4]=ax[8]; c->rX[5]=ax[9];
            break;
        }
        case SRC:
        {
            unsigned char ax[10] = {c->rA[1],c->rA[2],c->rA[3],c->rA[4],c->rA[5],
                                    c->rX[1],c->rX[2],c->rX[3],c->rX[4],c->rX[5]};
            for (i=0; i<=adds; ++i)
            {
                unsigned char tmp = ax[9];
                for (j=9; j>0; --j) ax[j] = ax[j-1];
                ax[j] = tmp;
            }
            c->rA[1]=ax[0]; c->rA[2]=ax[1]; c->rA[3]=ax[2]; c->rA[4]=ax[3]; c->rA[5]=ax[4];
            c->rX[1]=ax[5]; c->rX[2]=ax[6]; c->rX[3]=ax[7]; c->rX[4]=ax[8]; c->rX[5]=ax[9];
            break;
        }
        default: break;
    }
    incRJ (c);
    setIP (c);
    return ;
}
//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::://
void move (struct MIX* c, const unsigned char miw[6])
{
    long long adds = makeaddss (c, miw);
    if (adds<0) adds = -adds;
    unsigned index = c->rI[0][1]; index = index << 8; index += c->rI[0][2];
    if (adds>3999 || index>3999) { incRJ (c); setIP(c); return; }
    for (int i=0; i<miw[3]; ++i)
    {
        if (adds>4000 || index>4000) break;
        c->vram[index][0] = c->vram[adds][0];   c->vram[index][1] = c->vram[adds][1];
        c->vram[index][2] = c->vram[adds][2];   c->vram[index][3] = c->vram[adds][3];
        c->vram[index][4] = c->vram[adds][4];   c->vram[index][5] = c->vram[adds][5];
        ++adds; ++index;
    }
    c->rI[0][1] = index >> 8;
    c->rI[0][2] = index - (index >> 8 << 8);
    incRJ (c);
    setIP (c);
    return ;
}