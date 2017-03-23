#include "common.h"

void Extract(char *substr,int p,int orig){
	uint32_t rest;
	rest=(uint32_t)orig;
	char rev[50];
	char *current=rev;
	if(rest==0){
		*(substr+1)='\0';
		*substr='0';
		return;
	}
	while(rest!=0){
		int remin=rest%p;
		if(remin<10){
			*current='0'+remin;
		}else{
			*current='a'+remin-10;
		}
		rest=rest/p;
		current++;
	}
	current--;
	while(current!=rev-1){
		*substr=*current;
		substr++;
		current--;
	}
	*substr='\0';
}

/* implement this function to support printk */
void vfprintf(void (*printer)(char), const char *ctl, void **args) {
	char c;
	for(c=*ctl;*ctl!='\0';ctl++,c=*ctl){
		if(c=='%'&&*(ctl+1)!='\0'){
			ctl++;
			char substr[50];
			if(*ctl=='x'){
				Extract(substr,16,*(int*)args);
				char *m;
				for(m=substr;*m!='\0';m++){
					printer(*m);
				}
				args++;
			}else if(*ctl=='d'){
				Extract(substr,10,*(int*)args);
				char *m;
				for(m=substr;*m!='\0';m++){
					printer(*m);
				}
				args++;
			}else if(*ctl=='s'){
				char *m=*(char **)args;
				while(*m!='\0'){
					printer(*m);
					m++;
				}
				args++;
			}else if(*ctl=='c'){
				printer(*(char*)args);
				args++;
			}else{
				printer('%');
				args++;
			}
		}else{
			printer(c);
		}
	}
}

extern void serial_printc(char);

/* __attribute__((__noinline__))  here is to disable inlining for this function to avoid some optimization problems for gcc 4.7 */
void __attribute__((__noinline__)) 
printk(const char *ctl, ...) {
	void **args = (void **)&ctl + 1;
	vfprintf(serial_printc, ctl, args);
}
