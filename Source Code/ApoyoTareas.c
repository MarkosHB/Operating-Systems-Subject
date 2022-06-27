/*------------------------------------------------------------------------------
Proyecto Shell para Linux
Módulo ApoyoTareas

Sistemas Operativos
Grados I. Informatica, Computadores & Software
Dept. Arquitectura de Computadores - UMA

Parte de este código ha sido adaptado del libro
"Fundamentos de Sistemas Operativos", de Silberschatz y cols.
------------------------------------------------------------------------------*/

#include "ApoyoTareas.h"
#ifndef __APPLE__
#include <malloc.h>
#endif
#include <string.h>

// -----------------------------------------------------------------------------
// get_command() lee de la línea de comandos, organizando el texto introducido
// en tokens, args[0], args[1], ...,  que se corresponden consecutivamente con
// las palabras tecleadas utilizando el espacio en blanco como delimitador.
// setup() coloca en el parám. "args" una cadena de caracteres acabada en NULL.
// -----------------------------------------------------------------------------
void get_command(char inputBuffer[], int size, char *args[],int *background)
{
  int length, // número de caracteres en la línea de comandos
      i,      // índice del bucle para acceder al vector "inputBuffer"
      start,  // índice al comienzo del siguiente parámetro del comando
      ct;     // índice a la posición donde colocar el próx. parámetro en args[]

  ct = 0;
  *background=0;

  /* Lee lo que el usuario introduce en la línea de comandos */
  length = read(STDIN_FILENO, inputBuffer, size);  

  start = -1;
  if (length == 0)
  {
    printf("\nSaliendo del Shell\n");
    exit(0); // Se pulsó ^D, aquí finaliza la entrada de comandos
  } 
  if (length < 0)
  {
    perror("Error leyendo el comando");
    exit(-1); // Finaliza con el código de error -1
  }

  /* Examinar cada carácter de "inputBuffer" */
  for (i=0; i<length; i++) 
  { 
    switch (inputBuffer[i])
    {
    case ' ':
    case '\t': // Carácter separador de argumentos: espacio o tabulador
      if (start != -1)
      {
        args[ct] = &inputBuffer[start]; // Habilita el puntero
        ct++;
      }
      inputBuffer[i] = '\0'; // Incorpora un carácter NULL para
      start = -1;            // construir una cadena de caracteres
    break;
    case '\n': // Éste debería ser el último carácter examinado
      if (start != -1)
      {
        args[ct] = &inputBuffer[start];     
        ct++;
      }
      inputBuffer[i] = '\0';
      args[ct] = NULL; // El comando actual no tiene más argumentos
      break;
    default: // Cualquier otro carácter
      if (inputBuffer[i] == '&') // Indicador de proceso en segundo plano
      {
        *background = 1;
        if (start != -1)
        {
          args[ct] = &inputBuffer[start];     
          ct++;
        }
        inputBuffer[i] = '\0';
        args[ct] = NULL; // No hay más argumentos para este comando
        i = length; // Asegura que el bucle "for" termina aquí
      }
      else if (start == -1) start = i; // Empieza otro argumento
    }  // end switch
  }  // end for   
  args[ct] = NULL; // Sólo en caso de que la línea de entrada supere MAXLINE
} 

// -----------------------------------------------------------------------------
// new_job() devuelve un puntero a un nodo que alberga el identificador, comando 
// y estado que se facilitan respectivamente en los tres argumentos de entrada.
// Devuelve NULL si no pudo realizarse la reserva de memoria
// -----------------------------------------------------------------------------
job *new_job(pid_t pid, const char *command, enum ground place)
{
  job *aux;
  aux = (job *) malloc(sizeof(job));
  aux->pgid = pid;
  aux->command = strdup(command);
  aux->ground = place;
  aux->next = NULL;
  return aux;
}

// -----------------------------------------------------------------------------
// add_job() inserta elemento al principio de la lista "list"
// -----------------------------------------------------------------------------
void add_job(job *list, job *item)
{
  job * aux = list->next;
  list->next = item;
  item->next = aux;
  list->pgid++;
}

// -----------------------------------------------------------------------------
// delete_job() elimina de la lista "list" el elemento "item"
// -----------------------------------------------------------------------------
int delete_job(job *list, job *item)
{
  job *aux=list;
  while ((aux->next != NULL) && (aux->next != item)) 
    aux = aux->next;
  if (aux->next)
  {
    aux->next = item->next;
    free(item->command);
    free(item);
    list->pgid--;
    return 1;
  }
  else
    return 0;
}

// -----------------------------------------------------------------------------
// get_item_bypid() busca y devuelve un elemento de "list" cuyo pid coincida 
// con "pid". Devuelve NULL si no lo encuentra
// -----------------------------------------------------------------------------
job *get_item_bypid(job *list, pid_t pid)
{
  job * aux=list;
  while ((aux->next!= NULL) && (aux->next->pgid != pid)) 
    aux = aux->next;
  return aux->next;
}

// -----------------------------------------------------------------------------
// get_item_bypos() busca y devuelve el n-ésimo elemento de la lista "list". 
// Devuelve NULL si la lista contiene menos de n elementos
// -----------------------------------------------------------------------------
job *get_item_bypos(job *list, int n)
{
  job *aux = list;
  if ((n < 1) || (n > list->pgid))
    return NULL;
  n--;
  while ((aux->next!= NULL) && n)
  { 
    aux = aux->next; 
    n--;
  }
  return aux->next;
}

// -----------------------------------------------------------------------------
// print_item() imprime una línea en el terminal con el pid, nombre y ubicación
// del comando representado por "item"
// -----------------------------------------------------------------------------
void print_item(job *item)
{
  printf("PID %d. Comando %s. Ubicado como %s.\n", 
         item->pgid, item->command, ground_strings[item->ground]);
}

// -----------------------------------------------------------------------------
// print_list() recorre la lista de trabajos y los va mostrando en pantalla
// -----------------------------------------------------------------------------
void print_list(job *list, void (*print)(job *))
{
  int n = 1;
  job *aux = list;
  printf("Contenidos de %s:\n", list->command);
  while(aux->next!= NULL) 
  {
    printf(" [%d] ", n);
    print(aux->next);
    n++;
    aux = aux->next;
  }
}

// -----------------------------------------------------------------------------
// analyze_status() interpreta el estatus que devuelve wait para proporcionar la
// causa de la finalización de un comando e info adicional asociada a dicho fin
// -----------------------------------------------------------------------------
enum status analyze_status(int status, int *info)
{
  if (WIFSTOPPED(status))
  {
    *info = WSTOPSIG(status);
    return(SUSPENDIDO);
  }
  else
  {
    // el proceso termio
    if (WIFSIGNALED(status))
    { 
      *info = WTERMSIG(status); 
      return(REANUDADO);
    }
    else
    { 
      *info = WEXITSTATUS(status); 
      return(FINALIZADO);
    }
  }
}

// -----------------------------------------------------------------------------
// terminal_signals() cambia la acción de las señales ligadas al terminal
// -----------------------------------------------------------------------------
void terminal_signals(void (*func) (int))
{
  signal (SIGINT,  func); // Se ha tecleado CTRL+C (interrupt - interrumpir)
  signal (SIGQUIT, func); // Se ha tecleado CTRL+\ (quit - acabar)
  signal (SIGTSTP, func); // Se ha tecleado CTRL+Z (stop - detener)
  signal (SIGTTIN, func); // El proceso en segundo plano quiere leer del terminal 
  signal (SIGTTOU, func); // El proceso en segundo plano quiere escribir en terminal
}		

// -----------------------------------------------------------------------------
// block_signal() bloquea/desbloquea una señal según sea 1/0 el último argumento
// -----------------------------------------------------------------------------
void block_signal(int signal, int block)
{
  /* Declara e inicializa la máscara  de señales */
  sigset_t block_sigchld;
  sigemptyset(&block_sigchld);
  sigaddset(&block_sigchld, signal);
  if (block)
    sigprocmask(SIG_BLOCK, &block_sigchld, NULL); // Bloquea la señal
  else
    sigprocmask(SIG_UNBLOCK, &block_sigchld, NULL); // Desbloquea la señal
}

