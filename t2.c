#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

typedef struct{
    int matr;
    char nome[25];
    int nota1;
    int nota2;
    int excluido;
} reg_aluno;

//TODO: Implementar um indice (em memoria) para facilitar a busca dos registros do arquivo
//TODO: Alterar as funcoes de busca, insercao e exclusao para usar o indice
typedef struct{
    int matr;
    int pos_seek;
    struct indice* prox;
} indice;


indice * lst_localiza_ultimo_menor(indice * inicio, int matricula){
   indice* p = inicio;	/* ponteiro para percorrer a lista	*/
   indice* ant = NULL;	/* ponteiro para elemento anterior	*/
   /* procura posi��o de inser��o */
   while (p != NULL && p->matr < matricula){
      ant = p;
      p = p->prox;
   }
   return ant;
}

void lst_insere_ordenado(indice** inicio, int matricula, long int fpos)
{
   indice* ant = lst_localiza_ultimo_menor(*inicio, matricula);	/* ponteiro para elemento anterior	*/
   /* cria novo elemento */
   indice* novo = (indice*) malloc(sizeof(indice));
   novo->matr = matricula;
   novo->pos_seek = fpos;
   /* encadeia elemento */
   if (ant == NULL){                /* insere elemento no início*/
      novo->prox = *inicio;
      *inicio = novo;
   }
   else {                           /* insere elemento no meio da lista */
      novo->prox = ant->prox;
      ant->prox = novo;
   }

}

indice* lst_busca (indice* inicio, int matricula)
{
    indice* p = inicio;
    while (p!=NULL)
    {
        if (p->matr ==  matricula)
            return  p;
        p = p->prox;
    }
    return NULL;
}

void lst_retira (indice** inicio, int matricula)
{
    indice* ant = NULL;
    indice* p = *inicio;
    while (p != NULL && p->matr != matricula){
        ant = p;
        p = p->prox;
    }
    if (p == NULL)   
        return;
    if (ant == NULL)
        *inicio = p->prox;
    else
        ant->prox = p->prox;
    free(p);
}


void lst_libera (indice** inicio){
    indice* p = *inicio;
    while (p != NULL) {
        indice* t = p->prox;
        free(p);
        p = t;
    }
    *inicio = NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void mostra(FILE *arq){
    reg_aluno aluno;
    rewind(arq);
    while(fread(&aluno,sizeof(aluno), 1, arq))
        if (aluno.excluido == 0)
            printf("%d\t%s\t%d\t%d\n",aluno.matr, aluno.nome, aluno.nota1, aluno.nota2);
}

int pesquisa(indice* prim, int matr, indice* ind){
    ind = lst_busca(prim, matr);
    if (ind){
        return 1;
    }
    return 0;
}

/*
int pesquisa(FILE *arq, int matr, reg_aluno *al){
    reg_aluno aluno;
    rewind(arq);
    while(fread(&aluno,sizeof(aluno), 1, arq))
    if(aluno.matr == matr && aluno.excluido == 0){ 
        *al=aluno;
        return 1;
    }
    return 0;
}
*/

void exclui(FILE *arq, int matr, indice* prim){
    indice* ind;
    if (pesquisa(prim, matr, ind)){
        int excl = 1;
        printf("Excluindo matrícula: %d\n", matr);
        arq = ((FILE*) ind->pos_seek) + sizeof(reg_aluno);
        fseek(arq, -1*sizeof(int), SEEK_CUR);
        fwrite(&excl, sizeof(int), 1, arq);
        fflush(arq);
        lst_retira(&prim, matr);
    }
}

/*
void exclui(FILE *arq, int matr, indice* prim){
    reg_aluno aluno;
    if (pesquisa(arq, matr, &aluno)){
        int excl = 1;
        printf("Excluindo: %s\n", aluno.nome);
        fseek(arq, -1*sizeof(int), SEEK_CUR);
        fwrite(&excl, sizeof(int), 1, arq);
        fflush(arq);
    }
}
*/

void inclui(FILE *arq, indice* prim){
    reg_aluno aluno;
    fseek(arq, 0, SEEK_END);
        //printf("Pos=%d", ftell(arq));
    printf("Informe os dados do aluno (matr, nome, nota1 e nota2) \n");
    scanf("%d%s%d%d",&aluno.matr, aluno.nome, &aluno.nota1, &aluno.nota2);
    aluno.excluido=0;
    lst_insere_ordenado(&prim, aluno.matr, ftell(arq));
    fwrite(&aluno, sizeof(aluno), 1, arq);
}

int main(){
    int matr, op;
    reg_aluno aluno;
    indice* lst_ini = NULL;

    FILE *arq;
    if (access("alunos.dat", F_OK) == 0) 
        arq=fopen("alunos.dat","r+"); // arquivo existe
    else 
        arq=fopen("alunos.dat","w+"); // arquivo nao existia
    
    do {
        printf("\nMenu\n 1. Mostrar todos\n 2. Pesquisar\n 3. Incluir\n 4. Excluir\n 5. Sair\nInforme uma opcao: ");
        scanf("%d",&op);
        switch(op){
            case 1:
                printf("\nAlunos gravados no arquivo: \n");
                mostra(arq);
                break;
            case 2: 
                printf("\nDigite a matricula a ser buscada: ");
                scanf("%d",&matr);
                if(pesquisa(arq, matr, &aluno)) {
                    printf("\nAluno encontrado:\n");
                    printf("%d\t%s\t%d\t%d\n",aluno.matr, aluno.nome, aluno.nota1, aluno.nota2);
                } else
                    printf("\nA matricula %d nao foi encontrada!\n",matr);
                break;
            case 3:
                inclui(arq, lst_ini);
                break;
            case 4:
                printf("\nDigite a matricula a ser excluida: ");
                scanf("%d",&matr);
                exclui(arq, matr, lst_ini);
                break;
            case 5:
                printf("\nSaindo...\n\n");
                break;
            default: printf("\nOpcao invalida!\n");
            break;
        }
    }
    while(op != 5);
    fclose(arq);
    lst_libera(&lst_ini);
    return 0;
}