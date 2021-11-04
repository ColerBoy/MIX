﻿Используемые ключи компиляции с GCC:
-m32
-std=C99
-msse2
-march=pentium4
-mtune=pentium4
-O4

файлы и содержание:
"temp.c":           главный файл исходного кода, содержит функцию main и тело виртуальной машины
"defs.h":           файл, содержащий макроопределения (в основном макросы инструкций для улучшения читабельности кода)
"instructions.c":   содержит около 500 строк кода различных функций, выполняющих имитацию работы инструкций
"addfunc.c":        содержит дополнительные функции, призванные, в основном, сократить объём кода программы в целом
"memory.dmp"        файл дампа VRAM виртуальной машины