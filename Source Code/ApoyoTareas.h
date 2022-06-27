/*------------------------------------------------------------------------------
Proyecto Shell para Linux
Prototipos de funciones, macros y estructuras de datos para
el módulo ApoyoTareas

Sistemas Operativos
Grados I. Informatica, Computadores & Software
Dept. Arquitectura de Computadores - UMA

Parte de este código ha sido adaptado del libro
"Fundamentos de Sistemas Operativos", de Silberschatz y cols.
------------------------------------------------------------------------------*/

#ifndef _JOB_CONTROL_H
#define _JOB_CONTROL_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

// -----------------------   TIPOS DE DATOS ENUMERADOS   -----------------------
enum status { SUSPENDIDO, REANUDADO, FINALIZADO };
enum ground { PRIMERPLANO, SEGUNDOPLANO, DETENIDO };
static char* status_strings[] = { "Suspendido", "Reanudado", "Finalizado" };
static char* ground_strings[] = { "Primer Plano", "Segundo Plano", "Detenido" };

// -----------------   TIPO DE TAREAS PARA LA LISTA DE TAREAS   ----------------
typedef struct job_
{
  pid_t pgid; // El identificador del grupo corresponde al del proceso líder
  char *command; // Nombre del programa
  enum ground ground;
  struct job_ *next; // Siguiente trabajo en la lista
} job;

// --------------------------   FUNCIONES PÚBLICAS   ---------------------------
void get_command(char inputBuffer[], int size, char *args[], int *background);
job *new_job(pid_t pid, const char *command, enum ground place);
void add_job (job * list, job * item);
int delete_job(job * list, job * item);
job *get_item_bypid  (job * list, pid_t pid);
job *get_item_bypos( job * list, int n);
enum status analyze_status(int status, int *info);

// -----   FUNCIONES PRIVADAS: ES MEJOR USARLAS DESDE LAS MACROS DE ABAJO   ----
void print_item(job *item);
void print_list(job *list, void (*print)(job *));
void terminal_signals(void (*func) (int));
void block_signal(int signal, int block);

// ----------------------------   MACROS PÚBLICAS   ----------------------------

#define list_size(list) list->pgid // Número de tareas en la lista
#define empty_list(list) !(list->pgid) // Devuelve 1 si la lista está vacía 
#define new_list(name) new_job(0,name,PRIMERPLANO) // Name debe ser const char *
#define print_job_list(list)       print_list(list, print_item)

#define restore_terminal_signals() terminal_signals(SIG_DFL)
#define ignore_terminal_signals()  terminal_signals(SIG_IGN)

#define set_terminal(pid)          tcsetpgrp (STDIN_FILENO,pid)
#define new_process_group(pid)     setpgid (pid, pid)

#define block_SIGCHLD()   	   block_signal(SIGCHLD, 1)
#define unblock_SIGCHLD() 	   block_signal(SIGCHLD, 0)

// --------------------   MACRO PARA DEPURACIÓN DE CÓDIGO   --------------------
// Para depurar el entero "i", utiliza: debug(i,%d);
// que imprimirá:  Número de línea actual, nombre de la función, nombre del 
// fichero, y también el nombre, valor y tipo de la variable
#define debug(x,fmt) fprintf(stderr,"\"%s\":%u:%s(): --> %s= " #fmt " (%s)\n", __FILE__, __LINE__, __FUNCTION__, #x, x, #fmt)

#endif

