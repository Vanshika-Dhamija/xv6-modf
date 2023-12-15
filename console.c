// Console input and output.
// Input is from the keyboard or serial port.
// Output is written to the screen and serial port.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "traps.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"

#define UP 226
#define DOWN 227

static void consputc(int);
int consoleread(struct inode *ip, char *dst, int n);
static int panicked = 0;

static struct {
  struct spinlock lock;
  int locking;
} cons;

static void
printint(int xx, int base, int sign)
{
  static char digits[] = "0123456789abcdef";
  char buf[16];
  int i;
  uint x;

  if(sign && (sign = xx < 0))
    x = -xx;
  else
    x = xx;

  i = 0;
  do{
    buf[i++] = digits[x % base];
  }while((x /= base) != 0);

  if(sign)
    buf[i++] = '-';

  while(--i >= 0)
    consputc(buf[i]);
}
//PAGEBREAK: 50

// Print to the console. only understands %d, %x, %p, %s.
void
cprintf(char *fmt, ...)
{
  int i, c, locking;
  uint *argp;
  char *s;

  locking = cons.locking;
  if(locking)
    acquire(&cons.lock);

  if (fmt == 0)
    panic("null fmt");

  argp = (uint*)(void*)(&fmt + 1);
  for(i = 0; (c = fmt[i] & 0xff) != 0; i++){
    if(c != '%'){
      consputc(c);
      continue;
    }
    c = fmt[++i] & 0xff;
    if(c == 0)
      break;
    switch(c){
    case 'd':
      printint(*argp++, 10, 1);
      break;
    case 'x':
    case 'p':
      printint(*argp++, 16, 0);
      break;
    case 's':
      if((s = (char*)*argp++) == 0)
        s = "(null)";
      for(; *s; s++)
        consputc(*s);
      break;
    case '%':
      consputc('%');
      break;
    default:
      // Print unknown % sequence to draw attention.
      consputc('%');
      consputc(c);
      break;
    }
  }
  if(locking)
    release(&cons.lock);
}

void
panic(char *s)
{
  int i;
  uint pcs[10];

  cli();
  cons.locking = 0;
  // use lapiccpunum so that we can call panic from mycpu()
  cprintf("lapicid %d: panic: ", lapicid());
  cprintf(s);
  cprintf("\n");
  getcallerpcs(&s, pcs);
  for(i=0; i<10; i++)
    cprintf(" %p", pcs[i]);
  panicked = 1; // freeze other CPU
  for(;;)
    ;
}

//PAGEBREAK: 50
#define BACKSPACE 0x100
#define CRTPORT 0x3d4
static ushort *crt = (ushort*)P2V(0xb8000);  // CGA memory

static void
cgaputc(int c)
{
  int pos;

  // Cursor position: col + 80*row.
  outb(CRTPORT, 14);
  pos = inb(CRTPORT+1) << 8;
  outb(CRTPORT, 15);
  pos |= inb(CRTPORT+1);

  if(c == '\n')
    pos += 80 - pos%80;
  else if(c == BACKSPACE){
    if(pos > 0) --pos;
  } else
    crt[pos++] = (c&0xff) | 0x0700;  // black on white

  if(pos < 0 || pos > 25*80)
    panic("pos under/overflow");

  if((pos/80) >= 24){  // Scroll up.
    memmove(crt, crt+80, sizeof(crt[0])*23*80);
    pos -= 80;
    memset(crt+pos, 0, sizeof(crt[0])*(24*80 - pos));
  }

  outb(CRTPORT, 14);
  outb(CRTPORT+1, pos>>8);
  outb(CRTPORT, 15);
  outb(CRTPORT+1, pos);
  crt[pos] = ' ' | 0x0700;
}

void
consputc(int c)
{
  if(panicked){
    cli();
    for(;;)
      ;
  }

  if(c == BACKSPACE){
    uartputc('\b'); uartputc(' '); uartputc('\b');
  } else
    uartputc(c);
  cgaputc(c);
}

#define INPUT_BUF 128
struct {
  char buf[INPUT_BUF];
  //int size;
  uint r;  // Read index
  uint w;  // Write index
  uint e;  // Edit index
} input;

char array[16][128];

int num=0;
int current=0;

#define C(x)  ((x)-'@')  // Control-x

void
consoleintr(int (*getc)(void))
{
  int c, doprocdump = 0;

  acquire(&cons.lock);
  while((c = getc()) >= 0){
    switch(c){
    case C('P'):  // Process listing.
      // procdump() locks cons.lock indirectly; invoke later
      doprocdump = 1;
      break;
    case C('U'):  // Kill line.
      while(input.e != input.w &&
            input.buf[(input.e-1) % INPUT_BUF] != '\n'){
        input.e--;
        consputc(BACKSPACE);
      }
      break;
    case C('H'): case '\x7f':  // Backspace
      if(input.e != input.w){
        input.e--;
        consputc(BACKSPACE);
      }
      break;
      
    case UP: 
    	if(current<0){}
    	else if(current==0){
    		//current=-1;
    	}
    	else{
    		for(int i= 0; i<strlen(array[current]); i++){
    			consputc(BACKSPACE);
    		}
    		current--;
    		//cprintf("%s", array[current]);
    		for(int i=0; i< strlen(array[current]); i++){
    			consputc(array[current][i]);
    		}
    		//consputc('\n');
    		
    		//strncpy(input.buf, array[current], strlen(array[current]));
    		//consputc(strlen(array[current]));
    		//input.buf[strlen(array[current])]='\0';
    		/*for(int i=0; i<strlen(input.buf); i++){
    			consputc('b');
    		}*/
    		
    	}   
      break;
    case DOWN: 
    	if(current>num){}
    	else if(current==num){
    		for(int i= 0; i<strlen(array[current]); i++){
    			consputc(BACKSPACE);
    		}
    		//input.buf[strlen(input.buf)]='\0';
    		current=num;
    	}
    	else{
	    	for(int i= 0; i<strlen(input.buf); i++){
	    		consputc(BACKSPACE);
	    	}
    		current++;
    		//cprintf("%s", array[current]);
    		for(int i=0; i< strlen(array[current]); i++){
    			consputc(array[current][i]);
    		}
    		//strncpy(input.buf, array[current], strlen(array[current]));
    		//input.buf[strlen(array[current])]='\0';
    		
    	}   
      break;
      case '\n':
    default:
      
      if(c != 0 && input.e-input.r < INPUT_BUF){
        c = (c == '\r') ? '\n' : c;
        input.buf[input.e++ % INPUT_BUF] = c;
        consputc(c);
        if(c == '\n' || c == C('D') || input.e == input.r+INPUT_BUF){ 
           char s[128]; 
           int j=0;
           /*for(int i= 0;i<input.e; i++){
           	consputc('e');
           }
           for(int i= 0;i<input.r; i++){
           	consputc('r');
           }
           for(int i= 0;i<input.w; i++){
           	consputc('w');
           }
           for(int i= 0;i<strlen(input.buf); i++){
           	consputc(input.buf[i]);
           }*/
           for(int i= input.w;i<strlen(input.buf)-1; i++){
           	s[j++]=input.buf[i];
           }
           s[j]='\0';
           /*for(int i=0; i<j; i++){
           	consputc(s[i]);
           }*/
           if(j>0){
           		//consputc('J');
	      		if(num<16){
	      			//consputc('A');
	      			/*strncpy(array[num], s, j-1);
	      			array[num][j-1]='\0';*/
	      			//array[num]=s;
	      			for(int i=0; i<j; i++){
	      				array[num][i]= s[i];
	      			}
	      			num++;
	      			current= num;
	      			
	      		}
	      		else{
	      			//consputc('B');
	      			for(int i=0; i<15; i++){
	      				//strncpy(array[i], array[i+1], strlen(array[i+1]));
	      				//array[i][strlen(array[i+1])]='\0';
	      				//array[i]= array[i+1];
	      				for(int j=0; j<strlen(array[i+1]); i++){
	      					array[i][j]=array[i+1][j];
	      				}
	      			}
	      			//strncpy(array[15], s, strlen(s));
	      			//array[num][strlen(s)]='\0';
	      			//array[15]=s;
	      			for(int i=0; i<j; i++){
	      				array[15][i]= s[i];
	      			}
	      			
	      			current= 16;
	      		}
	      	}
           input.w = input.e;
           wakeup(&input.r);
        }
         
          
          /*else if(c=='\n' || c=='\r'){
	      	if(strlen(input.buf)>0){
	      		if(num<16){
	      			strncpy(array[num], input.buf, strlen(input.buf));
	      			array[num][strlen(input.buf)]='\0';
	      			current= num;
	      			num++;
	      		}
	      		else{
	      			for(int i=0; i<15; i++){
	      				strncpy(array[i], array[i+1], strlen(array[i+1]));
	      				array[i][strlen(array[i+1])]='\0';
	      			}
	      			strncpy(array[15], input.buf, strlen(input.buf));
	      			array[num][strlen(input.buf)]='\0';
	      			current= 16;
	      		}
	      	}
	      	//running the console??
	   }*/   
       }
      break;
    }
  }
  release(&cons.lock);
  if(doprocdump) {
    procdump();  // now call procdump() wo. cons.lock held
  }
}

int
consoleread(struct inode *ip, char *dst, int n)
{
  uint target;
  int c;

  iunlock(ip);
  target = n;
  acquire(&cons.lock);
  while(n > 0){
    while(input.r == input.w){
      if(myproc()->killed){
        release(&cons.lock);
        ilock(ip);
        return -1;
      }
      sleep(&input.r, &cons.lock);
    }
    c = input.buf[input.r++ % INPUT_BUF];
    if(c == C('D')){  // EOF
      if(n < target){
        // Save ^D for next time, to make sure
        // caller gets a 0-byte result.
        input.r--;
      }
      break;
    }
    *dst++ = c;
    --n;
    if(c == '\n')
      break;
  }
  release(&cons.lock);
  ilock(ip);

  return target - n;
}

int
consolewrite(struct inode *ip, char *buf, int n)
{
  int i;

  iunlock(ip);
  acquire(&cons.lock);
  for(i = 0; i < n; i++)
    consputc(buf[i] & 0xff);
  release(&cons.lock);
  ilock(ip);

  return n;
}

void
consoleinit(void)
{
  initlock(&cons.lock, "console");

  devsw[CONSOLE].write = consolewrite;
  devsw[CONSOLE].read = consoleread;
  cons.locking = 1;
  /*for(int i=0; i<16; i++){
	array[i]="ls";
	cprintf(array[i]);
   }
   current=16;*/
  
  //struct inode ip;
  //consolewrite(&ip, "Num in histor", 13);
  
  ioapicenable(IRQ_KBD, 0);
}


int history(char * buffer, int historyId){
	/*consputc('\n');
	for(int i=0; i<num; i++){
		consputc('t');
	}
	consputc('\n');
	for(int i=0; i<historyId; i++){
		consputc('h');
	}
	consputc('\n');
	*/
	if(historyId<0 || historyId>15){
		return -2;
	}
	if(historyId>=num){
		return -1;
	}
	//buffer= array[historyId];
	//consputc(' ');
	if(strlen(array[historyId])>1){
		for(int i=0; i<strlen(array[historyId]); i++){
			consputc(array[historyId][i]);
		}
	}
	
	return 0;
}
