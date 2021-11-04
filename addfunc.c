//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::://
long long makeaddss (struct MIX *cpuunit, const unsigned char miw[6]) // возвращает содержимое полей
{// адреса в переданном слове
    long long adds = miw[1];
    adds = adds << 8; adds += miw[2];
    if (miw[0]) adds = -adds;
    if (miw[3])
    {
        int i = miw[3]-1;
        long long index = cpuunit->rI[i][1];
        index = index<<8; index += cpuunit->rI[i][2];
        if (cpuunit->rI[i][0]) index = -index;
        //printf ("INDEX DEBUG\n%i\n[%2X][%2X]\n%X\n%X\n",i,cpuunit->rI[i][1], cpuunit->rI[i][2], index,adds);
        adds += index; //printf ("%X\n\n",adds);
    }
    return adds;
}
//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::://
void inverse (struct MIX *cpuunit, unsigned char miw[6]) // изменение знака в слове памяти, адрес
{// которого записан в переданной инструкции
    long long adds = makeaddss (cpuunit, miw);
    if (adds<0) adds = -adds;
    if (adds>3999) return ;
    if (cpuunit->vram[adds][0]) cpuunit->vram[adds][0] = 0;
    else                        cpuunit->vram[adds][0] = 1;
    return ;
}
//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::://
void incRJ (struct MIX *cpuunit) // приращение регистра rJ
{
    unsigned tmpadds = cpuunit->rJ[1];
    tmpadds = tmpadds << 8;
    tmpadds += cpuunit->rJ[2] + 1;
    cpuunit->rJ[1] = tmpadds >> 8;
    cpuunit->rJ[2] = tmpadds - (tmpadds>>8<<8);
    return ;
}
//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::://
void setRJ (struct MIX *cpuunit, unsigned short newval) // установка нового значения регистра rJ
{
    cpuunit->rJ[1] = newval >> 8;cpuunit->rJ[2] = newval- (newval>>8<<8);
}
//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::://
void zeromemory (struct MIX* cpuunit) // обнуляет всё и вся в процессоре
{
    cpuunit->rA[0] = cpuunit->rA[1] = cpuunit->rA[2] = cpuunit->rA[3] = cpuunit->rA[4] = 
    cpuunit->rA[5] = cpuunit->rX[0] = cpuunit->rX[1] = cpuunit->rX[2] = cpuunit->rX[3] = 
    cpuunit->rX[4] = cpuunit->rX[5] = 
    cpuunit->rI[0][0] = cpuunit->rI[0][1] = cpuunit->rI[0][2] =
    cpuunit->rI[1][0] = cpuunit->rI[1][1] = cpuunit->rI[1][2] =
    cpuunit->rI[2][0] = cpuunit->rI[2][1] = cpuunit->rI[2][2] =
    cpuunit->rI[3][0] = cpuunit->rI[3][1] = cpuunit->rI[3][2] =
    cpuunit->rI[4][0] = cpuunit->rI[4][1] = cpuunit->rI[4][2] =
    cpuunit->rI[5][0] = cpuunit->rI[5][1] = cpuunit->rI[5][2] =
    cpuunit->rJ[0] = cpuunit->rJ[1] = cpuunit->rJ[2] =
    cpuunit->overflow = cpuunit->compare = 0;
    cpuunit->ip = 0;
    for (int i = 0; i<4000; ++i)
        cpuunit->vram[i][0] = cpuunit->vram[i][1] = cpuunit->vram[i][2] = 
        cpuunit->vram[i][3] = cpuunit->vram[i][4] = cpuunit->vram[i][5] = 0;
    return ;
}
//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::://
void setIP (struct MIX* cpuunit) // устанавливает значение instruction pointer равным содержимому
{// полей регистра rJ
    cpuunit->ip = (((unsigned short)cpuunit->rJ[1])<<8) + cpuunit->rJ[2];
}
//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::://
void loader (struct MIX* c) // цель - создать в virtual RAM некую тестовую программу, дабы прогнать
{// её и поиздеваться над виртуальной машиной
    int a = 10;
    c->vram[a][5] = AOPA;   c->vram[a][4] = 2;      c->vram[a][3] = 0;  // [10] ENTA +1234h
    c->vram[a][2] = 0x34;   c->vram[a][1] = 0x12;   c->vram[a][0] = 0;  // F = 4
    ++a;////////////////////////////////////////////////////////////////////////////////////////////
    c->vram[a][5] = AOPA;   c->vram[a][4] = 0;      c->vram[a][3] = 0;  // [11] INCA -1200h
    c->vram[a][2] = 0x00;   c->vram[a][1] = 0x12;   c->vram[a][0] = 1;  // F = 0
    ++a;////////////////////////////////////////////////////////////////////////////////////////////
    c->vram[a][5] = STA;    c->vram[a][4] = 5;      c->vram[a][3] = 0;  // [12] STA  +0005h
    c->vram[a][2] = 0x05;   c->vram[a][1] = 0x00;   c->vram[a][0] = 0;  // F = 5
    ++a;////////////////////////////////////////////////////////////////////////////////////////////
    c->vram[a][5] = LDX;    c->vram[a][4] = 5;      c->vram[a][3] = 0;  // [13] LDX  +0005h
    c->vram[a][2] = 0x05;   c->vram[a][1] = 0x00;   c->vram[a][0] = 0;  // F = 5
    ++a;////////////////////////////////////////////////////////////////////////////////////////////
    c->vram[a][5] = LD1;    c->vram[a][4] = 5;      c->vram[a][3] = 0;  // [14] LD1  +0005h
    c->vram[a][2] = 0x05;   c->vram[a][1] = 0x00;   c->vram[a][0] = 0;  // F = 5
    ++a;////////////////////////////////////////////////////////////////////////////////////////////
    c->vram[a][5] = LD2N;   c->vram[a][4] = 5;      c->vram[a][3] = 0;  // [15] LD2N +0005h
    c->vram[a][2] = 0x05;   c->vram[a][1] = 0x00;   c->vram[a][0] = 0;  // F = 5
    ++a;////////////////////////////////////////////////////////////////////////////////////////////
    c->vram[a][5] = ADD;    c->vram[a][4] = 5;      c->vram[a][3] = 0;  // [16] ADD  +0005h
    c->vram[a][2] = 0x05;   c->vram[a][1] = 0x00;   c->vram[a][0] = 0;  // F = 5
    ++a;////////////////////////////////////////////////////////////////////////////////////////////
    c->vram[a][5] = AOP3;   c->vram[a][4] = 3;      c->vram[a][3] = 0;  // [17] ENT3 +0200h
    c->vram[a][2] = 0x02;   c->vram[a][1] = 0x00;   c->vram[a][0] = 0;  // F = 3
    ++a;////////////////////////////////////////////////////////////////////////////////////////////
    c->vram[a][5] = ST3;    c->vram[a][4] = 5;      c->vram[a][3] = 0;  // [18] ST3  +0105h
    c->vram[a][2] = 0x05;   c->vram[a][1] = 0x01;   c->vram[a][0] = 0;  // F = 5
    ++a;////////////////////////////////////////////////////////////////////////////////////////////
    c->vram[a][5] = MUL;    c->vram[a][4] = 0x2D;   c->vram[a][3] = 0;  // [19] MUL  +0005h(5:5)!!!!
    c->vram[a][2] = 0x05;   c->vram[a][1] = 0x01;   c->vram[a][0] = 0;  // F = 2Dh (45dec)
    ++a;////////////////////////////////////////////////////////////////////////////////////////////
    c->vram[a][5] = JMP;    c->vram[a][4] = 1;      c->vram[a][3] = 4;  // [20] JSJ  +0100h,4
    c->vram[a][2] = 0x00;   c->vram[a][1] = 0x01;   c->vram[a][0] = 0;  // F = 1, I = 4 (r4)
    a = 0x101;//////////////////////////////////////////////////////////////////////////////////////
    c->vram[a][5] = HLT;    c->vram[a][4] = 0;      c->vram[a][3] = 0;  // [257] HLT
    c->vram[a][2] = 0x00;   c->vram[a][1] = 0x00;   c->vram[a][0] = 0;  // 
    a=21;///////////////////////////////////////////////////////////////////////////////////////////
    c->vram[a][5] = AOP4;   c->vram[a][4] = 0;      c->vram[a][3] = 0;  // [21] INC4  +0001h
    c->vram[a][2] = 0x01;   c->vram[a][1] = 0x00;   c->vram[a][0] = 0;  // F = 0
    ++a;////////////////////////////////////////////////////////////////////////////////////////////
    c->vram[a][5] = CMPA;   c->vram[a][4] = 0;      c->vram[a][3] = 0;  // [22] CMPA  +0005h
    c->vram[a][2] = 0x05;   c->vram[a][1] = 0x00;   c->vram[a][0] = 0;  // F = 5
    ++a;////////////////////////////////////////////////////////////////////////////////////////////
    c->vram[a][5] = SHFT;   c->vram[a][4] = SLAX;   c->vram[a][3] = 0;  // [23] SLAX  +0001h
    c->vram[a][2] = 0x01;   c->vram[a][1] = 0x00;   c->vram[a][0] = 0;  // F = 2
    ++a;////////////////////////////////////////////////////////////////////////////////////////////
    c->vram[a][5] = SHFT;   c->vram[a][4] = SLAX;   c->vram[a][3] = 0;  // [24] SLAX  +0001h
    c->vram[a][2] = 0x01;   c->vram[a][1] = 0x00;   c->vram[a][0] = 0;  // F = 2
    ++a;////////////////////////////////////////////////////////////////////////////////////////////
    c->vram[a][5] = SHFT;   c->vram[a][4] = SLAX;   c->vram[a][3] = 0;  // [25] SLAX  +0001h
    c->vram[a][2] = 0x01;   c->vram[a][1] = 0x00;   c->vram[a][0] = 0;  // F = 2
    ++a;////////////////////////////////////////////////////////////////////////////////////////////
    c->vram[a][5] = SHFT;   c->vram[a][4] = SLAX;   c->vram[a][3] = 0;  // [26] SLAX  +0001h
    c->vram[a][2] = 0x01;   c->vram[a][1] = 0x00;   c->vram[a][0] = 0;  // F = 2
    ++a;////////////////////////////////////////////////////////////////////////////////////////////
    c->vram[a][5] = SHFT;   c->vram[a][4] = SLAX;   c->vram[a][3] = 0;  // [27] SLAX  +0001h
    c->vram[a][2] = 0x01;   c->vram[a][1] = 0x00;   c->vram[a][0] = 0;  // F = 2
    ++a;////////////////////////////////////////////////////////////////////////////////////////////
    c->vram[a][5] = JMP;    c->vram[a][4] = 0;      c->vram[a][3] = 0;  // [28] JMP  +000Ah
    c->vram[a][2] = 0x0A;   c->vram[a][1] = 0x00;   c->vram[a][0] = 0;  // F = 0
    return;
}
//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::://
void memorydump (struct MIX* c) // создаёт файл дампа памяти
{
    FILE *f = fopen ("memory.dmp","w");
    for (int i=0; i<4000; ++i)
    {
        if (c->vram[i][0])  fprintf (f,"[%4i]     [-]",i);
        else                fprintf (f,"[%4i]     [+]",i);
        fprintf (f," [%2X] [%2X] [%2X] [%2X] [%2X]\n",c->vram[i][1],c->vram[i][2],c->vram[i][3],c->vram[i][4],c->vram[i][5]);
    }
    fclose (f);
}