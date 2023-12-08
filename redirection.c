#include "redirection.h"

size_t tailleTableauChar(char **tableau) {
    size_t taille = 0;
    while (tableau[taille] != NULL) {taille++;}
    return taille;
}

char** before_com(char **res){
    char** t=malloc(sizeof res);
    for(size_t i=0;i<tailleTableauChar(res);i++){
        if(strcmp(res[i],"<")==0 ||
            strcmp(res[i],">")==0 ||
            strcmp(res[i],">|")==0 ||
            strcmp(res[i],">>")==0 ||
            strcmp(res[i],"2>")==0 ||
            strcmp(res[i],"2>|")==0 ||
            strcmp(res[i],"2>>")==0){
                break;
            }
        t[i]=res[i];
    }
    free(res);
    return t;
}

bool verif_fic(const char *fic){
    struct stat file_stat;
    return stat(fic, &file_stat) == 0 && S_ISREG(file_stat.st_mode);
}

void command_r(char** res){
    if(strcmp(res[0], "pwd") == 0 || strncmp(res[0], "cd", 2) == 0 || strcmp(res[0], "?") == 0 ||  strncmp(res[0], "exit",4) == 0) {
        // Les pipes utiles pour les redirection 
        int pipefd[2];
        pid_t child_pid;
       
        if (pipe(pipefd) != 0) { perror("Erreur au pipe"); exit(EXIT_FAILURE);}

        // Creation du fils pour executer la commande à gauche 
        child_pid = fork();

        if (child_pid < 0) {
            perror("Erreur fork");
            exit(EXIT_FAILURE);
        } 
        
        if (child_pid == 0)    {  // Code du processus enfant
        redirect(res,pipefd,child_pid); 
  
        // Exécuter la commande interne dans le processus enfant
        execute_internal_command(res[0]);
        exit(EXIT_SUCCESS);
        }
        else {  // Code du processus parent
            close(pipefd[1]);  // Fermer la fin inutilisée du pipe en écriture 

            // Attendre la fin de l'exécution du processus enfant
            waitpid(child_pid, NULL, 0);
            
            // Réinitialiser l'entrée standard pour permettre l'interaction utilisateur
            if (dup2(STDIN_FILENO, 0) == -1) {
                perror("Erreur lors de la réinitialisation de l'entrée standard");
                exit(EXIT_FAILURE);
            }
        }
    }
    else{
        int pipefd[2];
        pid_t child_pid;
         if (pipe(pipefd) != 0) { perror("Erreur au pipe"); exit(EXIT_FAILURE);}

        // Creation du fils pour executer la commande à gauche 
        child_pid = fork();
        if (child_pid < 0) { perror("Erreur fork");  exit(EXIT_FAILURE);}

        if (child_pid < 0) {
            perror("Erreur fork");
            exit(EXIT_FAILURE);
        } 

        if (child_pid == 0)    {  // Code du processus enfant
        redirect(res,pipefd,child_pid); 
  
        // Exécuter la commande interne dans le processus enfant
        //printf("test commande");
        char** utilisation = before_com(res);

        execute_command(utilisation[0],utilisation);
        exit(EXIT_SUCCESS);
        }
        else {  // Code du processus parent
            close(pipefd[1]);  // Fermer la fin inutilisée du pipe en écriture 

            // Attendre la fin de l'exécution du processus enfant
            waitpid(child_pid, NULL, 0);
            
            // Réinitialiser l'entrée standard pour permettre l'interaction utilisateur
            if (dup2(STDIN_FILENO, 0) == -1) {
                perror("Erreur lors de la réinitialisation de l'entrée standard");
                exit(EXIT_FAILURE);
            }
        }

    }
}

/*------------------------- GESTION PRINCIPAL DES REDIRECTIONS ---------------------------------*/

void redirect(char** res, int* pipefd, pid_t child_pid ){
        if (child_pid < 0) {
            perror("Erreur fork");
            exit(EXIT_FAILURE);
        } 
    
        close(pipefd[0]);  // Fermer la fin inutilisée du pipe

        int p = dup2(pipefd[1], STDIN_FILENO);
        if (p == -1) {
            perror("Erreur lors de la redirection de la sortie standard");
            exit(EXIT_FAILURE);
        }

        close(pipefd[1]);  // Fermer le descripteur de fichier supplémentaire

    for (size_t i = 0; i < tailleTableauChar(res) -1; i++) {
         //printf("%s %s \n",res[i],res[i+1]);
        if (strcmp(res[i], "<") == 0) {
            //printf("%s \n",res[i+1]);
            if (verif_fic(res[i + 1]) == true) {
                    
                int fd = open(res[i + 1], O_RDONLY);
                    //printf("test 2");
                if (fd < 0) {
                    perror("Erreur lors de l'ouverture du fichier");
                    exit(EXIT_FAILURE);
                }

                // Rediriger l'entrée standard vers le fichier
                int fd2 = dup2(fd, STDIN_FILENO);
                if (fd2 == -1) {
                    perror("Erreur lors de la redirection de l'entrée standard");
                    exit(EXIT_FAILURE);
                }

                close(fd);
                break;  // Fermer le descripteur de fichier supplémentaire
            } else {
                perror(strcat(res[i + 1], ": Aucun fichier ou dossier de ce type"));
                exit(EXIT_FAILURE);
            }
        }

         else if(strcmp(res[i], ">") == 0){
                 // Ouvrir ou créer le fichier en écriture seulement, en ajoutant au contenu existant
                 //printf("%s 1\n",res[i+1]);
                int fd = open(res[i+1],O_WRONLY | O_CREAT | O_EXCL |  O_APPEND, 0666);
                if (fd == -1) {
                   // printf( "Erreur numéro %d \n", errno);

                    perror("Erreur lors de l'ouverture ou de la création du fichier");
                    exit(EXIT_FAILURE);
                }

                // Rediriger la sortie erreur vers le fichier
                if (dup2(fd, STDOUT_FILENO) == -1) {
                    perror("Erreur lors de la redirection de la sortie standard");
                    exit(EXIT_FAILURE);
                }

                // Fermer le descripteur de fichier supplémentaire
                close(fd);
                break;
        }
        else if(strcmp(res[i], ">|")==0
        //  == 0 && strcmp(res[i+1], ">") != 0 )||
        //     (strcmp(res[i], ">") == 0 && strcmp(res[i+1], "|") != 0)
            ){
                //printf("%s 2\n",res[i+1]);
                // Ouvrir ou créer le fichier en écriture seulement, en ajoutant au contenu existant
                int fd = open(res[i+1], O_WRONLY | O_CREAT |O_TRUNC, 0666);

                if (fd == -1) {
                    perror("Erreur lors de l'ouverture ou de la création du fichier");
                    exit(EXIT_FAILURE);
                }

                // Rediriger la sortie standard vers le fichier
                if (dup2(fd, STDOUT_FILENO) == -1) {
                    perror("Erreur lors de la redirection de la sortie standard");
                    exit(EXIT_FAILURE);
                }

                // Fermer le descripteur de fichier supplémentaire
                close(fd);
                break;
        }
        else if(strcmp(res[i], ">>")==0){
            // Ouvrir ou créer le fichier en écriture seulement, en concaténant le contenu existant
            //printf("%s 4\n",res[i+1]);
            int fd = open(res[i+1], O_WRONLY | O_CREAT | O_APPEND, 0666);

            if (fd == -1) {
                perror("Erreur lors de l'ouverture ou de la création du fichier");
                exit(EXIT_FAILURE);
            }

            // Rediriger la sortie standard vers le fichier
            if (dup2(fd, STDOUT_FILENO) == -1) {
                perror("Erreur lors de la redirection de la sortie standard");
                exit(EXIT_FAILURE);
            }

            // Fermer le descripteur de fichier supplémentaire
            close(fd);
            break;
        }
        
        else if(strcmp(res[i], "2>") == 0){
                 // Ouvrir ou créer le fichier en écriture seulement, en ajoutant au contenu existant
                //printf("%s 6\n",res[i+1]);
                int fd = open(res[i+1], O_WRONLY | O_CREAT | O_APPEND | O_EXCL, 0666);

                if (fd == -1) {
                    perror("Erreur lors de l'ouverture ou de la création du fichier");
                    exit(EXIT_FAILURE);
                }

                // Rediriger la sortie erreur vers le fichier
                if (dup2(fd, STDERR_FILENO) == -1) {
                    perror("Erreur lors de la redirection de la sortie standard");
                    exit(EXIT_FAILURE);
                }

                // Fermer le descripteur de fichier supplémentaire
                close(fd);
                break;
        }
        
        else if(strcmp(res[i], "2>>") == 0){
            // Ouvrir ou créer le fichier en écriture seulement, en écrasant le contenu existant 
            //printf("%s 7\n",res[i+1]);
            int fd = open(res[i+1], O_WRONLY | O_CREAT | O_APPEND, 0666);

            if (fd == -1) {
                perror("Erreur lors de l'ouverture ou de la création du fichier");
                exit(EXIT_FAILURE);
            }

            // Rediriger la sortie erreur standard vers le fichier
            if (dup2(fd, STDERR_FILENO) == -1) {
                perror("Erreur lors de la redirection de la sortie standard");
                exit(EXIT_FAILURE);
            }

            // Fermer le descripteur de fichier supplémentaire
            close(fd);
            break;
        }
        
        else if(strcmp(res[i], "2>|") == 0){
            // Ouvrir ou créer le fichier en écriture seulement, en concaténant le contenu existant
            //printf("%s 8\n",res[i+1]);
            int fd = open(res[i+1], O_WRONLY | O_CREAT | O_TRUNC, 0666);

            if (fd == -1) {
                perror("Erreur lors de l'ouverture ou de la création du fichier");
                exit(EXIT_FAILURE);
            }

            // Rediriger la sortie erreur standard vers le fichier
            if (dup2(fd, STDERR_FILENO) == -1) {
                perror("Erreur lors de la redirection de la sortie standard");
                exit(EXIT_FAILURE);
            }

            // Fermer le descripteur de fichier supplémentaire
            close(fd);
            break;
        }
    }
}
