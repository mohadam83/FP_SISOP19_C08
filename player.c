#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <termios.h>
#include <dirent.h>
#include <ao/ao.h>
#include <mpg123.h>
#include <pthread.h>
#define BITS 8

pthread_t tid1,tid2;
char file[60];
char folder[60]; 
int scene;

typedef struct
{
    char filename[60];
}fname;

fname playlist[60];

int isifolder(void *arg[])
{
    int n=0, i=0;
    DIR *d;
    struct dirent *dir;
    d = opendir(folder);

    //Determine the number of files
    while((dir = readdir(d)) != NULL) 
    {
        if ( !strcmp(dir->d_name, ".") || !strcmp(dir->d_name, "..") )
        {

        } 
        else 
        {
            n++;
        }
    }

    rewinddir(d);

    //Put file names into the array
    while((dir = readdir(d)) != NULL) 
    {
        if ( !strcmp(dir->d_name, ".") || !strcmp(dir->d_name, "..") )
        {}
        else 
        {
            strncpy (playlist[i].filename,dir->d_name, strlen(dir->d_name) );
            i++;
        }
    }

    rewinddir(d);

    for(int i=0; i<n; i++)
    {
        printf("%d. %s\n", i, playlist[i].filename);
    }

}

void* player(void *arg)
{
    mpg123_handle *mh;
    unsigned char *buffer;
    size_t buffer_size;
    size_t done;
    int err;

    int driver;
    ao_device *dev;

    ao_sample_format format;
    int channels, encoding;
    long rate;


    /* initializations */
    ao_initialize();
    driver = ao_default_driver_id();
    mpg123_init();
    mh = mpg123_new(NULL, &err);
    buffer_size = mpg123_outblock(mh);
    buffer = (unsigned char*) malloc(buffer_size * sizeof(unsigned char));

    /* open the file and get the decoding format */
    mpg123_open(mh, file);
    mpg123_getformat(mh, &rate, &channels, &encoding);

    /* set the output format and open the output device */
    format.bits = mpg123_encsize(encoding) * BITS;
    format.rate = rate;
    format.channels = channels;
    format.byte_format = AO_FMT_NATIVE;
    format.matrix = 0;
    dev = ao_open_live(driver, &format, NULL);

    while(1)
    {
        if(scene == 0) //pause 
        {
            sleep(0);
        }
        else if(scene == 1) //resume
        { 
            mpg123_read(mh, buffer, buffer_size, &done);
            ao_play(dev, buffer, done);
        }
        else if(scene==2) //stop
        {
            break;
        }
        else if (scene==3) 
        {
            free(buffer);
            ao_close(dev);
            mpg123_close(mh);
            mpg123_delete(mh);
            mpg123_exit();
            ao_shutdown();
        }
    }
}




void* menu(void* arg)
{
    int pilihan;
    while(1)
    {
        scanf("%d", &pilihan);
        if(pilihan == 1) //play
        {
            scene = 2;
            scene = 0;

            isifolder(folder);
            
            int pil;
            
            printf("Pilih : \n");
            scanf("%d", &pil);
            
            strcpy(file,playlist[pil].filename);
            
            scene = 1;

            pthread_create(&(tid1), NULL, player, NULL);
        }
        else if(pilihan == 2)
        {
            scene = 0;
        }   
        else if(pilihan == 3)
        {
            scene = 1;
        }
        else if(pilihan == 4)
        {
            scene = 2;
        }
        else if(pilihan == 5)
        {
            isifolder(folder);
        }
    }
}


int main()
{   
    scene = 1;

    scanf("%s", folder);
    
    isifolder(folder);
    int pil;
    
    printf("Choice: ");    
    scanf("%d", &pil);
    
    printf("%s\n", playlist[pil].filename);
    strcpy(file,playlist[pil].filename);
    
    scene = 0;

    pthread_create(&(tid1), NULL, player, NULL);
    pthread_create(&(tid2), NULL, main, NULL);
    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);

    return 0;
}

