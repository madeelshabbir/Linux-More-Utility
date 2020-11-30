#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<termios.h>
#include<string.h>
#include<wait.h>
int terLines(){
	struct winsize sz;
	ioctl(STDIN_FILENO, TIOCGWINSZ, &sz);
	return sz.ws_row - 1;
}
struct termios sav;
struct termios nCanEc(){
	tcgetattr(STDIN_FILENO, &sav);
	struct termios trm;
	tcgetattr(STDIN_FILENO, &trm);
	trm.c_lflag &= ~(ICANON|ECHO);
	trm.c_cc[VMIN] = 1;
	trm.c_cc[VTIME] = 0;
	return trm;
}
int totLn(int argc, char* argv[]){
	int count = 0;
	FILE* f;
	char buf[512];
	if(argc > 2)
		count += 3 * (argc - 1);
	else if(argc == 1){
		if((f = fopen(".tmp.txt", "w")) != NULL){
			while(fgets(buf, 512, stdin) != NULL){
				fputs(buf, f);
				count++;
			}
			fclose(f);
		}
		return count;
	}
	for(int i = 1; i<argc; i++){
		if((f = fopen(argv[i],"r")) != NULL){
			while(fgets(buf,512,f) != NULL)
				count++;
			fclose(f);
		}
	}
	return count;
}
int curPer(int c, int t){
	return c*100/t;
}
void printFileName(char* n){
	printf("::::::::::::::::\n");
	printf("%s\n",n);
	printf("::::::::::::::::\n");
}
void printMore(FILE* f, int n, bool b,int curLen,int totLen, bool p, char* fName){
	struct termios at = nCanEc();
	char buf[512];
	int i = n;
	if(b)
		i -= 3;
	curLen += i;
	char blast[512];
	char bslast[512];
	char stop[512];
	int iter = 0, stopC = 0;
	bool m = true, v = true, pr = true;
	FILE* trml = fopen("/dev/tty", "r");
	char wst[512];
	while(fgets(buf, 512, f)!= NULL){
		if(m && v)
			fputs(buf, stdout);
		v = true;
		i--;
		if(i == 0){
			IVINP:
			if(m)
				printf("\033[7m--MORE--(%d%)\033[m",curPer(curLen, totLen));	
			m = true;
			tcsetattr(STDIN_FILENO,TCSAFLUSH,&at);
			char c = getc(trml);
			tcsetattr(STDIN_FILENO, TCSANOW,&sav);
			if(p)
				printf("\033[1A");
			pr = true;
			printf("\033[2K \033[1G");
			if( c == 'q')
				exit(0);
			else if(c == '\n'){
				i = 1;
				curLen+=i;
			}
			else if(c == ' '){
				i = n;
				curLen+=i;
			}
			else if(c == '/'){
				stopC = curLen;
				printf("/");
				char pat[512];
			        fgets(pat, 512,trml);
				if(p)
					fputs(pat, stdout);
				bool found = false;
				while(fgets(buf, 512,f) != NULL){
					iter++;
					curLen++;
					if(strstr(buf,pat)){
						found = true;
						printf("...skipping\n");
						i = n - 1;
						if(iter == 1){
							i -= 1;
							fputs(blast,stdout);
						}
						else if(iter != 0){
							i -= 2;
							fputs(bslast,stdout);
							fputs(blast,stdout);
						}
						curLen+=i;
						break;
					}
					strcpy(bslast, blast);
				        strcpy(blast, buf);
				}
				if(!found){
					printf("\033[7mPattern not found\033[m");
					m = false;
					curLen = stopC;
					fclose(f);
					f = fopen(fName, "r");
					for(int k = 0; k<curLen - 1; k++)
						fgets(wst,512, f);
					i = 1;
				}

			}
			else if(c == 'v'){
				if(p){
					getc(trml);
					goto IVINP;
				}
				int status;
				if(fork() == 0){
					execl("/usr/bin/vim","vim",fName,NULL);
					exit(0);
				}
				else{
					fclose(f);
					wait(&status);
					printf("vi -c %s--------------\n",fName);
					f = fopen(fName,"r");
					for(int k = 0; k < curLen - 1; k++)
						fgets(wst,512,f);
					i = 1;
					v = false;
					if(p)
						m = true;
				}
			}
			else{
				if(p)
					getc(trml);
				goto IVINP;
			}
		}
	}
	fclose(trml);
	remove(".tmp1.txt");
}
int main(int argc, char* argv[]){
	int scrnLen = terLines();
	int curLen = 0;
	int totLen = totLn(argc, argv);
	if(argc == 1){
		printMore(fopen(".tmp.txt", "r"), 22, false, curLen, totLen, true,".tmp.txt");
		remove(".tmp.txt");
		exit(0);
	}
	FILE* file;
	for(int i = 1; i < argc; i++){
		if((file = fopen(argv[i], "r")) != NULL){
			bool b = false;
			if(argc > 2){
				printFileName(argv[i]);
				b = true;
			}
			printMore(file, scrnLen, b, curLen, totLen, false, argv[i]);
		
			fclose(file);	
		}
	}
	exit(0);
}
