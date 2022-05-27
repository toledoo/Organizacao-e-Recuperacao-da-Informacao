/* ==========================================================================
 * Universidade Federal de São Carlos - Campus Sorocaba
 * Disciplina: Organização de Recuperação da Informação
 * Prof. Tiago A. Almeida
 *
 * Trabalho Sobre Indexação
 *
 * RA: 743590
 * Aluno: Rafael Toledo
 * ========================================================================== */

/* Bibliotecas */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <ctype.h>
#include <string.h>

// O enum faz com que tenhamos false = 0 e true = 1.
typedef enum {false, true} bool;

/* Tamanho dos campos dos registros */
/* Campos de tamanho fixo */
#define TAM_ID_USER 12
#define TAM_CELULAR 12
#define TAM_SALDO 14
#define TAM_DATE 9
#define TAM_ID_GAME 9
#define QTD_MAX_CATEGORIAS 3

/* Campos de tamanho variável (tamanho máximo) */
#define TAM_MAX_USER 48
#define TAM_MAX_TITULO 44
#define TAM_MAX_EMPRESA 48
#define TAM_MAX_EMAIL 42
#define TAM_MAX_CATEGORIA 20

#define MAX_REGISTROS 1000
#define TAM_REGISTRO_USUARIO (TAM_ID_USER+TAM_MAX_USER+TAM_MAX_EMAIL+TAM_SALDO+TAM_CELULAR)
#define TAM_REGISTRO_JOGO 256
#define TAM_REGISTRO_COMPRA (TAM_ID_USER+TAM_DATE+TAM_ID_GAME-3)
#define TAM_ARQUIVO_USUARIO (TAM_REGISTRO_USUARIO * MAX_REGISTROS + 1)
#define TAM_ARQUIVO_JOGO (TAM_REGISTRO_JOGO * MAX_REGISTROS + 1)
#define TAM_ARQUIVO_COMPRA (TAM_REGISTRO_COMPRA * MAX_REGISTROS + 1)

#define TAM_RRN_REGISTRO 4
#define TAM_CHAVE_USUARIOS_IDX (TAM_ID_USER + TAM_RRN_REGISTRO - 1)
#define TAM_CHAVE_JOGOS_IDX (TAM_ID_GAME + TAM_RRN_REGISTRO - 1)
#define TAM_CHAVE_COMPRAS_IDX (TAM_ID_USER + TAM_ID_GAME + TAM_RRN_REGISTRO - 2)
#define TAM_CHAVE_TITULO_IDX (TAM_MAX_TITULO + TAM_ID_GAME - 2)
#define TAM_CHAVE_DATA_USER_GAME_IDX (TAM_DATE + TAM_ID_USER + TAM_ID_GAME - 3)
#define TAM_CHAVE_CATEGORIAS_SECUNDARIO_IDX (TAM_MAX_CATEGORIA - 1)
#define TAM_CHAVE_CATEGORIAS_PRIMARIO_IDX (TAM_ID_GAME - 1)

#define TAM_ARQUIVO_USUARIOS_IDX (1000 * MAX_REGISTROS + 1)
#define TAM_ARQUIVO_JOGOS_IDX (1000 * MAX_REGISTROS + 1)
#define TAM_ARQUIVO_COMPRAS_IDX (1000 * MAX_REGISTROS + 1)
#define TAM_ARQUIVO_CATEGORIAS_IDX (1000 * MAX_REGISTROS + 1)

/* Mensagens padrões */
#define SUCESSO                          "OK\n"
#define REGS_PERCORRIDOS                "Registros percorridos:"
#define AVISO_NENHUM_REGISTRO_ENCONTRADO "AVISO: Nenhum registro encontrado\n"
#define ERRO_OPCAO_INVALIDA              "ERRO: Opcao invalida\n"
#define ERRO_MEMORIA_INSUFICIENTE        "ERRO: Memoria insuficiente\n"
#define ERRO_PK_REPETIDA                 "ERRO: Ja existe um registro com a chave %s\n"
#define ERRO_REGISTRO_NAO_ENCONTRADO     "ERRO: Registro nao encontrado\n"
#define ERRO_SALDO_NAO_SUFICIENTE        "ERRO: Saldo insuficiente\n"
#define ERRO_CATEGORIA_REPETIDA          "ERRO: O jogo %s ja possui a categoria %s\n"
#define ERRO_VALOR_INVALIDO              "ERRO: Valor invalido\n"
#define ERRO_ARQUIVO_VAZIO               "ERRO: Arquivo vazio\n"
#define ERRO_NAO_IMPLEMENTADO            "ERRO: Funcao %s nao implementada\n"

/* Registro de Usuario */
typedef struct {
    char id_user[TAM_ID_USER];
    char username[TAM_MAX_USER];
    char email[TAM_MAX_EMAIL];
    char celular[TAM_CELULAR];
    double saldo;
} Usuario;

/* Registro de Jogo */
typedef struct {
    char id_game[TAM_ID_GAME];
    char titulo[TAM_MAX_TITULO];
    char desenvolvedor[TAM_MAX_EMPRESA];
    char editora[TAM_MAX_EMPRESA];
    char data_lancamento[TAM_DATE];
    double preco;
    char categorias[QTD_MAX_CATEGORIAS][TAM_MAX_CATEGORIA];
} Jogo;

/* Registro de Compra */
typedef struct {
    char id_user_dono[TAM_ID_USER];
    char id_game[TAM_ID_GAME];
    char data_compra[TAM_DATE];
} Compra;


/*----- Registros dos índices -----*/


/* Struct para o índice primário dos usuários */
typedef struct {
    char id_user[TAM_ID_USER];
    int rrn;
} usuarios_index;

/* Struct para o índice primário dos jogos */
typedef struct {
    char id_game[TAM_ID_GAME];
    int rrn;
} jogos_index;

/* Struct para índice primário dos compras */
typedef struct {
    char id_user[TAM_ID_USER];
    char id_game[TAM_ID_GAME];
    int rrn;
} compras_index;

/* Struct para o índice secundário dos titulos */
typedef struct {
    char titulo[TAM_MAX_TITULO];
    char id_game[TAM_ID_GAME];
} titulos_index;

/* Struct para o índice secundário das datas das compras */
typedef struct {
    char data[TAM_DATE];
    char id_user[TAM_ID_USER];
    char id_game[TAM_ID_GAME];
} data_user_game_index;

/* Struct para o índice secundário das categorias (lista invertida) */
typedef struct {
    char chave_secundaria[TAM_MAX_CATEGORIA];   //string com o nome da categoria
    int primeiro_indice;
} categorias_secundario_index;

/* Struct para o índice primário das categorias (lista invertida) */
typedef struct {
    char chave_primaria[TAM_ID_GAME];   //string com o id do jogo
    int proximo_indice;
} categorias_primario_index;

/* Struct para os parâmetros de uma lista invertida */
typedef struct {
    // Ponteiro para o índice secundário
    categorias_secundario_index *categorias_secundario_idx;

    // Ponteiro para o arquivo de índice primário
    categorias_primario_index *categorias_primario_idx;

    // Quantidade de registros de índice secundário
    unsigned qtd_registros_secundario;

    // Quantidade de registros de índice primário
    unsigned qtd_registros_primario;

    // Tamanho de uma chave secundária nesse índice
    unsigned tam_chave_secundaria;

    // Tamanho de uma chave primária nesse índice
    unsigned tam_chave_primaria;

    // Função utilizada para comparar as chaves do índice secundário.
    // Igual às funções de comparação do bsearch e qsort.
    int (*compar)(const void *key, const void *elem);
} inverted_list;

/* Variáveis globais */
/* Arquivos de dados */
char ARQUIVO_USUARIOS[TAM_ARQUIVO_USUARIO];
char ARQUIVO_JOGOS[TAM_ARQUIVO_JOGO];
char ARQUIVO_COMPRAS[TAM_ARQUIVO_COMPRA];

/* Índices */
usuarios_index *usuarios_idx = NULL;
jogos_index *jogos_idx = NULL;
compras_index *compras_idx = NULL;
titulos_index *titulo_idx = NULL;
data_user_game_index *data_user_game_idx = NULL;
inverted_list categorias_idx = {
    .categorias_secundario_idx = NULL,
    .categorias_primario_idx = NULL,
    .qtd_registros_secundario = 0,
    .qtd_registros_primario = 0,
    .tam_chave_secundaria = TAM_CHAVE_CATEGORIAS_SECUNDARIO_IDX,
    .tam_chave_primaria = TAM_CHAVE_CATEGORIAS_PRIMARIO_IDX,
};

/* Funções auxiliares para o qsort.
 * Com uma pequena alteração, é possível utilizá-las no bsearch, assim, evitando código duplicado.
 * */
int qsort_usuarios_idx(const void *a, const void *b);
int qsort_jogos_idx(const void *a, const void *b);
int qsort_compras_idx(const void *a, const void *b);
int qsort_titulo_idx(const void *a, const void *b);
int qsort_data_user_game_idx(const void *a, const void *b);
int qsort_data_idx(const void *a, const void *b);
int qsort_categorias_secundario_idx(const void *a, const void *b);

/* Contadores */
unsigned qtd_registros_usuarios = 0;
unsigned qtd_registros_jogos = 0;
unsigned qtd_registros_compras = 0;

/* Funções de geração determinística de números pseudo-aleatórios */
uint64_t prng_seed;

void prng_srand(uint64_t value) {
    prng_seed = value;
}

uint64_t prng_rand() {
    // https://en.wikipedia.org/wiki/Xorshift#xorshift*
    uint64_t x = prng_seed; // O estado deve ser iniciado com um valor diferente de 0
    x ^= x >> 12; // a
    x ^= x << 25; // b
    x ^= x >> 27; // c
    prng_seed = x;
    return x * UINT64_C(0x2545F4914F6CDD1D);
}

/* Funções de manipulação de data */
int64_t epoch;

void set_time(int64_t value) {
    epoch = value;
}

void tick_time() {
    epoch += prng_rand() % 864000; // 10 dias
}

struct tm gmtime_(const int64_t lcltime) {
    // based on https://sourceware.org/git/?p=newlib-cygwin.git;a=blob;f=newlib/libc/time/gmtime_r.c;
    struct tm res;
    long days = lcltime / 86400 + 719468;
    long rem = lcltime % 86400;
    if (rem < 0) {
        rem += 86400;
        --days;
    }

    res.tm_hour = (int) (rem / 3600);
    rem %= 3600;
    res.tm_min = (int) (rem / 60);
    res.tm_sec = (int) (rem % 60);

    int weekday = (3 + days) % 7;
    if (weekday < 0) weekday += 7;
    res.tm_wday = weekday;

    int era = (days >= 0 ? days : days - 146096) / 146097;
    unsigned long eraday = days - era * 146097;
    unsigned erayear = (eraday - eraday / 1460 + eraday / 36524 - eraday / 146096) / 365;
    unsigned yearday = eraday - (365 * erayear + erayear / 4 - erayear / 100);
    unsigned month = (5 * yearday + 2) / 153;
    unsigned day = yearday - (153 * month + 2) / 5 + 1;
    month += month < 10 ? 2 : -10;

    int isleap = ((erayear % 4) == 0 && (erayear % 100) != 0) || (erayear % 400) == 0;
    res.tm_yday = yearday >= 306 ? yearday - 306 : yearday + 59 + isleap;
    res.tm_year = (erayear + era * 400 + (month <= 1)) - 1900;
    res.tm_mon = month;
    res.tm_mday = day;
    res.tm_isdst = 0;

    return res;
}

/**
 * Escreve a <i>data</i> atual no formato <code>AAAAMMDD</code> em uma <i>string</i>
 * fornecida como parâmetro.<br />
 * <br />
 * Exemplo de uso:<br />
 * <code>
 * char timestamp[TAM_DATE];<br />
 * current_date(date);<br />
 * printf("data atual: %s&#92;n", date);<br />
 * </code>
 *
 * @param buffer String de tamanho <code>TAM_DATE</code> no qual será escrita
 * a <i>timestamp</i>. É terminado pelo caractere <code>\0</code>.
 */
void current_date(char buffer[TAM_DATE]) {
    // http://www.cplusplus.com/reference/ctime/strftime/
    // http://www.cplusplus.com/reference/ctime/gmtime/
    // AAAA MM DD
    // %Y   %m %d
    struct tm tm_ = gmtime_(epoch);
    strftime(buffer, TAM_DATE, "%Y%m%d", &tm_);
}

/* Remove comentários (--) e caracteres whitespace do começo e fim de uma string */
void clear_input(char *str) {
    char *ptr = str;
    int len = 0;

    // O valor inicial do len vai ser o último valor que ele assumiu, ou seja, 0.
    for (; ptr[len]; ++len) {
        // O strncmp() compara os 2 primeiros caracteres da string com "--". Se forem iguais, retorna 0.
        if (strncmp(&ptr[len], "--", 2) == 0) {
            ptr[len] = '\0';
            break;
        }
    }

    while(len-1 > 0 && isspace(ptr[len-1]))
        ptr[--len] = '\0';

    while(*ptr && isspace(*ptr))
        ++ptr, --len;

    memmove(str, ptr, len + 1);
}


/* ==========================================================================
 * ========================= PROTÓTIPOS DAS FUNÇÕES =========================
 * ========================================================================== */

/* Cria o índice respectivo */
void criar_usuarios_idx();
void criar_jogos_idx();
void criar_compras_idx();
void criar_titulo_idx();
void criar_data_user_game_idx();
void criar_categorias_idx();

/* Exibe um registro com base no RRN */
bool exibir_usuario(int rrn);
bool exibir_jogo(int rrn);
bool exibir_compra(int rrn);

/* Recupera do arquivo o registro com o RRN informado
 * e retorna os dados nas structs Usuario, Jogo e Compra */
Usuario recuperar_registro_usuario(int rrn);
Jogo recuperar_registro_jogo(int rrn);
Compra recuperar_registro_compra(int rrn);

/* Escreve em seu respectivo arquivo na posição informada (RRN) */
void escrever_registro_usuario(Usuario u, int rrn);
void escrever_registro_jogo(Jogo j, int rrn);
void escrever_registro_compra(Compra c, int rrn);

/* Funções principais */
void cadastrar_usuario_menu(char* id_user, char* username, char* email);
void cadastrar_celular_menu(char* id_user, char* celular);
void remover_usuario_menu(char *id_user);
void cadastrar_jogo_menu(char* titulo, char* desenvolvedor, char* editora, char* lancamento, double preco);
void adicionar_saldo_menu(char* id_user, double valor);
void comprar_menu(char* id_user, char* titulo);
void cadastrar_categoria_menu(char* titulo, char* categoria);

/* Busca */
void buscar_usuario_id_user_menu(char *id_user);
void buscar_jogo_id_menu(char *id_game);
void buscar_jogo_titulo_menu(char *titulo);

/* Listagem */
void listar_usuarios_id_user_menu();
void listar_jogos_categorias_menu(char *categoria);
void listar_compras_periodo_menu(char *data_inicio, char *data_fim);

/* Liberar espaço */
void liberar_espaco_menu();

/* Imprimir arquivos de dados */
void imprimir_arquivo_usuarios_menu();
void imprimir_arquivo_jogos_menu();
void imprimir_arquivo_compras_menu();

/* Imprimir índices primários */
void imprimir_usuarios_idx_menu();
void imprimir_jogos_idx_menu();
void imprimir_compras_idx_menu();

/* Imprimir índices secundários */
void imprimir_titulo_idx_menu();
void imprimir_data_user_game_idx_menu();
void imprimir_categorias_secundario_idx_menu();
void imprimir_categorias_primario_idx_menu();

/* Liberar memória e encerrar programa */
void liberar_memoria_menu();

/* Funções de manipulação de Lista Invertida */
/**
 * Responsável por inserir duas chaves (chave_secundaria e chave_primaria) em uma Lista Invertida (t).<br />
 * Atualiza os parâmetros dos índices primário e secundário conforme necessário.<br />
 * As chaves a serem inseridas devem estar no formato correto e com tamanho t->tam_chave_primario e t->tam_chave_secundario.<br />
 * O funcionamento deve ser genérico para qualquer Lista Invertida, adaptando-se para os diferentes parâmetros presentes em seus structs.<br />
 *
 * @param chave_secundaria Chave a ser buscada (caso exista) ou inserida (caso não exista) no registro secundário da Lista Invertida.
 * @param chave_primaria Chave a ser inserida no registro primário da Lista Invertida.
 * @param t Ponteiro para a Lista Invertida na qual serão inseridas as chaves.
 */
void inverted_list_insert(char *chave_secundaria, char *chave_primaria, inverted_list *t);

/**
 * Responsável por buscar uma chave no índice secundário de uma Lista invertida (T). O valor de retorno indica se a chave foi encontrada ou não.
 * O ponteiro para o int result pode ser fornecido opcionalmente, e conterá o índice inicial das chaves no registro primário.<br />
 * <br />
 * Exemplos de uso:<br />
 * <code>
 * // Exemplo 1. A chave encontrada deverá ser retornada e o caminho não deve ser informado.<br />
 * ...<br />
 * int result;<br />
 * bool found = inverted_list_secondary_search(&result, false, categoria, &categorias_idx);<br />
 * ...<br />
 * <br />
 * // Exemplo 2. Não há interesse na chave encontrada, apenas se ela existe, e o caminho não deve ser informado.<br />
 * ...<br />
 * bool found = inverted_list_secondary_search(NULL, false, categoria, &categorias_idx);<br />
 * ...<br />
 * <br />
 * // Exemplo 3. Há interesse no caminho feito para encontrar a chave.<br />
 * ...<br />
 * int result;<br />
 * bool found = inverted_list_secondary_search(&result, true, categoria, &categorias_idx);<br />
 * </code>
 *
 * @param result Ponteiro para ser escrito o índice inicial (primeira ocorrência) das chaves do registro primário. É ignorado caso NULL.
 * @param exibir_caminho Indica se o caminho percorrido deve ser impresso.
 * @param chave_secundaria Chave a ser buscada na Árvore-B.
 * @param t Ponteiro para o índice do tipo Lista invertida no qual será buscada a chave.
 * @return Indica se a chave foi encontrada.
 */
bool inverted_list_secondary_search(int *result, bool exibir_caminho, char *chave_secundaria, inverted_list *t);

/**
 * Responsável por percorrer o índice primário de uma Lista invertida (T). O valor de retorno indica a quantidade de chaves encontradas.
 * O ponteiro para o vetor de strings result pode ser fornecido opcionalmente, e será populado com a lista de todas as chaves encontradas.
 * O ponteiro para o inteiro indice_final também pode ser fornecido opcionalmente, e deve conter o índice do último campo da lista encadeada 
 * da chave primaria fornecida (isso é útil na inserção de um novo registro).<br />
 * <br />
 * Exemplos de uso:<br />
 * <code>
 * // Exemplo 1. As chaves encontradas deverão ser retornadas e tanto o caminho quanto o indice_final não devem ser informados.<br />
 * ...<br />
 * char chaves[MAX_REGISTROS][TAM_CHAVE_CATEGORIAS_PRIMARIO_IDX];<br />
 * int qtd = inverted_list_primary_search(chaves, false, indice, NULL, &categorias_idx);<br />
 * ...<br />
 * <br />
 * // Exemplo 2. Não há interesse nas chaves encontradas, apenas no indice_final, e o caminho não deve ser informado.<br />
 * ...<br />
 * int indice_final;
 * int qtd = inverted_list_primary_search(NULL, false, indice, &indice_final, &categorias_idx);<br />
 * ...<br />
 * <br />
 * // Exemplo 3. Há interesse nas chaves encontradas e no caminho feito.<br />
 * ...<br />
 * char chaves[MAX_REGISTROS][TAM_CHAVE_CATEGORIAS_PRIMARIO_IDX];<br />
 * int qtd = inverted_list_primary_search(chaves, true, indice, NULL, &categorias_idx);<br />
 * ...<br />
 * <br />
 * </code>
 *
 * @param result Ponteiro para serem escritas as chaves encontradas. É ignorado caso NULL.
 * @param exibir_caminho Indica se o caminho percorrido deve ser impresso.
 * @param indice Índice do primeiro registro da lista encadeada a ser procurado.
 * @param indice_final Ponteiro para ser escrito o índice do último registro encontrado (cujo campo indice é -1). É ignorado caso NULL.
 * @param t Ponteiro para o índice do tipo Lista invertida no qual será buscada a chave.
 * @return Indica a quantidade de chaves encontradas.
 */
int inverted_list_primary_search(char result[][TAM_CHAVE_CATEGORIAS_PRIMARIO_IDX], bool exibir_caminho, int indice, int *indice_final, inverted_list *t);

/**
 * Preenche uma string str com o caractere pad para completar o tamanho size.<br />
 *
 * @param str Ponteiro para a string a ser manipulada.
 * @param pad Caractere utilizado para fazer o preenchimento à direita.
 * @param size Tamanho desejado para a string.
 */
char* strpadright(char *str, char pad, unsigned size);

/* Funções de busca binária */
/**
 * Função Genérica de busca binária, que aceita parâmetros genéricos (assinatura baseada na função bsearch da biblioteca C).
 * 
 * @param key Chave de busca genérica.
 * @param base0 Base onde ocorrerá a busca, por exemplo, um ponteiro para um vetor.
 * @param nmemb Número de elementos na base.
 * @param size Tamanho do tipo do elemento na base, dica: utilize a função sizeof().
 * @param compar Ponteiro para a função que será utilizada nas comparações.
 * @param exibir_caminho Indica se o caminho percorrido deve ser impresso.
 * @return Retorna o elemento encontrado ou NULL se não encontrou.
 */
void* busca_binaria(const void *key, const void *base0, size_t nmemb, size_t size, int (*compar)(const void *, const void *), bool exibir_caminho);

/**
 * Função Genérica de busca binária que encontra o elemento de BAIXO mais próximo da chave.
 * Sua assinatura também é baseada na função bsearch da biblioteca C.
 * 
 * @param key Chave de busca genérica.
 * @param base0 Base onde ocorrerá a busca, por exemplo, um ponteiro para um vetor.
 * @param nmemb Número de elementos na base.
 * @param size Tamanho do tipo do elemento na base, dica: utilize a função sizeof().
 * @param compar Ponteiro para a função que será utilizada nas comparações.
 * @param exibir_caminho Indica se o caminho percorrido deve ser impresso.
 * @return Retorna o elemento encontrado ou o de BAIXO mais próximo.
 */
void* busca_binaria_piso(const void* key, void* base, size_t num, size_t size, int (*compar)(const void*,const void*));

/**
 * Função Genérica de busca binária que encontra o elemento de CIMA mais próximo da chave.
 * Sua assinatura também é baseada na função bsearch da biblioteca C.
 * 
 * @param key Chave de busca genérica.
 * @param base0 Base onde ocorrerá a busca, por exemplo, um ponteiro para um vetor.
 * @param nmemb Número de elementos na base.
 * @param size Tamanho do tipo do elemento na base, dica: utilize a função sizeof().
 * @param compar Ponteiro para a função que será utilizada nas comparações.
 * @param exibir_caminho Indica se o caminho percorrido deve ser impresso.
 * @return Retorna o elemento encontrado ou o de CIMA mais próximo.
 */
void* busca_binaria_teto(const void* key, void* base, size_t num, size_t size, int (*compar)(const void*,const void*));

/* <<< COLOQUE AQUI OS DEMAIS PROTÓTIPOS DE FUNÇÕES, SE NECESSÁRIO >>> */


/* ==========================================================================
 * ============================ FUNÇÃO PRINCIPAL ============================
 * =============================== NÃO ALTERAR ============================== */

int main() {
    // variáveis utilizadas pelo interpretador de comandos
    char input[500];
    uint64_t seed = 2;
    uint64_t time = 1616077800; // UTC 18/03/2021 14:30:00
    char id_user[TAM_ID_USER];
    char username[TAM_MAX_USER];
    char email[TAM_MAX_EMAIL];
    char celular[TAM_CELULAR];
    char id[TAM_ID_GAME];
    char titulo[TAM_MAX_TITULO];
    char desenvolvedor[TAM_MAX_EMPRESA];
    char editora[TAM_MAX_EMPRESA];
    char lancamento[TAM_DATE];
    char categoria[TAM_MAX_CATEGORIA];
    double valor;
    char data_inicio[TAM_DATE];
    char data_fim[TAM_DATE];
    
    scanf("SET ARQUIVO_USUARIOS '%[^\n]\n", ARQUIVO_USUARIOS);
    int temp_len = strlen(ARQUIVO_USUARIOS);
    if (temp_len < 2) temp_len = 2; // corrige o tamanho caso a entrada seja omitida
    qtd_registros_usuarios = (temp_len - 2) / TAM_REGISTRO_USUARIO;
    ARQUIVO_USUARIOS[temp_len - 2] = '\0';

    scanf("SET ARQUIVO_JOGOS '%[^\n]\n", ARQUIVO_JOGOS);
    temp_len = strlen(ARQUIVO_JOGOS);
    if (temp_len < 2) temp_len = 2; // corrige o tamanho caso a entrada seja omitida
    qtd_registros_jogos = (temp_len - 2) / TAM_REGISTRO_JOGO;
    ARQUIVO_JOGOS[temp_len - 2] = '\0';

    scanf("SET ARQUIVO_COMPRAS '%[^\n]\n", ARQUIVO_COMPRAS);
    temp_len = strlen(ARQUIVO_COMPRAS);
    if (temp_len < 2) temp_len = 2; // corrige o tamanho caso a entrada seja omitida
    qtd_registros_compras = (temp_len - 2) / TAM_REGISTRO_COMPRA;
    ARQUIVO_COMPRAS[temp_len - 2] = '\0';

    // inicialização do gerador de números aleatórios e função de datas
    prng_srand(seed);
    set_time(time);

    criar_usuarios_idx();
    criar_jogos_idx();
    criar_compras_idx();
    criar_titulo_idx();
    criar_data_user_game_idx();
    criar_categorias_idx();

    while (1) {
        fgets(input, 500, stdin);
        printf("%s", input);
        clear_input(input);

        // O strcmp() compara "" (string vazia) com o input. Se forem iguais, retorna 0.
        if (strcmp("", input) == 0)
            continue; // não avança o tempo nem imprime o comando este seja em branco

        /* Funções principais */
        // O sscanf() tem como primeiro argumento um ponteiro para a string da onde serão lidos os dados, o resto dos argumentos são os mesmos que os de um scanf() normal.
        if (sscanf(input, "INSERT INTO usuarios VALUES ('%[^']', '%[^']', '%[^']');", id_user, username, email) == 3)
            cadastrar_usuario_menu(id_user, username, email);
        else if (sscanf(input, "UPDATE usuarios SET celular = '%[^']' WHERE id_user = '%[^']';", celular, id_user) == 2)
            cadastrar_celular_menu(id_user, celular);
        else if (sscanf(input, "DELETE FROM usuarios WHERE id_user = '%[^']';", id_user) == 1)
            remover_usuario_menu(id_user);
        else if (sscanf(input, "INSERT INTO jogos VALUES ('%[^']', '%[^']', '%[^']', '%[^']', %lf);", titulo, desenvolvedor, editora, lancamento, &valor) == 5)
            cadastrar_jogo_menu(titulo, desenvolvedor, editora, lancamento, valor);
        else if (sscanf(input, "UPDATE usuarios SET saldo = saldo + %lf WHERE id_user = '%[^']';", &valor, id_user) == 2)
            adicionar_saldo_menu(id_user, valor);
        else if (sscanf(input, "INSERT INTO compras VALUES ('%[^']', '%[^']');", id_user, titulo) == 2)
            comprar_menu(id_user, titulo);
        else if (sscanf(input, "UPDATE jogos SET categorias = array_append(categorias, '%[^']') WHERE titulo = '%[^']';", categoria, titulo) == 2)
            cadastrar_categoria_menu(titulo, categoria);

        /* Busca */
        else if (sscanf(input, "SELECT * FROM usuarios WHERE id_user = '%[^']';", id_user) == 1)
            buscar_usuario_id_user_menu(id_user);
        else if (sscanf(input, "SELECT * FROM jogos WHERE id_game = '%[^']';", id) == 1)
            buscar_jogo_id_menu(id);
        else if (sscanf(input, "SELECT * FROM jogos WHERE titulo = '%[^']';", titulo) == 1)
            buscar_jogo_titulo_menu(titulo);

        /* Listagem */
        else if (strcmp("SELECT * FROM usuarios ORDER BY id_user ASC;", input) == 0)
            listar_usuarios_id_user_menu();
        else if (sscanf(input, "SELECT * FROM jogos WHERE '%[^']' = ANY (categorias) ORDER BY titulo ASC;", categoria) == 1)
            listar_jogos_categorias_menu(categoria);
        else if (sscanf(input, "SELECT * FROM compras WHERE data_compra BETWEEN '%[^']' AND '%[^']' ORDER BY data_compra ASC;", data_inicio, data_fim) == 2)
            listar_compras_periodo_menu(data_inicio, data_fim);

        /* Liberar espaço */
        else if (strcmp("VACUUM usuarios;", input) == 0)
            liberar_espaco_menu();

        /* Imprimir arquivos de dados */
        else if (strcmp("\\echo file ARQUIVO_USUARIOS", input) == 0)
            imprimir_arquivo_usuarios_menu();
        else if (strcmp("\\echo file ARQUIVO_JOGOS", input) == 0)
            imprimir_arquivo_jogos_menu();
        else if (strcmp("\\echo file ARQUIVO_COMPRAS", input) == 0)
            imprimir_arquivo_compras_menu();
        
        /* Imprimir índices primários */
        else if (strcmp("\\echo index usuarios_idx", input) == 0)
            imprimir_usuarios_idx_menu();
        else if (strcmp("\\echo index jogos_idx", input) == 0)
            imprimir_jogos_idx_menu();
        else if (strcmp("\\echo index compras_idx", input) == 0)
            imprimir_compras_idx_menu();

        /* Imprimir índices secundários */
        else if (strcmp("\\echo index titulo_idx", input) == 0)
            imprimir_titulo_idx_menu();
        else if (strcmp("\\echo index data_user_game_idx", input) == 0)
            imprimir_data_user_game_idx_menu();
        else if (strcmp("\\echo index categorias_secundario_idx", input) == 0)
            imprimir_categorias_secundario_idx_menu();
        else if (strcmp("\\echo index categorias_primario_idx", input) == 0)
            imprimir_categorias_primario_idx_menu();

        /* Liberar memória eventualmente alocada e encerrar programa */
        else if (strcmp("\\q", input) == 0)
            { liberar_memoria_menu(); return 0; }
        else if (sscanf(input, "SET SRAND %lu;", &seed) == 1)
            { prng_srand(seed); printf(SUCESSO); continue; }
        else if (sscanf(input, "SET TIME %lu;", &time) == 1)
            { set_time(time); printf(SUCESSO); continue; }
        else
            printf(ERRO_OPCAO_INVALIDA);

        tick_time();
    }
}

/* ========================================================================== */

/* Cria o índice primário usuarios_idx */
void criar_usuarios_idx() {
    // Se usuarios_idx não existir, vou alocar memória para criá-lo.
    if (!usuarios_idx)
        usuarios_idx = malloc(MAX_REGISTROS * sizeof(usuarios_index));

    if (!usuarios_idx) {
        printf(ERRO_MEMORIA_INSUFICIENTE);
        exit(1);
    }

    for (unsigned i = 0; i < qtd_registros_usuarios; ++i) {
        Usuario u = recuperar_registro_usuario(i);

        if (strncmp(u.id_user, "*|", 2) == 0)
            usuarios_idx[i].rrn = -1; // registro excluído
        else
            usuarios_idx[i].rrn = i;

        strcpy(usuarios_idx[i].id_user, u.id_user);
    }

    qsort(usuarios_idx, qtd_registros_usuarios, sizeof(usuarios_index), qsort_usuarios_idx);
}

/* Cria o índice primário jogos_idx */
void criar_jogos_idx() {
    /* <<< MINHA IMPLEMENTAÇÃO >>> */
    if (!jogos_idx)
        jogos_idx = malloc(MAX_REGISTROS * sizeof(jogos_index));

    if (!jogos_idx) {
        printf(ERRO_MEMORIA_INSUFICIENTE);
        exit(1);
    }

    for (unsigned i = 0; i < qtd_registros_jogos; ++i) {
        Jogo j = recuperar_registro_jogo(i);

        // Já adiciono os 2 atributos do jogos_idx sem verificar, pois os índices de jogos não podem ser removidos.
        jogos_idx[i].rrn = i;
        strcpy(jogos_idx[i].id_game, j.id_game);
    }

    qsort(jogos_idx, qtd_registros_jogos, sizeof(jogos_index), qsort_jogos_idx);
}

/* Cria o índice primário compras_idx */
void criar_compras_idx() {
    /* <<< MINHA IMPLEMENTAÇÃO >>> */
    if (!compras_idx)
        compras_idx = malloc(MAX_REGISTROS * sizeof(compras_index));

    if (!compras_idx) {
        printf(ERRO_MEMORIA_INSUFICIENTE);
        exit(1);
    }

    for (unsigned i = 0; i < qtd_registros_compras; ++i) {
        Compra c = recuperar_registro_compra(i);

        // Já adiciono os 2 atributos do compras_idx sem verificar, pois os índices de compras não podem ser removidos.
        compras_idx[i].rrn = i;
        strcpy(compras_idx[i].id_user, c.id_user_dono);
        strcpy(compras_idx[i].id_game, c.id_game);
    }

    qsort(jogos_idx, qtd_registros_jogos, sizeof(jogos_index), qsort_jogos_idx);
}

/* Cria o índice secundário titulo_idx */
void criar_titulo_idx() {
    /* <<< MINHA IMPLEMENTAÇÃO >>> */
    if (!titulo_idx)
        titulo_idx = malloc(MAX_REGISTROS * sizeof(titulos_index));

    if (!titulo_idx) {
        printf(ERRO_MEMORIA_INSUFICIENTE);
        exit(1);
    }

    for (unsigned i = 0; i < qtd_registros_jogos; ++i) {
        Jogo j = recuperar_registro_jogo(i);

        // Já adiciono os 2 atributos do titulo_idx sem verificar, pois os índices de títulos não podem ser removidos.
        strcpy(titulo_idx[i].titulo, j.titulo);
        strcpy(titulo_idx[i].id_game, j.id_game);
    }

    qsort(titulo_idx, qtd_registros_jogos, sizeof(titulos_index), qsort_titulo_idx);
}

/* Cria o índice secundário data_user_game_idx */
void criar_data_user_game_idx() {
    /* <<< MINHA IMPLEMENTAÇÃO >>> */
    if (!data_user_game_idx)
        data_user_game_idx = malloc(MAX_REGISTROS * sizeof(data_user_game_index));

    if (!data_user_game_idx) {
        printf(ERRO_MEMORIA_INSUFICIENTE);
        exit(1);
    }

    for (unsigned i = 0; i < qtd_registros_compras; ++i) {
        Compra c = recuperar_registro_compra(i);

        // Já adiciono os 3 atributos do data_user_game_idx sem verificar, pois os índices de data_user_game não podem ser removidos.
        strcpy(data_user_game_idx[i].data, c.data_compra);
        strcpy(data_user_game_idx[i].id_user, c.id_user_dono);
        strcpy(data_user_game_idx[i].id_game, c.id_game);
    }

    qsort(compras_idx, qtd_registros_compras, sizeof(compras_index), qsort_data_user_game_idx);
}

/* Cria os índices (secundário e primário) de categorias_idx */
void criar_categorias_idx() {
    /* <<< MINHA IMPLEMENTAÇÃO >>> */
    if (!categorias_idx.categorias_primario_idx && !categorias_idx.categorias_secundario_idx) {
        categorias_idx.categorias_primario_idx = malloc(MAX_REGISTROS * sizeof(categorias_idx.categorias_primario_idx));
        categorias_idx.categorias_secundario_idx = malloc(MAX_REGISTROS * sizeof(categorias_idx.categorias_secundario_idx));
    }

    if (!categorias_idx.categorias_primario_idx || !categorias_idx.categorias_secundario_idx) {
        printf(ERRO_MEMORIA_INSUFICIENTE);
        exit(1);
    }

    for (unsigned i = 0; i < qtd_registros_jogos; ++i) {
        Jogo j = recuperar_registro_jogo(i);
        
        // Caso que o jogo possui 3 categorias:
        if(j.categorias[0][0] != '\0' && j.categorias[1][0] != '\0' && j.categorias[2][0] != '\0') {
            for(int numCat = 0; numCat < 3; numCat++) {
                // Atribui as 3 chaves secundárias.
                inverted_list_insert(j.categorias[numCat], j.id_game, &categorias_idx);
            }
        }
        // Caso que o jogo possui 2 categorias:
        else if(j.categorias[0][0] != '\0' && j.categorias[1][0] != '\0') {
            for(int numCat = 0; numCat < 2; numCat++) {
                inverted_list_insert(j.categorias[numCat], j.id_game, &categorias_idx);
            }
        }
        // Caso que o jogo possui apenas 1 categoria:
        else if(j.categorias[0][0] != '\0') {
            inverted_list_insert(j.categorias[0], j.id_game, &categorias_idx);
        }
    }
}


/* Exibe um usuario dado seu RRN */
bool exibir_usuario(int rrn) {
    if (rrn < 0)
        return false;

    Usuario u = recuperar_registro_usuario(rrn);

    printf("%s, %s, %s, %s, %.2lf\n", u.id_user, u.username, u.email, u.celular, u.saldo);
    return true;
}

/* Exibe um jogo dado seu RRN */
bool exibir_jogo(int rrn) {
    if (rrn < 0)
        return false;

    Jogo j = recuperar_registro_jogo(rrn);

    printf("%s, %s, %s, %s, %s, %.2lf\n", j.id_game, j.titulo, j.desenvolvedor, j.editora, j.data_lancamento, j.preco);
    return true;
}

/* Exibe uma compra dado seu RRN */
bool exibir_compra(int rrn) {
    if (rrn < 0)
        return false;

    Compra c = recuperar_registro_compra(rrn);

    printf("%s, %s, %s\n", c.id_user_dono, c.data_compra, c.id_game);

    return true;
}


/* Recupera do arquivo de usuários o registro com o RRN
 * informado e retorna os dados na struct Usuario */
Usuario recuperar_registro_usuario(int rrn) {
    Usuario u;
    char temp[TAM_REGISTRO_USUARIO + 1], *p;
    strncpy(temp, ARQUIVO_USUARIOS + (rrn * TAM_REGISTRO_USUARIO), TAM_REGISTRO_USUARIO);
    temp[TAM_REGISTRO_USUARIO] = '\0';

    p = strtok(temp, ";");
    strcpy(u.id_user, p);
    p = strtok(NULL, ";");
    strcpy(u.username, p);
    p = strtok(NULL, ";");
    strcpy(u.email, p);
    p = strtok(NULL, ";");
    strcpy(u.celular, p);
    p = strtok(NULL, ";");
    u.saldo = atof(p);
    p = strtok(NULL, ";");

    return u;
}

/* Recupera do arquivo de jogos o registro com o RRN
 * informado e retorna os dados na struct Jogo */
Jogo recuperar_registro_jogo(int rrn) {
    /* <<< MINHA IMPLEMENTAÇÃO >>> */
    Jogo j;
    char temp[TAM_REGISTRO_JOGO + 1], *p;
    strncpy(temp, ARQUIVO_JOGOS + (rrn * TAM_REGISTRO_JOGO), TAM_REGISTRO_JOGO);
    temp[TAM_REGISTRO_JOGO] = '\0';

    // A função strtok() quebra a string temp em um token, usando o delimitador ";". O token é atribuido a p.
    p = strtok(temp, ";");
    // A função strcpy() copia o token p (que corresponde ao id_game) em j.id_game.
    strcpy(j.id_game, p);
    p = strtok(NULL, ";");
    strcpy(j.titulo, p);
    p = strtok(NULL, ";");
    strcpy(j.desenvolvedor, p);
    p = strtok(NULL, ";");
    strcpy(j.editora, p);
    p = strtok(NULL, ";");
    strcpy(j.data_lancamento, p);
    p = strtok(NULL, ";");
    j.preco = atof(p);

    // Para no primeiro byte da categorias[0].
    p = strtok(NULL, ";");

    // Se o primeiro byte de p (que é o primeiro byte de categorias[0]) for diferente de "#", significa que a categorias[0] está preenchida.
    if(*p != '#') {
        // Coloca em p os bytes que vem depois do ";" até o "|". Caso não encontre o "|", coloca todos os bytes até o final do registro.
        p = strtok(p, "|");

        // Atribui o conteúdo de p a categorias[0].
        strcpy(j.categorias[0], p);

        p = strtok(NULL, "|");

        // Se p (que é o categorias[1]) for diferente de NULL, significa que a categorias[1] está preenchida.
        if(p != NULL) {

            // Atribui o conteúdo de p a categorias[1].
            strcpy(j.categorias[1], p);

            p = strtok(NULL, "|");
            
            // Se p (que é o categorias[2]) for diferente de NULL, significa que a categorias[2] está preenchida.
            if(p != NULL) {
                // Atribui o conteúdo de p a categorias[2].
                strcpy(j.categorias[2], p);
            }
            else {
                // Atribuo '\0' a todas as categorias não preenchidas, para garantir que não vou pegar lixo de memória.
                j.categorias[2][0] = '\0';
            }
        }
        else {
            // Atribuo '\0' a todas as categorias não preenchidas, para garantir que não vou pegar lixo de memória.
            j.categorias[1][0] = j.categorias[2][0] = '\0';
        }
    }
    else {
        // Atribuo '\0' a todas as categorias não preenchidas, para garantir que não vou pegar lixo de memória.
        j.categorias[0][0] = j.categorias[1][0] = j.categorias[2][0] = '\0';
    }

    return j;
}

/* Recupera do arquivo de compras o registro com o RRN
 * informado e retorna os dados na struct Compra */
Compra recuperar_registro_compra(int rrn) {
    /* <<< MINHA IMPLEMENTAÇÃO >>> */
    Compra c;
    char temp[TAM_REGISTRO_COMPRA + 1];
    strncpy(temp, ARQUIVO_COMPRAS + (rrn * TAM_REGISTRO_COMPRA), TAM_REGISTRO_COMPRA);
    //temp[TAM_REGISTRO_COMPRA] = '\0';

    // Copia os 11 primeiros bytes do vetor temp em c.id_user_dono, sem adicionar o '\0' no final.
    strncpy(c.id_user_dono, temp, 11);
    // Adiciona o '\0' no final do c.id_user_dono.
    c.id_user_dono[11] = '\0';
    // Pula os 11 primeiros bytes (temp + 11) e copia os 8 bytes seguintes do vetor temp em c.id_user_dono, sem adicionar o '\0' no final.
    strncpy(c.data_compra, (temp + 11), 8);
    // Adiciona o '\0' no final do c.data_compra.
    c.data_compra[8] = '\0';
    // Pula os 19 primeiros bytes (temp + 19) e copia os 8 bytes seguintes do vetor temp em c.id_user_dono, sem adicionar o '\0' no final.
    strncpy(c.id_game, (temp + 19), 8);
    // Adiciona o '\0' no final do c.id_game.
    c.id_game[8] = '\0';

    return c;
}


/* Escreve no arquivo de usuários na posição informada (RRN)
 * os dados na struct Usuario */
void escrever_registro_usuario(Usuario u, int rrn) {
    char temp[TAM_REGISTRO_USUARIO + 1], p[100];
    temp[0] = '\0'; p[0] = '\0';

    strcpy(temp, u.id_user);
    strcat(temp, ";");
    strcat(temp, u.username);
    strcat(temp, ";");
    strcat(temp, u.email);
    strcat(temp, ";");
    strcat(temp, u.celular);
    strcat(temp, ";");
    sprintf(p, "%013.2lf", u.saldo);
    strcat(temp, p);
    strcat(temp, ";");

    // Completa os bytes faltantes do registro com '#'.
    for (int i = strlen(temp); i < TAM_REGISTRO_USUARIO; i++)
        temp[i] = '#';

    strncpy(ARQUIVO_USUARIOS + rrn*TAM_REGISTRO_USUARIO, temp, TAM_REGISTRO_USUARIO);
    ARQUIVO_USUARIOS[qtd_registros_usuarios*TAM_REGISTRO_USUARIO] = '\0';
}

/* Escreve no arquivo de jogos na posição informada (RRN)
 * os dados na struct Jogo */
void escrever_registro_jogo(Jogo j, int rrn) {
    /* <<< MINHA IMPLEMENTAÇÃO >>> */
    char temp[TAM_REGISTRO_JOGO + 1], p[100];
    temp[0] = '\0'; p[0] = '\0';

    strcpy(temp, j.id_game);
    strcat(temp, ";");
    strcat(temp, j.titulo);
    strcat(temp, ";");
    strcat(temp, j.desenvolvedor);
    strcat(temp, ";");
    strcat(temp, j.editora);
    strcat(temp, ";");
    strcat(temp, j.data_lancamento);
    strcat(temp, ";");
    sprintf(p, "%013.2lf", j.preco);
    strcat(temp, p);
    strcat(temp, ";");
    // Verifico quais categorias são diferentes de '\0' e escrevo elas no registro.
    if(j.categorias[0][0] != '\0') {
        strcat(temp, j.categorias[0]);
    }
    if(j.categorias[1][0] != '\0') {
        strcat(temp, "|");
        strcat(temp, j.categorias[1]);
    }
    if(j.categorias[2][0] != '\0') {
        strcat(temp, "|");
        strcat(temp, j.categorias[2]);
    }
    strcat(temp, ";");

    // Completa os bytes faltantes do registro com '#'.
    for (int i = strlen(temp); i < TAM_REGISTRO_JOGO; i++)
        temp[i] = '#';

    strncpy(ARQUIVO_JOGOS + rrn*TAM_REGISTRO_JOGO, temp, TAM_REGISTRO_JOGO);
    ARQUIVO_JOGOS[qtd_registros_jogos*TAM_REGISTRO_JOGO] = '\0';
}

/* Escreve no arquivo de compras na posição informada (RRN)
 * os dados na struct Compra */
void escrever_registro_compra(Compra c, int rrn) {
    /* <<< MINHA IMPLEMENTAÇÃO >>> */
    char temp[TAM_REGISTRO_COMPRA + 1];
    temp[0] = '\0';

    // Como todos os campos possuem um tamanho fixo, não são usados delimitadores e o final do registro também não é completado com '#'.
    // Nem a função strncpy(), nem a strncat() adicionam '\0' no final de temp.
    strncpy(temp, c.id_user_dono, 11);
    strncat(temp, c.data_compra, 8);
    strncat(temp, c.id_game, 8);
    
    strncpy(ARQUIVO_COMPRAS + rrn*TAM_REGISTRO_COMPRA, temp, TAM_REGISTRO_COMPRA);
    ARQUIVO_COMPRAS[qtd_registros_compras*TAM_REGISTRO_COMPRA] = '\0';
}


/* Funções principais */
void cadastrar_usuario_menu(char *id_user, char *username, char *email) {
    /* <<< MINHA IMPLEMENTAÇÃO >>> */
    usuarios_index stUsuario;
    strcpy(stUsuario.id_user, id_user);

    usuarios_index *pstBuscado = (usuarios_index *)busca_binaria(&stUsuario, usuarios_idx, qtd_registros_usuarios, sizeof(usuarios_index), qsort_usuarios_idx, false);

    if(pstBuscado == NULL) {
        Usuario usuario;
        
        // 1º: preciso atribuir o id_user ao usuário, que é do tipo Usuário.
        strcpy(usuario.id_user, id_user);
        strcpy(usuario.username, username);
        strcpy(usuario.email, email);
        strcpy(usuario.celular, "***********");
        usuario.saldo = 0.0;
        
        // 2º: preciso atribuir o id_user e o RRN ao stUsuario, que é do tipo usuarios_index.
        strcpy(stUsuario.id_user, usuario.id_user);
        stUsuario.rrn = qtd_registros_usuarios;

        // 3º: só então posso atribuir o stUsuario ao vetor usuarios_idx, na posição qtd_registros_usuarios.
        usuarios_idx[qtd_registros_usuarios] = stUsuario;

        // Escreve no ARQUIVO_USUARIOS e depois incrementa qtd_registros_usuarios.
        escrever_registro_usuario(usuario, usuarios_idx[qtd_registros_usuarios++].rrn);
        
        // Após inserir o registro, ordeno o vetor de variáveis do tipo usuario_index, chamado usuarios_idx.
        qsort(usuarios_idx, qtd_registros_usuarios, sizeof(usuarios_index), qsort_usuarios_idx);
        
        printf(SUCESSO);
    }
    else {
        printf(ERRO_PK_REPETIDA, stUsuario.id_user);
    }
}

void cadastrar_celular_menu(char* id_user, char* celular) {
    /* <<< MINHA IMPLEMENTAÇÃO >>> */
    usuarios_index stUsuario;
    strcpy(stUsuario.id_user, id_user);
    usuarios_index *pstBuscado = (usuarios_index *)busca_binaria(&stUsuario, usuarios_idx, qtd_registros_usuarios, sizeof(usuarios_index), qsort_usuarios_idx, false);
    
    if(pstBuscado != NULL) {
        Usuario usuario = recuperar_registro_usuario(pstBuscado->rrn);

        // A função strcpy() copia exatamente o tamanho e o contéudo da segunda string na primeira (usuario.celular).
        strcpy(usuario.celular, celular);

        escrever_registro_usuario(usuario, pstBuscado->rrn);

        printf(SUCESSO);
    }
    else {
        printf(ERRO_REGISTRO_NAO_ENCONTRADO);
    }
}

void remover_usuario_menu(char *id_user) {
    /* <<< MINHA IMPLEMENTAÇÃO >>> */
    usuarios_index stUsuario;
    strcpy(stUsuario.id_user, id_user);

    usuarios_index *pstBuscado = (usuarios_index *)busca_binaria(&stUsuario, usuarios_idx, qtd_registros_usuarios, sizeof(usuarios_index), qsort_usuarios_idx, false);
    
    if(pstBuscado != NULL && pstBuscado->rrn != -1) {
        Usuario usuario = recuperar_registro_usuario(pstBuscado->rrn);

        ARQUIVO_USUARIOS[pstBuscado->rrn * TAM_REGISTRO_USUARIO] = '*';
        ARQUIVO_USUARIOS[pstBuscado->rrn * TAM_REGISTRO_USUARIO + 1] = '|';
        pstBuscado->rrn = -1;

        printf(SUCESSO);
    }
    else {
        printf(ERRO_REGISTRO_NAO_ENCONTRADO);
    }
}

void cadastrar_jogo_menu(char *titulo, char *desenvolvedor, char *editora, char* lancamento, double preco) {
    /* <<< MINHA IMPLEMENTAÇÃO >>> */
    Jogo jogo;
    jogos_index stJogo;
    titulos_index stTitulo;
    
    // O "%08u" faz com que sejam adicionados até 8 zeros à esquerda do conteúdo de qtd_registros_jogos.
    sprintf(jogo.id_game, "%08u", qtd_registros_jogos);
    // Copia o id_game do jogo em stJogo.
    strcpy(stJogo.id_game, jogo.id_game);

    strcpy(jogo.titulo, titulo);
    // Copia o titulo do jogo em stTitulo.
    strcpy(stTitulo.titulo, jogo.titulo);

    titulos_index *pstBuscadoTitle = (titulos_index *)busca_binaria(&stTitulo, titulo_idx, qtd_registros_jogos, sizeof(titulos_index), qsort_titulo_idx, false);

    if(pstBuscadoTitle == NULL) {
        strcpy(jogo.desenvolvedor, desenvolvedor);
        strcpy(jogo.editora, editora);
        strcpy(jogo.data_lancamento, lancamento);
        jogo.preco = preco;
        // Uso o '\0' para garantir que as categorias serão inicializadas sem nada.
        jogo.categorias[0][0] = '\0';
        jogo.categorias[1][0] = '\0';
        jogo.categorias[2][0] = '\0';
        
        // Atribuo o id_game e o RRN ao stJogo, que é do tipo jogos_index.
        strcpy(stJogo.id_game, jogo.id_game);
        stJogo.rrn = qtd_registros_jogos;

        jogos_idx[qtd_registros_jogos] = stJogo;

        // Atribuo o titulo e o id_game ao stTitulo, que é do tipo titulos_index.
        strcpy(stTitulo.titulo, jogo.titulo);
        strcpy(stTitulo.id_game, jogo.id_game);
        
        titulo_idx[qtd_registros_jogos] = stTitulo;

        escrever_registro_jogo(jogo, jogos_idx[qtd_registros_jogos++].rrn);
        
        qsort(jogos_idx, qtd_registros_jogos, sizeof(jogos_index), qsort_jogos_idx);
        qsort(titulo_idx, qtd_registros_jogos, sizeof(titulos_index), qsort_titulo_idx);
        
        printf(SUCESSO);
    }
    else {
        printf(ERRO_PK_REPETIDA, stTitulo.titulo);
    }
}

void adicionar_saldo_menu(char *id_user, double valor) {
    /* <<< MINHA IMPLEMENTAÇÃO >>> */
    if(valor <= 0) {
            printf(ERRO_VALOR_INVALIDO);
            return;
    }

    usuarios_index stUsuario;
    strcpy(stUsuario.id_user, id_user);

    usuarios_index *pstBuscado = (usuarios_index *)busca_binaria(&stUsuario, usuarios_idx, qtd_registros_usuarios, sizeof(usuarios_index), qsort_usuarios_idx, false);
    
    if(pstBuscado != NULL) {
        Usuario usuario = recuperar_registro_usuario(pstBuscado->rrn);

        usuario.saldo = usuario.saldo + valor;

        escrever_registro_usuario(usuario, pstBuscado->rrn);

        printf(SUCESSO);
    }
    else {
        printf(ERRO_REGISTRO_NAO_ENCONTRADO);
    }
}

void comprar_menu(char *id_user, char *titulo) {
    /* <<< MINHA IMPLEMENTAÇÃO >>> */
    usuarios_index stUsuario;
    strcpy(stUsuario.id_user, id_user);
    usuarios_index *pstBuscadoUser = (usuarios_index *)busca_binaria(&stUsuario, usuarios_idx, qtd_registros_usuarios, sizeof(usuarios_index), qsort_usuarios_idx, false);

    titulos_index stTitulo;
    strcpy(stTitulo.titulo, titulo);
    titulos_index *pstBuscadoTitle = (titulos_index *)busca_binaria(&stTitulo, titulo_idx, qtd_registros_jogos, sizeof(titulos_index), qsort_titulo_idx, false);

    if(pstBuscadoUser == NULL || (pstBuscadoUser != NULL && pstBuscadoUser->rrn == -1) || pstBuscadoTitle == NULL) {
        printf(ERRO_REGISTRO_NAO_ENCONTRADO);

        return;
    }

    compras_index stCompra;
    strcpy(stCompra.id_user, pstBuscadoUser->id_user);
    strcpy(stCompra.id_game, pstBuscadoTitle->id_game);
    compras_index *pstBuscadoBuy = (compras_index *)busca_binaria(&stCompra, compras_idx, qtd_registros_compras, sizeof(compras_index), qsort_compras_idx, false);

    // Verifica se não há nenhuma compra igual já registrada no índice compras_idx.
    if(pstBuscadoBuy == NULL) {
        Usuario usuario = recuperar_registro_usuario(pstBuscadoUser->rrn);
        Jogo jogo = recuperar_registro_jogo(atol(pstBuscadoTitle->id_game));

        // Se o usuário tiver saldo, ele compra o jogo.
        if(usuario.saldo >= jogo.preco) {
            // Atualiza o saldo do usuário que acabou de comprar o jogo.
            usuario.saldo = usuario.saldo - jogo.preco;

            escrever_registro_usuario(usuario, pstBuscadoUser->rrn);

            Compra compra;
            char data[TAM_DATE];

            current_date(data);

            strcpy(compra.id_user_dono, pstBuscadoUser->id_user);
            strcpy(compra.data_compra, data);
            strcpy(compra.id_game, pstBuscadoTitle->id_game);
            
            // Atribui os valores ao índice compras_idx.
            strcpy(compras_idx[qtd_registros_compras].id_user, usuario.id_user);
            strcpy(compras_idx[qtd_registros_compras].id_game, jogo.id_game);
            compras_idx[qtd_registros_compras].rrn = qtd_registros_compras;
            
            // Atribui os valores ao índice data_user_game_idx.
            strcpy(data_user_game_idx[qtd_registros_compras].data, data);
            strcpy(data_user_game_idx[qtd_registros_compras].id_user, usuario.id_user);
            strcpy(data_user_game_idx[qtd_registros_compras].id_game, jogo.id_game);

            escrever_registro_compra(compra, compras_idx[qtd_registros_compras++].rrn);
            
            qsort(compras_idx, qtd_registros_compras, sizeof(compras_index), qsort_compras_idx);
            qsort(data_user_game_idx, qtd_registros_compras, sizeof(data_user_game_index), qsort_data_user_game_idx);

            printf(SUCESSO);
        }
        // Se não, avisa que o saldo é insuficiente.
        else {
            printf(ERRO_SALDO_NAO_SUFICIENTE);
        }
    }
        
    else {
        char aux[TAM_ID_USER + TAM_ID_GAME + 1];
        strcpy(aux, pstBuscadoBuy->id_user);
        strcat(aux, pstBuscadoBuy->id_game);

        printf(ERRO_PK_REPETIDA, aux);
    }
}

void cadastrar_categoria_menu(char* titulo, char* categoria) {
    /* <<< MINHA IMPLEMENTAÇÃO >>> */
    titulos_index jogoTitulo;
    strcpy(jogoTitulo.titulo, titulo);

    titulos_index *pstBuscado = (titulos_index *)busca_binaria(&jogoTitulo, titulo_idx, qtd_registros_jogos, sizeof(titulos_index), qsort_titulo_idx, false);
    
    if(pstBuscado != NULL) {
        // Passo como parâmetro o qtd_registros_jogos ao invés do RRN, pois o RRN dos jogos é incremental e eles não podem ser excluídos.
        Jogo jogo = recuperar_registro_jogo(atol(pstBuscado->id_game));
        
        for(int numCat = 0; numCat < QTD_MAX_CATEGORIAS; numCat++) {
            // Se o primeiro byte de "categorias[numCat]" for '\0', significa que "categorias[numCat]" está vazio e é possível inserir nele.
            if(jogo.categorias[numCat][0] == '\0') {
                // Insere no "categorias[numCat]" do jogo.
                strcpy(jogo.categorias[numCat], categoria);

                // A função atol() vai transformar a string pstBuscado->id_game em inteiro, que vai ser usado para atribuir o RRN.
                escrever_registro_jogo(jogo, atol(pstBuscado->id_game));

                // Insere a nova categoria na lista invertida.
                inverted_list_insert(categoria, pstBuscado->id_game, &categorias_idx);

                printf(SUCESSO);

                // Sai do loop, após inserir a nova categoria em um espaço vazio.
                break;
            }
            // Se categorias[numCat] for igual à categoria nova, significa que a categoria é repetida.
            else if(strcmp(jogo.categorias[numCat], categoria) == 0) {
                printf(ERRO_CATEGORIA_REPETIDA, jogo.titulo, jogo.categorias[numCat]);

                // Sai do loop, pois a categoria que era para ser inserida é repetida.
                break;
            }
        }
    }
    else {
        printf(ERRO_REGISTRO_NAO_ENCONTRADO);
    }
}


/* Busca */
void buscar_usuario_id_user_menu(char *id_user) {
    /* <<< MINHA IMPLEMENTAÇÃO >>> */
    usuarios_index stUsuario;
    strcpy(stUsuario.id_user, id_user);

    usuarios_index *pstBuscado = (usuarios_index *)busca_binaria(&stUsuario, usuarios_idx, qtd_registros_usuarios, sizeof(usuarios_index), qsort_usuarios_idx, true);
    
    if(pstBuscado != NULL) {
        exibir_usuario(pstBuscado->rrn);
    }
    else {
        printf(ERRO_REGISTRO_NAO_ENCONTRADO);
    }
}

void buscar_jogo_id_menu(char *id_game) {
    /* <<< MINHA IMPLEMENTAÇÃO >>> */
    jogos_index stJogo;
    strcpy(stJogo.id_game, id_game);

    jogos_index *pstBuscado = (jogos_index *)busca_binaria(&stJogo, jogos_idx, qtd_registros_jogos, sizeof(jogos_index), qsort_jogos_idx, true);
    
    if(pstBuscado != NULL) {
        exibir_jogo(pstBuscado->rrn);
    }
    else {
        printf(ERRO_REGISTRO_NAO_ENCONTRADO);
    }
}

void buscar_jogo_titulo_menu(char *titulo) {
    /* <<< MINHA IMPLEMENTAÇÃO >>> */
    titulos_index stTitulo;
    strcpy(stTitulo.titulo, titulo);

    titulos_index *pstBuscadoTitulo = (titulos_index *)busca_binaria(&stTitulo, titulo_idx, qtd_registros_jogos, sizeof(titulos_index), qsort_titulo_idx, true);

    if(pstBuscadoTitulo != NULL) {
        jogos_index stJogo;
        strcpy(stJogo.id_game, pstBuscadoTitulo->id_game);

        jogos_index *pstBuscadoJogo = (jogos_index *)busca_binaria(&stJogo, jogos_idx, qtd_registros_jogos, sizeof(jogos_index), qsort_jogos_idx, true);

        if(pstBuscadoJogo != NULL) {
            exibir_jogo(pstBuscadoJogo->rrn);
        }
        else {
            printf(ERRO_REGISTRO_NAO_ENCONTRADO);
        }
    }
    else {
        printf(ERRO_REGISTRO_NAO_ENCONTRADO);
    }
}


/* Listagem */
void listar_usuarios_id_user_menu() {
    /* <<< MINHA IMPLEMENTAÇÃO >>> */
    if(qtd_registros_usuarios > 0) {
        for(int i = 0; i < qtd_registros_usuarios; i++) {
            exibir_usuario(usuarios_idx[i].rrn);
        }
    }
    else
        printf(AVISO_NENHUM_REGISTRO_ENCONTRADO);
}

void listar_jogos_categorias_menu(char *categoria) {
    /* <<< MINHA IMPLEMENTAÇÃO >>> */
    categorias_secundario_index jogoCat;
    strcpy(jogoCat.chave_secundaria, categoria);
    
    categorias_secundario_index *pstBuscado = (categorias_secundario_index *)busca_binaria(&jogoCat, categorias_idx.categorias_secundario_idx, categorias_idx.qtd_registros_secundario, sizeof(categorias_secundario_index), qsort_categorias_secundario_idx, false);

    if(pstBuscado == NULL) {
        printf(AVISO_NENHUM_REGISTRO_ENCONTRADO);
    }
    else {
        int posicao = pstBuscado->primeiro_indice;

        // Cria o vetor para receber as chaves primárias encontradas.
        char resultChar[MAX_REGISTROS][TAM_CHAVE_CATEGORIAS_PRIMARIO_IDX];

        // Recebe a quantidade de chaves primárias encontradas.
        char aux[TAM_CHAVE_CATEGORIAS_PRIMARIO_IDX];

        int qtdChavesPrim = inverted_list_primary_search(&resultChar, true, posicao, &posicao, &categorias_idx);

        for(int i = qtdChavesPrim - 1; i > 0; i--) {
            for(int j = 0; j < i; j++) {
                if (strcmp(resultChar[j], resultChar[j + 1]) > 0) {
                    strcpy(aux, resultChar[j]);
                    strcpy(resultChar[j], resultChar[j + 1]);
                    strcpy(resultChar[j + 1], aux);
                }
            }
        }
        for (int i = 0; i < qtdChavesPrim; i++) {
            exibir_jogo(atol(resultChar[i]));
        }
        if (qtdChavesPrim == 0) {
            printf(AVISO_NENHUM_REGISTRO_ENCONTRADO);
        }
    }
}

void listar_compras_periodo_menu(char *data_inicio, char *data_fim) {
    /* <<< MINHA IMPLEMENTAÇÃO >>> */
    if(qtd_registros_compras <= 0) {
        printf(AVISO_NENHUM_REGISTRO_ENCONTRADO);
    }
    else {
        data_user_game_index stInicio;
        data_user_game_index stFim;

        strcpy(stInicio.data, data_inicio);
        strcpy(stFim.data, data_fim);

        data_user_game_index *pstPiso = (data_user_game_index *)busca_binaria_piso(&stInicio, data_user_game_idx, qtd_registros_compras, sizeof(data_user_game_index), qsort_data_idx);
        data_user_game_index *pstTeto = (data_user_game_index *)busca_binaria_teto(&stFim, data_user_game_idx, qtd_registros_compras, sizeof(data_user_game_index), qsort_data_idx);

        // Se a pstTeto for menor que a data_inicio ou se a pstPiso for maior que o data_fim, significa que não há compras no período buscado.
        if(pstPiso == NULL || pstTeto == NULL) {
            printf(AVISO_NENHUM_REGISTRO_ENCONTRADO);
        }
        else {
            for(data_user_game_index *pstAux = pstPiso; pstAux <= pstTeto; pstAux++) {
                compras_index stCompra;
                strcpy(stCompra.id_user, pstAux->id_user);
                strcpy(stCompra.id_game, pstAux->id_game);

                compras_index *pstBuscado = (compras_index *)busca_binaria(&stCompra, compras_idx, qtd_registros_compras, sizeof(compras_index), qsort_compras_idx, true);

                exibir_compra(pstBuscado->rrn);
            }
        }
    }
}


/* Liberar espaço */
void liberar_espaco_menu() {
    /* <<< MINHA IMPLEMENTAÇÃO >>> */
    int antigo = 0, novo = 0;

    // Verifica se não chegou ao final do arquivo.
    while(ARQUIVO_USUARIOS[antigo * TAM_REGISTRO_USUARIO] != '\0') {
        // Se os primeiros 2 bytes forem "*|", é porque o registro foi apagado e seu espaço dever ser liberado.
        if(ARQUIVO_USUARIOS[antigo * TAM_REGISTRO_USUARIO] == '*' && ARQUIVO_USUARIOS[antigo * TAM_REGISTRO_USUARIO + 1] == '|') {
            // Pula o registro apagado.
            antigo++;
        }
        else {
            // Copia 1 registro antigo não apagado no arquivo novo.
            strncpy(ARQUIVO_USUARIOS + novo * TAM_REGISTRO_USUARIO, ARQUIVO_USUARIOS + (antigo * TAM_REGISTRO_USUARIO), 256);

            antigo++;
            novo++;
        }
    }

    // Insire o '\0' para marcar o final do arquivo.
    ARQUIVO_USUARIOS[novo * TAM_REGISTRO_USUARIO] = '\0';

    // Atualiza a quantidade de registros de usuários que existem no arquivo novo.
    qtd_registros_usuarios = novo;

    // Recria o índice primário de usuários, que é o único que pode ser apagado.
    criar_usuarios_idx();

    printf(SUCESSO);
}


/* Imprimir arquivos de dados */
void imprimir_arquivo_usuarios_menu() {
    if (qtd_registros_usuarios == 0)
        printf(ERRO_ARQUIVO_VAZIO);
    else
        printf("%s\n", ARQUIVO_USUARIOS);
}

void imprimir_arquivo_jogos_menu() {
    if (qtd_registros_jogos == 0)
        printf(ERRO_ARQUIVO_VAZIO);
    else
        printf("%s\n", ARQUIVO_JOGOS);
}

void imprimir_arquivo_compras_menu() {
    if (qtd_registros_compras == 0)
        printf(ERRO_ARQUIVO_VAZIO);
    else
        printf("%s\n", ARQUIVO_COMPRAS);
}


/* Imprimir índices primários */
void imprimir_usuarios_idx_menu() {
    if (qtd_registros_usuarios == 0)
        printf(ERRO_ARQUIVO_VAZIO);

    for (unsigned i = 0; i < qtd_registros_usuarios; ++i)
        printf("%s, %d\n", usuarios_idx[i].id_user, usuarios_idx[i].rrn);
}

void imprimir_jogos_idx_menu() {
    if (qtd_registros_jogos == 0)
        printf(ERRO_ARQUIVO_VAZIO);

    for (unsigned i = 0; i < qtd_registros_jogos; ++i)
        printf("%s, %d\n", jogos_idx[i].id_game, jogos_idx[i].rrn);
}

void imprimir_compras_idx_menu() {
    if (qtd_registros_compras == 0)
        printf(ERRO_ARQUIVO_VAZIO);

    for (unsigned i = 0; i < qtd_registros_compras; ++i)
        printf("%s, %s, %d\n", compras_idx[i].id_user, compras_idx[i].id_game, compras_idx[i].rrn);
}


/* Imprimir índices secundários */
void imprimir_titulo_idx_menu() {
    if (qtd_registros_jogos == 0)
        printf(ERRO_ARQUIVO_VAZIO);

    for (unsigned i = 0; i < qtd_registros_jogos; ++i)
        printf("%s, %s\n", titulo_idx[i].titulo, titulo_idx[i].id_game);
}

void imprimir_data_user_game_idx_menu() {
    if (qtd_registros_compras == 0) {
        printf(ERRO_ARQUIVO_VAZIO);
        return;
    }

    for (unsigned i = 0; i < qtd_registros_compras; ++i)
        printf("%s, %s, %s\n", data_user_game_idx[i].data, data_user_game_idx[i].id_user, data_user_game_idx[i].id_game);
}

void imprimir_categorias_secundario_idx_menu() {
    if (categorias_idx.qtd_registros_secundario == 0)
        printf(ERRO_ARQUIVO_VAZIO);

    for (unsigned i = 0; i < categorias_idx.qtd_registros_secundario; ++i)
        printf("%s, %d\n", (categorias_idx.categorias_secundario_idx)[i].chave_secundaria, (categorias_idx.categorias_secundario_idx)[i].primeiro_indice);
}

void imprimir_categorias_primario_idx_menu() {
    if (categorias_idx.qtd_registros_primario == 0)
        printf(ERRO_ARQUIVO_VAZIO);

    for (unsigned i = 0; i < categorias_idx.qtd_registros_primario; ++i)
        printf("%s, %d\n", (categorias_idx.categorias_primario_idx)[i].chave_primaria, (categorias_idx.categorias_primario_idx)[i].proximo_indice);
}


/* Liberar memória e encerrar programa */
void liberar_memoria_menu() {
    /* <<< MINHA IMPLEMENTAÇÃO >>> */
    // Libera a memória dos índices primários.
    free(usuarios_idx);
    free(jogos_idx);
    free(compras_idx);

    // Libera a memória dos índices secundários.
    free(titulo_idx);
    free(data_user_game_idx);
    free(categorias_idx.categorias_primario_idx);
    free(categorias_idx.categorias_secundario_idx);
}


/* Implementações da função compar(): */
/* Função de comparação entre chaves do índice usuarios_idx */
int qsort_usuarios_idx(const void *a, const void *b) {
    return strcmp( ( (usuarios_index *)a )->id_user, ( (usuarios_index *)b )->id_user);
}

/* Função de comparação entre chaves do índice jogos_idx */
int qsort_jogos_idx(const void *a, const void *b) {
    /* <<< MINHA IMPLEMENTAÇÃO >>> */
    return strcmp( ( (jogos_index *)a )->id_game, ( (jogos_index *)b )->id_game);
}

/* Função de comparação entre chaves do índice compras_idx */
int qsort_compras_idx(const void *a, const void *b) {
    /* <<< MINHA IMPLEMENTAÇÃO >>> */
    int aux = strcmp( ( (compras_index *)a )->id_user, ( (compras_index *)b )->id_user);

    // Se os id_user forem diferentes, retorno a ordenação pelo id_user, já que é para ordenar primeiro pelo id_user e depois pelo id_game.
    if(aux != 0) {
        return aux;
    }
    // Se os id_user forem iguais, retorno a ordenação pelo id_game.
    else {
        return strcmp( ( (compras_index *)a )->id_game, ( (compras_index *)b )->id_game);
    }
}

/* Função de comparação entre chaves do índice titulo_idx */
int qsort_titulo_idx(const void *a, const void *b) {
    /* <<< MINHA IMPLEMENTAÇÃO >>> */
    return strcmp( ( (titulos_index *)a )->titulo, ( (titulos_index *)b )->titulo);
}

/* Funções de comparação entre chaves do índice data_user_game_idx */
int qsort_data_idx(const void *a, const void *b) {
    /* <<< MINHA IMPLEMENTAÇÃO >>> */
    return strcmp( ( (data_user_game_index *)a)->data, ( (data_user_game_index *)b)->data);
}


int qsort_data_user_game_idx(const void *a, const void *b) {
    /* <<< MINHA IMPLEMENTAÇÃO >>> */
    int aux = strcmp( ( (data_user_game_index *)a)->data, ( (data_user_game_index *)b )->data);

    if(aux != 0) {
        return aux;
    }
    else {
        return strcmp( ( (data_user_game_index *)a)->id_user, ( (data_user_game_index *)b )->id_user);
    }
}

/* Função de comparação entre chaves do índice secundário de categorias_idx */
int qsort_categorias_secundario_idx(const void *a, const void *b) {
    /* <<< MINHA IMPLEMENTAÇÃO >>> */
    return strcmp( ( (categorias_secundario_index *)a )->chave_secundaria, ( (categorias_secundario_index *)b )->chave_secundaria);
}


/* Funções de manipulação de Lista Invertida */
void inverted_list_insert(char *chave_secundaria, char *chave_primaria, inverted_list *t) {
    /* <<< MINHA IMPLEMENTAÇÃO >>> */
    //int posicao = t->categorias_secundario_idx->primeiro_indice;
    int posicao = 0;
    
    // Caso que a categoria já existe:
    if(inverted_list_secondary_search(&posicao, false, chave_secundaria, t) == true) {

        inverted_list_primary_search(NULL, false, posicao, &posicao, t);

        // Substitui o "proximo_indice" da categoria primária, que é "-1" pela última posição do vetor de registros primários, onde será inserido o novo jogo da categoria.
        t->categorias_primario_idx[posicao].proximo_indice = t->qtd_registros_primario;

        
    }
    // Caso que a categoria é nova:
    else {
        categorias_secundario_index catSec;
        strcpy(catSec.chave_secundaria, chave_secundaria);

        // O primeiro índice é a posição onde está sendo inserido o primeiro jogo da categoria nova.
        catSec.primeiro_indice = t->qtd_registros_primario;

        t->categorias_secundario_idx[t->qtd_registros_secundario] = catSec;
        
        // Incrementa a quantidade de registros secundários (para que o novo elemento seja considerado) e ordena o índice secundário da lista invertida, "categorias_secundario_idx".
        t->qtd_registros_secundario++;
        qsort(t->categorias_secundario_idx, t->qtd_registros_secundario, sizeof(categorias_secundario_index), qsort_categorias_secundario_idx);
    }

    categorias_primario_index catPrim;
    strcpy(catPrim.chave_primaria, chave_primaria);
    catPrim.proximo_indice = -1;

    t->categorias_primario_idx[t->qtd_registros_primario] = catPrim;

    // O índice primário da lista invertida, "categorias_primario_idx", não vai ser ordenado, pois é uma lista encadeada.
    t->qtd_registros_primario++;
}

bool inverted_list_secondary_search(int *result, bool exibir_caminho, char *chave_secundaria, inverted_list *t) {
    /* <<< MINHA IMPLEMENTAÇÃO >>> */
    bool encontrou = false;
    
    categorias_secundario_index catSec;
    strcpy(catSec.chave_secundaria, chave_secundaria);
    categorias_secundario_index *catBuscada;

    catBuscada = (categorias_secundario_index *)busca_binaria(&catSec, t->categorias_secundario_idx, t->qtd_registros_secundario, sizeof(categorias_secundario_index), qsort_categorias_secundario_idx, false);

    if(catBuscada != NULL) {
        *result = catBuscada->primeiro_indice;

        encontrou = true;
    }
    
    // Retorna se a chave secundária buscada foi encontrada ou não.
    return encontrou;
}

int inverted_list_primary_search(char result[][TAM_CHAVE_CATEGORIAS_PRIMARIO_IDX], bool exibir_caminho, int indice, int *indice_final, inverted_list *t) {
    /* <<< MINHA IMPLEMENTAÇÃO >>> */
    int qtdChaves = 0;

    if(exibir_caminho == true) {
        printf(REGS_PERCORRIDOS);
    }

    //while(t->categorias_primario_idx[indice].chave_primaria != NULL) {
    while(t->categorias_primario_idx[indice].proximo_indice != -1) {
        if(result != NULL) {
            strcpy(result[qtdChaves], &t->categorias_primario_idx[indice].chave_primaria[1]);
            //result[qtdChaves][9] = '\0';
        }

        if(exibir_caminho == true) {
            printf(" %d", indice);
        }

        // Atualiza o indice_final para o último índice diferente de "-1" que foi encontrado.
        *indice_final = indice;
        // Passa para o próximo índice de result (começa em 0).
        qtdChaves++;
        // Passa para o próximo índice da lista ligada.
        indice = t->categorias_primario_idx[indice].proximo_indice;
    }
    if(result != NULL) {
        strcpy(result[qtdChaves], &t->categorias_primario_idx[indice].chave_primaria[1]);
        //result[qtdChaves][9] = '\0';
    }
    if(exibir_caminho == true) {
        printf(" %d", indice);
    }
    *indice_final = indice;
    qtdChaves++;
    indice = t->categorias_primario_idx[indice].proximo_indice;

    if(exibir_caminho == true) {
        printf("\n");
    }

    // Retorna a quantidade de chaves primárias encontradas.
    return qtdChaves;
}


char* strpadright(char *str, char pad, unsigned size) {
    for (unsigned i = strlen(str); i < size; ++i)
        str[i] = pad;
    str[size] = '\0';
    return str;
}


/* Funções da busca binária */
void* busca_binaria(const void *key, const void *base0, size_t nmemb, size_t size, int (*compar)(const void *, const void *), bool exibir_caminho) {
    /* <<< MINHA IMPLEMENTAÇÃO >>> */
    size_t esq = 0;
    size_t dir = nmemb;
    size_t meio;
    const void *meioBase;
    int comparacao;
    
    if(exibir_caminho == true) {
        printf(REGS_PERCORRIDOS);
    }

    while (esq < dir) {
        meio = (esq + dir) / 2;
        
        // Atribui ao meioBase o endereço da variável para onde base0 aponta (do tipo usuarios_index).
        meioBase = base0 + (meio * size);
        // Recebe o resultado da comparação entre a key e o meio do vetor.
        comparacao = compar(key, meioBase);
        
        // Significa que a key buscada é MAIOR que a posição atual do meioBase, portanto, devo ignorar a metade esquerda do meioBase.
        if(comparacao > 0) {
            if(exibir_caminho == true) {
                printf(" %d", meio);
            }

            esq = meio + 1;
        }
        // Significa que a key buscada é MENOR que a posição atual do meioBase, portanto, devo ignorar a metade direita do meioBase.
        else if(comparacao < 0) {
            if(exibir_caminho == true) {
                printf(" %d", meio);
            }

            dir = meio;
        }
        // Se a função compar() retornar 0, significa que a key buscada foi encontrada no índice.
        else {
            // Imprime o caminho percorrido.
            if(exibir_caminho == true) {
                printf(" %d\n", meio);
            }
            
            // Realiza o typecast para o tipo void*, pois a assinatura da função exige isso.
            return (void *)meioBase;
        }
    }

    if(exibir_caminho == true) {
        printf("\n");
    }

    // A key buscada não foi encontrada no meioBase.
    return NULL;
}

void *busca_binaria_piso(const void *key, void *base, size_t num, size_t size, int (*compar)(const void *, const void *)) {
    /* <<< MINHA IMPLEMENTAÇÃO >>> */
    size_t esq = 0;
    size_t dir = num;
    size_t meio;
    const void *meioBase;
    int comparacao;
    const void *aux = NULL;

    // Se o vetor da busca binária estiver vazio, retorna NULL.
    if(num == 0) {
        return NULL;
    }

    while(esq < dir) {
        meio = (esq + dir) / 2;

        // Atribui ao meioBase o endereço da variável para onde base0 aponta (do tipo usuarios_index).
        meioBase = base + (meio * size);
        // Recebe o resultado da comparação entre a key e o meio do vetor.
        comparacao = compar(key, meioBase);

        // Significa que a key buscada é MAIOR que a posição atual do meioBase, portanto, devo ignorar a metade esquerda do meioBase.
        if(comparacao > 0) {
            esq = meio + 1;
        }
        // Significa que a key buscada é MENOR que a posição atual do meioBase, portanto, devo ignorar a metade direita do meioBase.
        else {
            dir = meio;

            aux = meioBase;
        }

        // Se o meio for menor que esq, não há um elemento piso.
        if(meio < esq) {
            return (void *)aux;
        }
    }

    // Se algum elemento for encontrado.
    return (void *)aux;
}

void *busca_binaria_teto(const void *key, void *base, size_t num, size_t size, int (*compar)(const void *, const void *)) {
    /* <<< MINHA IMPLEMENTAÇÃO >>> */
    size_t esq = 0;
    size_t dir = num;
    size_t meio;
    const void *meioBase;
    int comparacao;
    const void *aux = NULL;

    // Se o vetor da busca binária estiver vazio, retorna NULL.
    if(num == 0) {
        return NULL;
    }

    while(esq < dir) {
        meio = (esq + dir) / 2;

        // Atribui ao meioBase o endereço da variável para onde base0 aponta (do tipo usuarios_index).
        meioBase = base + (meio * size);
        // Recebe o resultado da comparação entre a key e o meio do vetor.
        comparacao = compar(key, meioBase);

        // Significa que a key buscada é MAIOR que a posição atual do meioBase, portanto, devo ignorar a metade esquerda do meioBase.
        if(comparacao >= 0) {
            esq = meio + 1;

            aux = meioBase;
        }
        // Significa que a key buscada é MENOR que a posição atual do meioBase, portanto, devo ignorar a metade direita do meioBase.
        else {
            dir = meio;
        }

        // Se o meio for maior que dir, não há um elemento teto.
        if(meio > dir) {
            return (void *)aux;
        }
    }

    // Se algum elemento for encontrado.
    return (void *)aux;
}
