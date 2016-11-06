#include <stdio.h>

int main() {
  int fsize = 296, parens;
  char mem[300000], *ptr = mem;
  for(int i = 0; i < 300000; i++) mem[i] = 0;
  char *str = ">++++++++++>>>+>+[>>>+[-[<<<<<[+<<<<<]>>[[-]>[<<+>+>-]<[>+<-]<[>+<-[>+<-[>+<-[>+<-[>+<-[>+<-[>+<-[>+<-[>+<-[>[-]>>>>+>+<<<<<<-[>+<-]]]]]]]]]]]>[<+>-]+>>>>>]<<<<<[<<<<<]>>>>>>>[>>>>>]++[-<<<<<]>>>>>>-]+>>>>>]<[>++<-]<<<<[<[>+<-]<<<<]>>[->[-]++++++[<++++++++>-]>>>>]<<<<<[<[>+>+<<-]>.<<<<<]>.>>>>]";
  for(int i = 0; i < 296; i++) {
    if(str[i] == '+') {
      ++*ptr;
    } else if(str[i] == '-') {
      --*ptr;
    } else if(str[i] == '<') {
      ptr--;
    } else if(str[i] == '>') {
      ptr++;
    } else if(str[i] == '.') {
      putchar(*ptr);
    } else if(str[i] == ',') {
      *ptr = getchar();
    } else if(str[i] == '[') {
      if (*ptr == 0) {
        parens = 1;
        while(parens != 0 && i < fsize) {
          i++;
          if (str[i] == '[')
            parens++;
          if (str[i] == ']')
            parens--;
        } 
      }
    } else if(str[i] == ']') {
      if (*ptr != 0) {
        parens = 1;
        while(parens != 0) {
          i--;
          if (str[i] == ']')
            parens++;
          if (str[i] == '[')
            parens--;
        } 
      }
    }
  }

  return 0;
}
