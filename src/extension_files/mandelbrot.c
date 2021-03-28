#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <math.h>
#include <pthread.h>
#define WIDTH 512
#define HEIGHT 512
#define SCALE 1
#define MOVE 10
#define JULIA 1
#define PRETTY 0
typedef struct{
long double  r,i;
}Complex;

typedef struct{
  Complex center;
  int maxIter;
  long double range;
  bool translated;
  Complex **area;
  int **itermap;
  int isJulia;
}Mandelbrot;


typedef struct{
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Event event;
}Screen;
typedef struct{
  Mandelbrot *m;
  int ***map_sect; 
  int w;
  int h;
  int size;
}thread_package;

Complex sqr(Complex c){
  Complex sqr = {c.r*c.r -c.i*c.i,2*c.r*c.i};
  return sqr;
}
Complex equation(Complex z,Complex c){
  Complex ret = sqr(z);
  ret.r+=c.r;
  ret.i+=c.i;
  return ret;
}
long double c_abs(Complex c){
  return c.r*c.r +c.i*c.i;
}
long double secretMod(long double x, int m){
  while (x > m)x-=m;
  return x;
}

void recurse(Mandelbrot *m,int w, int h){
  Complex c = m->area[w][h];
  Complex z ={0.0,0.0};
  if (m->isJulia){
    c=m->center;
    z = m->area[w][h];
  }
  for (int i=0; i< m->maxIter;i++){
    z = equation(z,c);
    if (c_abs(z)>4.0){
      m->itermap[w][h] = i;
      return;
    }
  }
  //m->area[w][h].r=z.r;
  //m->area[w][h].i=z.i;
  m->itermap[w][h]=m->maxIter;
}
void set(Mandelbrot *m){
  long double real,im;
  for (long double i=0.0;i<WIDTH;i++){
    for (long double j=0.0;j<HEIGHT;j++){
      real = (m->center.r)*(1-m->isJulia) + (m->range*(i/WIDTH - 0.5));
      im = (m->center.i)*(1-m->isJulia) + (m->range*(j/HEIGHT - 0.5));
      m->area[(int)i][(int)j].r=real;
      m->area[(int)i][(int)j].i=im;
      m->itermap[(int)i][(int)j]=0;
    }
  }
}
void setSingle(Mandelbrot *m, int w, int h){
    long double x,y,real,im;
    x= (long double) w;
    y= (long double) h;
    real = (m->center.r)*(1-m->isJulia) + (m->range*(x/WIDTH - 0.5));
    im = (m->center.i)*(1-m->isJulia) + (m->range*(y/HEIGHT - 0.5));
    m->area[w][h].r=real;
    m->area[w][h].i=im;
    m->itermap[w][h]=0;
}
void calcRecursive(Mandelbrot *m,int s_x,int s_y, int size){
  if (size<=4){
    for (int i=0;i<size;i++){
      for (int j=0;j<size;j++){
        recurse(m,s_x+i,s_y+j);
      }
    }
    return;
  }
  recurse(m,s_x,s_y);
  int sameValue = m->itermap[s_x][s_y];
  bool same = true;
  for (int y=0; y<size;y++){
    if (!m->itermap[s_x][s_y+y])recurse(m,s_x,s_y+y);
    same =same && (sameValue == m->itermap[s_x][s_y+y]);
  }

  for (int x=1; x<size;x++){
    if (!m->itermap[s_x+x][s_y]) recurse(m,s_x+x,s_y);
    same =same && (sameValue == m->itermap[s_x+x][s_y]);
  }

  for (int y=1; y<size;y++){
    if (!m->itermap[s_x+size-1][s_y])recurse(m,s_x+size-1,s_y+y);
    same =same && (sameValue == m->itermap[s_x+size-1][s_y+y]);
  }

  for (int x=1; x<size-1;x++){
    if (!m->itermap[s_x+x][s_y+size-1])recurse(m,s_x+x,s_y+size-1);
    same =same && (sameValue == m->itermap[s_x+x][s_y+size-1]);
  }
  if (same){
    for (int i=0;i<size;i++){
      for (int j=0;j<size;j++){
        m->itermap[i+s_x][j+s_y] = sameValue;
      }
    }
    return;
  }
  int ns = size/2;
  calcRecursive(m,s_x,s_y,ns);
  calcRecursive(m,s_x+ns,s_y,ns);
  calcRecursive(m,s_x,s_y+ns,ns);
  calcRecursive(m,s_x+ns,s_y+ns,ns);
}
void *RthreadDepack(void *mand){ 
  Mandelbrot *m = (Mandelbrot *) mand;
  calcRecursive(m,0,0,WIDTH);
  return (void *)NULL;
}
void calcThreading(Mandelbrot *m, Mandelbrot *j){
  pthread_t thread_m,thread_j; 
  /*int ***map = calloc(4,sizeof(int **));
  for (int i=0;i<4;i++){
    map[i] = calloc(WIDTH,sizeof(int *));
    for (int j=0;i<WIDTH;i++){
      map[i][j] = calloc(HEIGHT,sizeof(int));
    }
  }*/
  pthread_create(&thread_m,NULL,RthreadDepack,(void *)m);
  pthread_create(&thread_j,NULL,RthreadDepack,(void *)j);
  pthread_join(thread_m,NULL);
  pthread_join(thread_j,NULL);
}

void calc(Mandelbrot *m, Mandelbrot *j, int JustJ){
  if (JULIA==0){
    set(m);
    calcRecursive(m,0,0,WIDTH);
    return;
  }
  j->center = m->center;
  j->maxIter = m->maxIter;
  set(j);
  if (JustJ) calcRecursive(j,0,0,WIDTH);
  else{
    set(m); 
    calcThreading(m,j);
  }
}
void calc_orig(Mandelbrot *m,Mandelbrot *_, int __){
  set(m);
  for (int i=0;i<WIDTH;i++){
    for (int j=0;j<HEIGHT;j++){
    }
  }
}


void init_mandel(Mandelbrot *m,long double r,long double i,long double range,int maxIter){
  
  m->center.r = r;
  m->center.i = i;
  m->maxIter = maxIter;
  m->range = range;
  m->area = calloc(WIDTH,sizeof(Complex *));
  m->itermap = calloc(WIDTH,sizeof(int *));
  for (int i =0;i<WIDTH; i++){
    m->area[i] = calloc(HEIGHT,sizeof(Complex));
    m->itermap[i] = calloc(HEIGHT,sizeof(int));
  }
}
void init_julia(Mandelbrot *j){
  j->isJulia = 1;
  init_mandel(j,0,0,2,50);
}

Screen *init_SDL(){
  Screen *sdl = calloc(1,sizeof(Screen));
  SDL_Init(SDL_INIT_VIDEO);
  sdl->window = SDL_CreateWindow("Mandelbrot/Julia Set",SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,SCALE*WIDTH*(1+JULIA),SCALE*HEIGHT,SDL_WINDOW_OPENGL);
  sdl->renderer = SDL_CreateRenderer(sdl->window,-1,SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  return sdl;
}
void Destroy_SDL(Screen *sdl){
  SDL_DestroyRenderer(sdl->renderer);
  SDL_DestroyWindow(sdl->window);
  SDL_Quit();
}
void get_colours(int *r,int *g, int *b, int iter){
  //long double x =secretMod(((long double) iter)/255,16); 
  *r = (iter)%255;
  *g=10;
  *b=25;
  int ColourScheme[29][3]= {
      {29,10,66},{23,13,77},{20,16,88},{15,18,99},{10,22,110},
      {4,24,121},{26,60,143},{50,100,173},{85,130,203},{102,160,223},
      {122,195,246},{135,205,248},{150,215,250},{165,225,252},{180,235,254},
      {193,255,255},{203,255,255},{216,255,255},{230,255,255},{243,255,255},
      {255,255,255},{252,252,230},{250,250,215},{248,247,200},{246,245,190},
      {245,243,182},//{,,},{,,},{,,},{,,},
      {251,211,38},//{,,},{,,},{,,},{,,},
      {255,142,0},//{,,},{,,},{,,},{,,},
      {97,24,41}//,{,,},{,,},{,,},{,,}
    };
    *r=ColourScheme[iter%29][0];
    *g=ColourScheme[iter%29][1];
    *b=ColourScheme[iter%29][2];
}
void draw(Screen *sdl,Mandelbrot *m,Mandelbrot *julia){
  //clear screen to black
  int r=0,g=0,b=0,a=255;
  SDL_SetRenderDrawColor(sdl->renderer,r,g,b,a);
  SDL_RenderClear(sdl->renderer);
  for (int i=0;i<WIDTH*SCALE;i++){
    for (int j=0;j<HEIGHT*SCALE;j++){
      //for the mandelbrot
      get_colours(&r,&g,&b,m->itermap[i/SCALE][j/SCALE]);
      if (m->itermap[i/SCALE][ j/SCALE]>=m->maxIter) r=g=b=0;
      SDL_SetRenderDrawColor(sdl->renderer,r,g,b,a);
      SDL_RenderDrawPoint(sdl->renderer,i,j);
      //for the julia set
      if (JULIA){
        get_colours(&r,&g,&b,julia->itermap[i/SCALE][ j/SCALE]);
        if (julia->itermap[i/SCALE][j/SCALE]>=julia->maxIter) r=g=b=0;
        SDL_SetRenderDrawColor(sdl->renderer,r,g,b,a);
        SDL_RenderDrawPoint(sdl->renderer,i+WIDTH*SCALE,SCALE*HEIGHT-1-j);
      }
    }
  }
  //show what was drawn
  SDL_RenderPresent(sdl->renderer);
}

void translate_left(Mandelbrot *m){
    m->center.r-= m->range*((long double) MOVE)/((long double) WIDTH);
    for(int i=WIDTH-1;i>MOVE-1;i--){
        //m->area[i] = m->area[i-1];
        //m->itermap[i]=m->itermap[i-1];
        for (int j=0;j<HEIGHT;j++){
            m->area[i][j].r = m->area[i-MOVE][j].r;
            m->area[i][j].i = m->area[i-MOVE][j].i;
            m->itermap[i][j]= m->itermap[i-MOVE][j];
        }
    }
    for (int i=0;i<MOVE;i++){
      for (int j=0;j<HEIGHT;j++){
        setSingle(m,i,j);
        recurse(m,i,j);
      }
    }
}
void translate_right(Mandelbrot *m){
    m->center.r+=m->range*((long double) MOVE)/((long double) WIDTH);
    for(int i=0;i<WIDTH-MOVE;i++){
        //m->area[i] = m->area[i+1];
        //m->itermap[i]=m->itermap[i+1];
        for (int j=0;j<HEIGHT;j++){
            m->area[i][j].r = m->area[i+MOVE][j].r;
            m->area[i][j].i = m->area[i+MOVE][j].i;
            m->itermap[i][j]=m->itermap[i+MOVE][j];
        }
    }
    for (int i=0;i<MOVE;i++){
      for (int j=0;j<HEIGHT;j++){
        setSingle(m,WIDTH-1-i,j);
        recurse(m,WIDTH-1-i,j);
      }
    }
}
void translate_up(Mandelbrot *m){
    m->center.i-=m->range*((long double) MOVE)/((long double) HEIGHT);

    for (int j = HEIGHT-1; j >= MOVE ; j--){
        for (int i = 0; i < WIDTH; i++){
            m->area[i][j].r = m->area[i][j-MOVE].r;
            m->area[i][j].i = m->area[i][j-MOVE].i;
            m->itermap[i][j] = m->itermap[i][j-MOVE];
        }
    }
    for (int j=0;j<MOVE;j++){
      for (int i = 0; i < WIDTH ;i++){
        setSingle(m,i,j);
        recurse(m,i,j);
      }
    }
}
void translate_down(Mandelbrot *m){
    m->center.i+=m->range*((long double) MOVE)/((long double) HEIGHT);
    for (int j = 0; j<HEIGHT-MOVE; j++){
        for (int i = 0; i < WIDTH; i++){
            m->area[i][j].r = m->area[i][j+MOVE].r;
            m->area[i][j].i = m->area[i][j+MOVE].i;
            m->itermap[i][j] = m->itermap[i][j+MOVE];
        }
    }
    for (int j=0;j<MOVE;j++){
      for (int i = 0; i < WIDTH ;i++){
        setSingle(m,i,HEIGHT-1-j);
        recurse(m,i,HEIGHT-1-j);
      }
    }
}


int main(int argc,char **argv){
  Mandelbrot *m = malloc(sizeof(Mandelbrot)),*j = malloc(sizeof(Mandelbrot));

  if (PRETTY) init_mandel(m,-0.734955,-0.197049,0.001,500);
  else init_mandel(m,-0.76,-0.24,1,50);
  init_julia(j);

  calc(m,j,0);
  Screen *main = init_SDL();
  draw(main,m,j);
  int running=1;  
  while (running){
    while (SDL_PollEvent(&main->event)){
      const Uint8* state = SDL_GetKeyboardState(NULL);
      if (state[SDL_SCANCODE_Q]){
        running = 0;
        break;
      }
      if (state[SDL_SCANCODE_M]) m->maxIter*=0.9;
      if (state[SDL_SCANCODE_V]) printf("%Lf, %Lf %d %10Lf\n",m->center.r,m->center.i,m->maxIter,m->range);
      
      
      if (state[SDL_SCANCODE_W])translate_up(m);//m->center.i+=m->range*((long double) MOVE)/((long double) HEIGHT);
      else if (state[SDL_SCANCODE_A])translate_left(m);//m->center.r-=m->range*((long double) MOVE)/((long double) WIDTH);//
      else if (state[SDL_SCANCODE_S])translate_down(m);//m->center.i-=m->range*((long double) MOVE)/((long double) HEIGHT);//
      else if (state[SDL_SCANCODE_D])translate_right(m);//m->center.r+=m->range*((long double) MOVE)/((long double) WIDTH);//

      else {
        if (state[SDL_SCANCODE_P]){
          m->range*=0.5;
          j->range*=0.9;
        }
        else if (state[SDL_SCANCODE_O]){
          m->range*=2;
          j->range*=1.1;
        }
        else if (state[SDL_SCANCODE_N]) m->maxIter*=1.1; 
        else break;
        calc(m,j,0);
      }
      if (state[SDL_SCANCODE_J])calc(m,j,1);
      draw(main,m,j);
    }
  }
  Destroy_SDL(main);
  return EXIT_SUCCESS;
}