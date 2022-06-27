/*------------------------------------------------------------------------------
Proyecto Shell de UNIX. Sistemas Operativos
Grados II. Informatica, Computadores & Software
Dept. Arquitectura de Computadores - UMA

Algunas secciones estan inspiradas en ejercicios publicados en el libro
"Fundamentos de Sistemas Operativos", Silberschatz et al.

Para compilar este programa: gcc ProyectoShell.c ApoyoTareas.c -o MiShell
Para ejecutar este programa: ./MiShell
Para salir del programa en ejecucion, pulsar Control+D
------------------------------------------------------------------------------*/

#include "ApoyoTareas.h" // Cabecera del modulo de apoyo ApoyoTareas.c
 
#define MAX_LINE 256 // 256 caracteres por linea para cada comando es suficiente
#include <string.h>  // Para comparar cadenas de cars. (a partir de la tarea 2)

job *lista; // Estructura global para almacenar las tareas del shell

// -------------------------------------------- //
//      Cuarta fase: manejador de senales       //
// -------------------------------------------- //

void manejador (int signal) 
{
	int status, info, pid_wait, i = list_size(lista);
	enum status status_res;
	job* j;
	
	// printf("SIGCHLD recibida\n");
	
	for (i; i>=1; i--) {
		j = get_item_bypos(lista, i);
		pid_wait == waitpid(j->pgid, &status, 
			WNOHANG | WUNTRACED | WCONTINUED);
		
		if (pid_wait == j->pgid)
		{
			status_res = analyze_status(status, &info);
			// printf("SIGCHLD ....");
			
			if (status_res == FINALIZADO)
			{
				delete_job(lista, j);
			} 
			else if (status_res == REANUDADO)
			{
				j->ground = SEGUNDOPLANO;
			} 
			else if (status_res == SUSPENDIDO)
			{
				j->ground = DETENIDO;
			}
			print_job_list(lista);
		}
	
	} // end for
	
	return;
}

// -------------------------------------------- //
//                     MAIN          		  //
// -------------------------------------------- //

int main(void)
{
      char inputBuffer[MAX_LINE]; 	// Bufer que alberga el comando introducido
      int background;         	// Vale 1 si el comando introducido finaliza con '&'
      char *args[MAX_LINE/2]; 	// La linea de comandos (de 256 cars.) tiene 128 argumentos como max
      
      int pid_fork, pid_wait; 	// pid para el proceso creado y esperado
      int status;             	// Estado que devuelve la funcion wait
      enum status status_res; 	// Estado procesado por analyze_status()
      int info;	       	// Informacion procesada por analyze_status()
      
      lista = new_list("tareas");     // Lista de tareas del shell
      job *nuevo; 			// Siguente job para almacenar en la lista
      
	
	// -------------------------------------------- //
	//        Primera fase: Estructra Basica        //
	// -------------------------------------------- // 
	
	ignore_terminal_signals(); // Control + C no acaba el shell y Control + Z no lo suspende

	signal(SIGCHLD, manejador); // Activamos el manejador de la senal
	
      while (1) // El programa termina cuando se pulsa Control+D dentro de get_command()
      {   		
        printf("COMANDO->");
        fflush(stdout);
        get_command(inputBuffer, MAX_LINE, args, &background); // Obtener el proximo comando
        if (args[0]==NULL) continue; // Si se introduce un comando vacio, no hacemos nada
 
 
	// -------------------------------------------- //
	//        Segunda fase: comandos internos       //
	// -------------------------------------------- //
	
	// Comando-> cd
    	if (!strncmp(args[0], "cd", MAX_LINE))
    	{
    		if (args[1] != NULL) // Si hay al menos un parametro
    		{
    			if (chdir(args[1]) == -1) // Si no existe el directorio
    			{
    				printf("ERROR. No existe el directorio '%s'\n", args[1]);
    			} else 
    			{
    				chdir(args[1]);
    			}
    		} else // Sin parametros volvemos a HOME
    		{
    			chdir(getenv("HOME"));
    		}
    		continue;		    
    		
	} // Comando-> logout
	else if (!strncmp(args[0], "logout", MAX_LINE)) { exit(0); }

		
	// -------------------------------------------- //
	//    Quinta fase: comandos en 1er y 2º plano   //
	// -------------------------------------------- //
	
	// Comando -> jobs
	else if (!strncmp(args[0], "jobs", MAX_LINE)) 
	{
		if (list_size(lista) > 0) // Si hay nodos en la lista
		{
			print_job_list(lista); // Mostramos por pantanlla
		} else 
		{
			printf("Lista de tareas completamente vacia\n");
		}
		continue;
		
	} // Comando -> fg
	else if (!strncmp(args[0], "fg", MAX_LINE)) 
	{
		job *j;
		int pos;
		
		if (args[1] == NULL) { pos = 1; } // Sin parametros
		else { pos = atoi(args[1]); } // atoi cambia un char a int
		
		j = get_item_bypos(lista, pos);
		
		if (j == NULL) // Comprueba que exista dicha pos
		{
			printf("ERROR. No se encuentra en la lista\n");
		} else
		{
			if (j->ground != PRIMERPLANO) // Si no es FG
			{ 
				j->ground = PRIMERPLANO; // Hacemos que sea FG
				set_terminal(j->pgid); // Cedemos la terminal
				
				killpg(j->pgid, SIGCONT);
				waitpid(j->pgid, &status, WUNTRACED);
				
				set_terminal(getpid()); // Devolvemos el terminal
				status_res = analyze_status(status, &info);
				
				// Actualizamos la lista segun status_res
				if (status_res == SUSPENDIDO) 
				{
					j->ground = DETENIDO; // Sincronizamos el campo ground acorde con status_res
				} else 
				{
					delete_job(lista, j); // Eliminamos de la lista si no esta suspendido
				}
				
				printf("El proceso %s en FG con pid %d se encuentra %s. Info: %d\n", args[0], pid_fork, status_strings[status_res], info);
			} else 
			{
				printf("El proceso %s con pid %d no ha estado suspendido ni en background", j->command, j->pgid);
			}
		}
		continue;
		
	} // Comando -> bg
	else if (!strncmp(args[0], "bg", MAX_LINE))
	{
		job *j;
		int pos;
		
		if (args[1] == NULL) { pos = 1; } // Sin parametros
		else { pos = atoi(args[1]); } // atoi cambia un char a int
		
		block_SIGCHLD(); 
		j = get_item_bypos(lista, pos);
		
		if (j == NULL) // Comprueba que exista dicha pos
		{
			printf("ERROR. No se encuentra en la lista\n");
		} else 
		{
			j->ground = SEGUNDOPLANO;
			printf("El proceso %s con pid %d se encuentra en BG\n", j->command, j->pgid);
			killpg(j->pgid, SIGCONT);
		}
		
		unblock_SIGCHLD();
		continue;
	}


	// -------------------------------------------- //
	//       Tercera fase: cesion del terminal      //
	// -------------------------------------------- //

    	pid_fork = fork(); // (1) Genera un proceso hijo con fork()
    	
    	if (pid_fork > 0) // (2) Padre, el Shell
    	{ 
    		new_process_group(pid_fork); // hijo en su nuevo grupo
    		
    		if (background) // (3) Hijo en BG, Padre continua
    		{
    			// Creamos el nuevo nodo de la lista de tareas en BG
    			nuevo = new_job(pid_fork, inputBuffer, SEGUNDOPLANO);
    			
    			block_SIGCHLD(); // Hacemos que la senal se quede pendiente
    			add_job(lista, nuevo); // Anadimos a la lista de tareas
    			unblock_SIGCHLD(); // Reanudamos las senales pendientes
    			
    			// (4) El Shell muestra el mensaje de estado del comando procesado 
    			printf("Comando '%s' ejecutado en BG con pid %d\n", args[0], pid_fork);
    			// (5) El bucle regresa a la funcion get_command()
    		} 
    		else // (3) Hijo en FG, Padre espera
    		{
    			enum status estado = analyze_status(status, &info);
    			
    			// (4) El Shell muestra el mensaje de estado del comando procesado 
    			printf("Comando '%s' ejecutado en FG con pid %d. Estado %s. Info: %d\n", args[0], pid_fork, status_strings[estado], info);
    			
    			set_terminal(pid_fork); // Cesion del terminal para el hijo
    			pid_wait = waitpid(pid_fork, &status, WUNTRACED); // Esperamos de forma bloqueante
    			set_terminal(getpid()); // El padre recupera el terminal
    					
    			// Comprobamos el estado de la finalizacion del hijo
    			estado = analyze_status(status, &info);
    			if (estado == FINALIZADO)
    			{
    				printf("El hijo en FG ha acabado correctamente\n");
    			} 
    			else if (estado == SUSPENDIDO) 
    			{
    				printf("El hijo en FG ha acabado suspendido\n");
    				
    				// Creamos el nuevo nodo de la lista de tareas SUSPENDIDO
    				nuevo = new_job(pid_fork, inputBuffer, DETENIDO);
    				block_SIGCHLD(); // Hacemos que la senal se quede pendiente
    				add_job(lista, nuevo); // Anadimos a la lista de tareas
    				unblock_SIGCHLD(); // Reanudamos las senales pendientes
    				
    			} 
    			else if (estado == REANUDADO) 
    			{
    				printf("El hijo en FG ha sido reanudado\n");
    			}
    			
    		}
    	} 
    	else if (pid_fork == 0) // Hijo, ejecutador del comando
    	{ 
    		new_process_group(getpid()); // grupo propio del hijo 
    		if (!background) set_terminal(getpid()); // coge el terminal si FG
    		
    		restore_terminal_signals(); // necesario antes de ejecutar el comando
    		execvp(args[0], args); //(2) El proceso hijo invocara a execvp(comando, parametros)
    		
    		// Solo se ejecuta en caso de comando erroneo
    		printf("ERROR. Comando %s no encontrado.\n", inputBuffer); 
    		exit(EXIT_FAILURE);
    	} 
    	else // Error catch
    	{
    		perror("Error catastrofico. Vuelva a intertarlo.");
    	}
    	
  } // end while
}

