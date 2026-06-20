#include <stdio.h>
#include <string.h>

int main() 
{
    int num = 45;
    int *p = &num;

    struct code {
        int num;
        char *name;
    };

    struct code *data;
    data->name = "lovelace";
    data->num = 4;



   if((printf("---\t%d @:%p\t---\n",num,p)) < 0){
        fprintf(stderr, "printf() failed.\n");
   }
   if((printf("---\t%d @:%d\t---\n",num,*p)) < 0){
        fprintf(stderr, "printf() failed.\n");
   }
   if((printf("---\t%s ---%ld @:%d ---%ld\t @:%p\t @:%ld\t---\n",data->name, sizeof(data->name), data->num, sizeof(data->num), data, sizeof(*data))) < 0){
        fprintf(stderr, "printf() failed.\n");
   }

    return 0;
}