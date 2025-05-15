
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <pthread.h>
#include <sched.h>
#include <sys/sysinfo.h>

#define K 24
#define CHUNCK 512

#ifdef D
	#define DEBUG if(1)
#else
	#define DEBUG if(0)
#endif

int **outputs;
int num_thread;
volatile int end_turn;
volatile int actual_turn;
volatile char * data_input;
volatile int size_per_thread;
volatile int rest_size_per_thread;
char *nome_file;

typedef struct param{
	int my_id;
	int count;
}param;

void * func_thread(void* p){
	int fd,size_read,my_id,pos,max_read;
	fd=open(nome_file,O_RDONLY);
	if(fd==0){
		printf("Errore apertura file\n");
		return NULL;
	}
	my_id=((param *) p)->my_id;
	char buff[CHUNCK];
	int i,j;
	int count=0;
	int cella=0;
	max_read=size_per_thread;
	if(my_id<rest_size_per_thread){
		max_read+=1;
	}
	if(my_id<rest_size_per_thread){
		pos=my_id*max_read;
	}else{
		pos=my_id*max_read+rest_size_per_thread;
	}
	DEBUG printf("Analizzatore %d inizio:\nposizione:%d\n",my_id,pos);
	lseek(fd,pos,SEEK_SET);
	outputs[my_id]=(int*)calloc(sizeof(int),max_read);

	if(outputs[my_id]==NULL){
		printf("Errore malloc output data\n");
		return NULL;
	}

	j=0;
	while(j<max_read){
		size_read=read(fd,&buff,CHUNCK);
		if(size_read<=0){
			break;
		}
		for(i=0;i<size_read;i++){
			if(buff[i]=='H'){
				count++;
			}
			if(buff[i]=='\n'){
				outputs[my_id][cella]=count;
				cella++;
				count=0;
				outputs[my_id][cella]=-1;
			}
			if(buff[i]=='\0'){
				outputs[my_id][cella]=count;
				cella++;
				count=0;
				outputs[my_id][cella]=-1;
				break;
			}
		}
		j=j+CHUNCK;
	}
	outputs[my_id][cella]=count;
mk	((param*)p)->count=cella;
	DEBUG printf("Analizzato Termino %d celle fatte %d\n",my_id,cella);
	return NULL;
}

int main(int argc,char*argv[]){

	int fd,i;
	pthread_t * tids;
	pthread_t scrittore;
	cpu_set_t cpuset;
	int num_cpu;
	int j,old_c;
	int size_string;

	if(argc!=3){
			printf("eseguire %s <input_file> <output_file>\n",argv[0]);
			return 0;
	}
	nome_file = argv[1];
	num_cpu=get_nprocs();
	DEBUG printf("This system has %d processors configured and %d processors available.\n",
        get_nprocs_conf(),num_cpu);

	DEBUG printf("INIZIALIZZO VARIABILI\n");
	struct stat stats;
	fd=open(nome_file,O_RDONLY);
	if(fd<0){
		printf("Errore apertura file\n");
		printf("Errore nella write: errno = %d, descrizione = %s\n", errno, strerror(errno));
		return 0;
	}
	// if(fstat(fd,&stats)<0){
	// 	printf("Errore stat del file\n");
	// 	return 0;
	// }
	int output_fd=open(argv[2],O_RDWR|O_CREAT|O_TRUNC,0666);
	if(output_fd<0){
		printf("Errore apertura file\n");
		printf("Errore nella write: errno = %d, descrizione = %s\n", errno, strerror(errno));
		return 0;
	}
	num_thread=num_cpu;
	size_t s=lseek(fd,0,SEEK_END);
	lseek(fd,0,SEEK_SET);
	size_per_thread=s/num_thread;
	rest_size_per_thread=s % num_thread;
	DEBUG printf("Il file è di %d\nOgni thread ha in cario %d\n",s,size_per_thread);

	param * p=(param*)malloc(sizeof(param)*num_thread);
	tids=(pthread_t*) malloc(sizeof(pthread_t)*num_thread);
	if(tids==NULL || p==NULL){
		printf("Errore malloc tids or param\n");
		return 0;
	}
	outputs=(int**) malloc(sizeof(int*)*num_thread);
	if(outputs==NULL){
		printf("Errore outputs del file\n");
		return 0;
	}
	DEBUG printf("Creo i thread\n");
	CPU_ZERO(&cpuset);
	int * param=malloc(sizeof(int)*num_thread);
	for(i=0;i<num_thread;i++)
	{
		CPU_SET((i)%num_cpu,&cpuset);
		p[i].my_id=i;
		pthread_create(tids+i,NULL,func_thread,p+i);
		if(pthread_setaffinity_np(tids[i],sizeof(cpuset),&cpuset)){
			printf("Errore affinity %d\n",i);
			exit(-1);
		}
		CPU_ZERO(&cpuset);
	}
	int SIZE_MAX=8192;
	char *string_output = (char*)calloc(sizeof(char),SIZE_MAX);
	if(string_output==NULL){ printf("Errore malloc in data_input_t[0]\n");exit(-1);}
	old_c=0;
	int k=0;
	for(i=0;i<num_thread;i++){
		pthread_join(tids[i],NULL);
		size_string=0;
		outputs[i][0]=outputs[i][0]+old_c;
		for(j=0;j<p[i].count;j++){
			size_string+=sprintf(string_output+size_string,"%d\n",outputs[i][j]);
			if(size_string>SIZE_MAX){
				 string_output = realloc(string_output, SIZE_MAX*2);
				SIZE_MAX=SIZE_MAX*2;
			}
			k++;
		}
		//old_c=outputs[i][p[i].count];
		DEBUG printf("thread %d termina\n",i);
		write(output_fd,string_output,size_string);
	}

	free(string_output);
	free(tids);
	for(i=0;i<num_thread;i++) free(outputs[i]);
	free(outputs);
	DEBUG printf("Lettore: <termino>\n");
	return 0;
}
