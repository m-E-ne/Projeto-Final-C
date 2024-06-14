#include <stdio.h> // Inclusão de biblioteca padrão de entrada e saída
#include <stdlib.h> // Inclusão de biblioteca padrão de funções
#include <string.h> // Inclusão de biblioteca para manipulação de strings
#include <stdbool.h> // Inclusão de biblioteca para uso de variáveis booleanas
#include <ctype.h> // Inclusão de biblioteca para manipulação de caracteres

// Definições e estruturas para o sistema de login
#define LOGIN_MAX 64
#define SENHA_MAX 32
#define MAX_TENTATIVAS 3

// Definição da estrutura para armazenar informações de usuário
typedef struct {
    char login[LOGIN_MAX];
    char senha[SENHA_MAX];
} Usuario;

// Funções para manipulação de senha, dependendo do sistema operacional
#if defined(_WIN32) || defined(_WIN64)
#include <conio.h>

// Função para receber a senha do usuário sem mostrar os caracteres digitados
void getSenha(char *senha) {
    int i = 0;
    char ch;
    while ((ch = getch()) != '\r' && i < SENHA_MAX - 1) {
        if (ch == '\b' && i > 0) { // Backspace
            printf("\b \b");
            i--;
        } else if (ch != '\b') {
            senha[i++] = ch;
            printf("*");
        }
    }
    senha[i] = '\0';
    printf("\n");
}

// Unix
#else
#include <termios.h>
#include <unistd.h>

// Função para receber a senha do usuário sem mostrar os caracteres digitados
void getSenha(char *senha) {
    struct termios oldt, newt;
    int ch;
    int i = 0;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    while ((ch = getchar()) != '\n' && i < SENHA_MAX - 1) {
        senha[i++] = ch;
    }
    senha[i] = '\0';

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    printf("\n");
}
#endif

// Função para cadastrar um novo usuário
void cadastrar() {
    
    FILE *arquivo;
    Usuario usuario, comp;
    bool existe = false;
    limparTela();
    printf("Cadastro de usuario:\n");
    printf("Usuario: ");
    scanf("%63s", usuario.login);
    printf("Senha: ");
    getSenha(usuario.senha);

    arquivo = fopen("usuarios.bin", "rb+");
    if (!arquivo) {
        arquivo = fopen("usuarios.bin", "wb");
    } else {
        // Verifica se o usuário já existe no arquivo
        while(fread(&comp, sizeof(Usuario), 1, arquivo)) {
            if(strcmp(usuario.login, comp.login) == 0) {
                printf("Este usuario ja esta cadastrado! Por favor, faca login.\n");
                existe = true;
                break;
            }
        }
    }

    // Se o usuário não existe, ele é registrado no arquivo
    if (!existe) {
        fseek(arquivo, 0, SEEK_END); // Move o ponteiro para o final do arquivo para adicionar o novo usuário
        fwrite(&usuario, sizeof(Usuario), 1, arquivo);
        printf("Usuario cadastrado com sucesso!\n");
    }

    fclose(arquivo);
}

// Função para realizar o login
bool login() {
    
    FILE *arquivo;
    Usuario usuario;
    char login[LOGIN_MAX];
    char senha[SENHA_MAX];
    int tentativas = 0;

    arquivo = fopen("usuarios.bin", "rb");
    if (!arquivo) {
        printf("Nenhum usuario cadastrado, por favor cadastre pelo menos um usuario.\n");
        return false;
    }
    limparTela();
    // Loop para permitir várias tentativas de login
    while (tentativas < MAX_TENTATIVAS) {
        printf("Usuario: ");
        scanf("%63s", login);
        printf("Senha: ");
        getSenha(senha);

        rewind(arquivo);
        // Verifica se o login e senha correspondem aos registros
        while (fread(&usuario, sizeof(Usuario), 1, arquivo)) {
            if (!strcmp(login, usuario.login) && !strcmp(senha, usuario.senha)) {
                fclose(arquivo);
                printf("Bem-vindo %s\n", usuario.login);
                return true;
            }
        }

        printf("\nUsuario ou senha invalidos! Tentativas restantes: %d\n", MAX_TENTATIVAS - (tentativas + 1));
        tentativas++;
    }

    printf("Numero maximo de tentativas alcancado!\n\n");
    fclose(arquivo);
    return false;
}

#define NUM_FILMES_RECOMENDADOS 15
#define MAX_LINHA 200
#define MAX_FILMES 100 // Definição de MAX_FILMES

// Definições e estruturas para o sistema de recomendação de filmes
typedef struct {
    char title[100];
    char genres[100];
} Filme;

// Funções para recomendação de filmes
void recomendarFilmes(const char filmesFavoritos[][100], const char genresFavoritos[][100], Filme filmes[], int numFilmes, int numFilmesFavoritos);
float calcularReincidencia(const char* genresUsuario, const char* genresFilme);
bool filmeNoArquivo(const char* titleFilme, Filme filmes[], int numFilmes);
void inserirFilmesFavoritos(const Filme filmes[], int numFilmes, char filmesFavoritos[][100], char genresFavoritos[][100], int maxFilmesFavoritos, int *numFilmesFavoritos);
void buscarFilmes(const Filme filmes[], int numFilmes);
void listarFilmes(const Filme filmes[], int numFilmes);
void ondeEstou();

// Função principal
int main() {
    
    printf("\nBem-vindo ao Sistema de Indicacao de Filmes\n");
    printf("===========================================\n\n");

    int opcao;
    bool logado = false;

    // Variável para armazenar o número de filmes favoritos
    int numFilmesFavoritos = 0;

    // Loop principal do menu
    do {
        // Limpar os dados de filmes favoritos a cada execução
        printf("1. Cadastrar\n2. Login\n3. Sair\n4. Onde Estou?\n\nEscolha uma opcao: ");
        scanf("%d", &opcao);
        getchar();
        limparTela(); 
        switch (opcao) {
            case 1:
                cadastrar();
                break;
            case 2:
                logado = login();
                if (logado) {
                    printf("Voce esta logado.\n");
                }
                break;
            case 3:
                printf("Saindo do sistema...\n\n");
                return 0;

            case 4:
                ondeEstou();
                break;
            default:
                printf("Opcao invalida! Tente novamente.\n");
        }
    } while (!logado);

    if (logado) {
      
        FILE *arquivo = fopen("movies.txt", "r");
        if (arquivo == NULL) {
            printf("Erro ao abrir o arquivo.\n");
            return 1;
        }

        Filme filmes[MAX_FILMES];
        int numFilmes = 0;
        char linha[MAX_LINHA];

        // Lê os filmes do arquivo de texto
        while (fgets(linha, sizeof(linha), arquivo) && numFilmes < MAX_FILMES) {
            char *titulo = strtok(linha, ",");
            char *generos = strtok(NULL, "\n");

            if (titulo != NULL && generos != NULL) {
                strcpy(filmes[numFilmes].title, titulo);
                strcpy(filmes[numFilmes].genres, generos);
                numFilmes++;
            }
        }

        fclose(arquivo);

        const int maxFilmesFavoritos = 5;
        char filmesFavoritos[maxFilmesFavoritos][100];
        char genresFavoritos[maxFilmesFavoritos][100];
        int numFilmesFavoritos = 0;

        int escolha;
        do {
            numFilmesFavoritos = 0;
             memset(filmesFavoritos, 0, sizeof(filmesFavoritos)); // Reinicializa a array de filmes favoritos
             memset(genresFavoritos, 0, sizeof(genresFavoritos)); // Reinicializa a array de gêneros favoritos
            limparTela();
            printf("\nMenu:\n");
            printf("1. Escolher filmes favoritos\n");
            printf("2. Buscar filmes\n");
            printf("3. Listar filmes\n");
            printf("4. Sair\n");
            printf("5. Onde Estou?\n");
            printf("Digite sua escolha: ");
            scanf("%d", &escolha);
            getchar(); 

            switch(escolha) {
                case 1:
                    inserirFilmesFavoritos(filmes, numFilmes, filmesFavoritos, genresFavoritos, maxFilmesFavoritos, &numFilmesFavoritos);
                    break;
                case 2:
                    buscarFilmes(filmes, numFilmes);
                    break;
                case 3:
                    listarFilmes(filmes, numFilmes);
                    break;
                case 4:
                    printf("Encerrando o programa.\n");
                    break;
                case 5:
                    ondeEstou();
                    break;
                default:
                    printf("Escolha invalida. Por favor, digite um numero do menu.\n");
            }
        } while (escolha != 4);
    }

    return 0;
}

// Função para limpar a tela
void limparTela() {
    // Windows
    #ifdef _WIN32
        system("cls");
    // Unix-like (Linux, macOS)
    #else
        system("clear");
    #endif
}

void ondeEstou() {
    limparTela();
    printf("\nMenu Principal\n");
    printf("|\n");
    printf("|-- 1. Cadastrar\n");
    printf("|\n");
    printf("|-- 2. Login\n");
    printf("|   |\n");
    printf("|   |-- 2.1. Inserir filmes favoritos\n");
    printf("|   |\n");
    printf("|   |-- 2.2. Buscar filmes\n");
    printf("|   |\n");
    printf("|   |-- 2.3. Listar filmes\n");
    printf("|   |\n");
    printf("|   |-- 2.4. Onde Estou?\n");
    printf("|   |-- 2.5. Sair\n");
    printf("|-- 3. Onde Estou?\n");
    printf("|-- 4. Sair\n");
    printf("\nPressione ENTER para continuar...\n");
    getchar();
}

// Função para inserir filmes favoritos do usuário
void inserirFilmesFavoritos(const Filme filmes[], int numFilmes, char filmesFavoritos[][100], char genresFavoritos[][100], int maxFilmesFavoritos, int *numFilmesFavoritos) {
    limparTela();
    printf("Digite seus filmes favoritos (ate %d filmes)[Presentes na base de dados]:\n", maxFilmesFavoritos);
    for (int i = 0; i < maxFilmesFavoritos; i++) {
        printf("Filme %d: ", i + 1);
        fgets(filmesFavoritos[i], sizeof(filmesFavoritos[i]), stdin);
        for (int j = 0; filmesFavoritos[i][j] != '\0'; j++) {
            filmesFavoritos[i][j] = tolower(filmesFavoritos[i][j]);
        }
        filmesFavoritos[i][strcspn(filmesFavoritos[i], "\n")] = '\0';
        if (strlen(filmesFavoritos[i]) == 0) {
            printf("Por favor, insira o nome de um filme.\n");
            i--; 
            continue;
        }
        if (filmeNoArquivo(filmesFavoritos[i], filmes, numFilmes)) {
            // Verifica se o filme já está na lista de favoritos
            bool filmeJaInserido = false;
            for (int j = 0; j < *numFilmesFavoritos; j++) {
                if (strcmp(filmesFavoritos[i], filmesFavoritos[j]) == 0) {
                    filmeJaInserido = true;
                    break;
                }
            }
            if (!filmeJaInserido) {
                for (int j = 0; j < numFilmes; j++) {
                    if (strcmp(filmes[j].title, filmesFavoritos[i]) == 0) {
                        strcpy(genresFavoritos[*numFilmesFavoritos], filmes[j].genres);
                        (*numFilmesFavoritos)++;
                        break;
                    }
                }
                if (*numFilmesFavoritos >= maxFilmesFavoritos) {
                    printf("Voce atingiu o numero maximo de filmes favoritos (%d).\n", maxFilmesFavoritos);
                    break;
                }
            } else {
                printf("Este filme ja foi inserido na lista de favoritos. Insira outro filme.\n");
                i--; 
            }
        } else {
            printf("Filme nao encontrado na base de dados. Por favor, insira um filme valido.\n");
            i--; 
        }
    }

    recomendarFilmes(filmesFavoritos, genresFavoritos, filmes, numFilmes, *numFilmesFavoritos);
}

// Função para buscar filmes por título ou gênero
void buscarFilmes(const Filme filmes[], int numFilmes) {
    char termoBusca[100];
    limparTela();
    printf("Digite um termo de busca (titulo ou genero, ignorando acentuacao): ");
    fgets(termoBusca, sizeof(termoBusca), stdin);
    for (int i = 0; termoBusca[i] != '\0'; i++) {
        termoBusca[i] = tolower(termoBusca[i]);
    }
    termoBusca[strcspn(termoBusca, "\n")] = '\0'; 
    
    bool encontrado = false;
    printf("Resultados da busca:\n");
    for (int i = 0; i < numFilmes; i++) {
        if (strstr(filmes[i].title, termoBusca) != NULL || strstr(filmes[i].genres, termoBusca) != NULL) {
            printf("%s\n", filmes[i].title);
            encontrado = true;
        }
    }
    if (!encontrado) {
        printf("Nenhum filme encontrado correspondente ao termo de busca.\n");
    }
    printf("Pressione ENTER para continuar...\n");
    getchar();
}

// Função para listar todos os filmes
void listarFilmes(const Filme filmes[], int numFilmes) {
    limparTela();
    printf("Lista de filmes disponiveis:\n");
    for (int i = 0; i < numFilmes; i++) {
        printf("%s\n", filmes[i].title);
        if ((i + 1) % 25 == 0) {
            printf("Pressione ENTER para continuar...\n");
            getchar();
            limparTela();
        }
    }
}

// Função para calcular a reincidência de gêneros entre o usuário e um filme
float calcularReincidencia(const char* genresUsuario, const char* genresFilme) {
    int coincidencias = 0;
    int totalGenerosUsuario = 1;
    for (int i = 0; genresUsuario[i] != '\0'; i++) {
        if (genresUsuario[i] == ',') totalGenerosUsuario++;
    }

    char generosCopia[100];
    strcpy(generosCopia, genresFilme);

    char *token = strtok(generosCopia, ",");
    while (token != NULL) {
        if (strstr(genresUsuario, token) != NULL) {
            coincidencias++;
        }
        token = strtok(NULL, ",");
    }

    return (coincidencias / (float)totalGenerosUsuario) * 100.0;
}

// Função para verificar se o filme está no arquivo
bool filmeNoArquivo(const char* titleFilme, Filme filmes[], int numFilmes) {
    for (int i = 0; i < numFilmes; i++) {
        if (strcmp(filmes[i].title, titleFilme) == 0) {
            return true;
        }
    }
    return false;
}

// Função para recomendar filmes
void recomendarFilmes(const char filmesFavoritos[][100],const char genresFavoritos[][100], Filme filmes[], int numFilmes, int numFilmesFavoritos) {
    float reincidencias[numFilmes];
    for (int i = 0; i < numFilmes; i++) {
        reincidencias[i] = 0.0;
        for (int j = 0; j < numFilmesFavoritos; j++) {
            reincidencias[i] += calcularReincidencia(genresFavoritos[j], filmes[i].genres);
        }
    }

    // Ordenar os filmes pela reincidência (simples bubble sort)
    for (int i = 0; i < numFilmes - 1; i++) {
        for (int j = i + 1; j < numFilmes; j++) {
            if (reincidencias[i] < reincidencias[j]) {
                // Trocar reincidências
                float tempReincidencia = reincidencias[i];
                reincidencias[i] = reincidencias[j];
                reincidencias[j] = tempReincidencia;

                // Trocar filmes
                Filme tempFilme = filmes[i];
                filmes[i] = filmes[j];
                filmes[j] = tempFilme;
            }
        }
    }

    
    printf("Top filmes recomendados:\n");
    for (int i = 0; i < NUM_FILMES_RECOMENDADOS && i < numFilmes; i++) {
        // Verifica se o filme já foi inserido na lista de favoritos
        bool filmeInserido = false;
        for (int j = 0; j < numFilmesFavoritos; j++) {
            if (strcmp(filmesFavoritos[j], filmes[i].title) == 0) {
                filmeInserido = true;
                break;
            }
        }

        // Se o filme não estiver na lista de favoritos, ele é recomendado
        if (!filmeInserido) {
            printf("%s \n", filmes[i].title);
        }
    }
    printf("Pressione ENTER para continuar...\n");
    getchar();
}

