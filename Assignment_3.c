#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <time.h>

typedef struct task                                                     //task structure
{
  float WCET;                               
  float deadline;
  float period;                                        
}task_def;

bool print_it;

bool UB_Test(int num_tasks, struct task *task_set);                    //function prototypes
bool RT_Test(int num_tasks, struct task *task_set);
bool EDF_test(int num_tasks, struct task *task_set);
bool EDF_UT_test(int num_tasks, struct task *task_set);
bool RM_Test(int num_tasks, struct task *task_set);
bool RT_Only_Test(int num_tasks, struct task *task_set);
bool effective_utilization(int num_tasks, struct task *task_set);



int compare_period (const void * elem1, const void * elem2)            //compare functions to sort
{
    task_def f = *((task_def*)elem1);
    task_def s = *((task_def*)elem2);
    if (f.period > s.period) return  1;
    if (f.period < s.period) return -1;
    return 0;
}

int compare_deadline (const void * elem1, const void * elem2)
{
    task_def f = *((task_def*)elem1);
    task_def s = *((task_def*)elem2);
    if (f.deadline > s.deadline) return  1;
    if (f.deadline < s.deadline) return -1;
    return 0;
}

bool PisequaltoD(int num_tasks, struct task *task_set)                //function to check if period is equal to deadline
{
    for(int j = 0; j < num_tasks; j++)
    {
        if(task_set[j].period != task_set[j].deadline)
            return 0;
    }
    return 1;
}

bool EDF_UT_test(int num_tasks, struct task *task_set)                //EDF utilization test
{
    float UT = 0;
    for(int j = 0; j < num_tasks; j++)
    {
        UT += task_set[j].WCET/task_set[j].deadline;
    }
    
    if(UT < 1.0)
    {
        if(print_it)
            printf("Task set is schedulable by EDF from Utilization test\n");
        
        return 1;
    }
    else
        return 0;
}


bool EDF_test(int num_tasks, struct task *task_set)                   //EDF general test including loading factor analysis
{
    if(PisequaltoD(num_tasks, task_set))
    {
        if(EDF_UT_test(num_tasks, task_set))
            return 1;
        else
        {
            if(print_it)
                printf("Task set is not schedulable by EDF from Utilization test\n");
            
            return 0;
        }
    }
    if(EDF_UT_test(num_tasks, task_set))
      return 1;
    int j, k;
    float L1 = 0, L2 = 0, LF = 0;
    float critical_instance = 0;
    for(j = 0; j < num_tasks; j++)
    {
        L1 += task_set[j].WCET;
    }

    while(1)                                                          //calculating busy period(L2)
    {
        L2=0;
        for(j = 0; j < num_tasks; j++)
        {
            L2+= ceil(L1/task_set[j].period)*task_set[j].WCET;
        }
        
        if(L1 == L2)
            break;
        else
            L1=L2;
    }
    
    for(k = 0; k < num_tasks; k++)                                  // checking loading at critical instants
    {
        critical_instance = task_set[k].deadline;
        while(critical_instance <= L2)
        {
            
           for(j = 0; j < num_tasks; j++)
           {
             LF += (1+floor((critical_instance-task_set[j].deadline)/task_set[j].period))*task_set[j].WCET;
           }
           if(LF/critical_instance > 1.0)
           {
               if(print_it)
                    printf("Task set is not schedulable by EDF as task %d missed its deadline at %f\n", (j+1),critical_instance);
               
               return 0;
           }
           LF = 0;
        critical_instance += task_set[k].period;
        }
    }
    
    if(print_it)
        printf("Task set is schedulable by EDF after loading factor test\n");

        return 1;

}

bool RM_Test(int num_tasks, struct task *task_set)                                     //Rate monotonic test
{
    qsort(task_set, num_tasks,sizeof(task_def), compare_period);
    
    if(PisequaltoD(num_tasks, task_set))
        return(UB_Test(num_tasks, task_set));
    else
    {
        bool stillRM = 1;
        struct task new_task_set[num_tasks];
        for(int i=0 ; i < num_tasks; i++)
        {
            new_task_set[i].WCET = task_set[i].WCET;
            new_task_set[i].period = task_set[i].period;
            new_task_set[i].deadline = task_set[i].deadline;
        }
        qsort(new_task_set, num_tasks,sizeof(task_def), compare_deadline);
        for(int i=0 ; i < num_tasks; i++)
        {
            if(new_task_set[i].period != task_set[i].period)
            {
                stillRM = 0;
                break;
            }
        }
        if(stillRM)
            return(UB_Test(num_tasks, task_set));
        else
        {
            if(effective_utilization(num_tasks, task_set))
                return 1;
            else
                return(RT_Only_Test(num_tasks, task_set));
        }
    }
    
}

bool DM_Test(int num_tasks, struct task *task_set)                                   //Deadline monotonic test
{
    qsort(task_set, num_tasks,sizeof(task_def), compare_deadline);
    
    if(PisequaltoD(num_tasks, task_set))
        return(UB_Test(num_tasks, task_set));
    else
    {
        if(effective_utilization(num_tasks, task_set))
            return 1;
        else
            return(RT_Only_Test(num_tasks, task_set));
    }
    
}


bool UB_Test(int num_tasks, struct task *task_set)                                  //Utilization test for RM and DM
{
	float UB = 0, UB_con =0;
	for(int j = 0; j < num_tasks; j++)		
	{
          UB += task_set[j].WCET/task_set[j].deadline;
	}
	UB_con = num_tasks*(powf(2.0,(float)1/num_tasks)-1);
	if(UB <= UB_con)
    {
        if(print_it)
            printf("Task set is schedulable from Utilization and RT combination test\n");
        
        return 1;
    }
	else
	return(RT_Test(num_tasks, task_set));
}



bool RT_Test(int num_tasks, struct task *task_set)                                 //Response-Time Analysis along with UB test
{
      float  A0 = 0, AN =0;
	  for(int j = 0; j < num_tasks; j++)
		   {
		     A0 += task_set[j].WCET;
		   }
	  while(1)
	  {
		  AN = task_set[num_tasks-1].WCET;
		  for(int j = 0; j < num_tasks - 1 ; j++)
		   {
		     AN += ceil(A0/task_set[j].period)*task_set[j].WCET;
		   }
          
		if(AN > task_set[num_tasks-1].period)
          {
            if(print_it)
              printf("Task set is not schedulable as task %d fails RT test\n",(num_tasks - 1));
              
           return 0;
          }
        else if(A0 == AN)
		   break;
		else
		   A0=AN;
	  }
    
    if(print_it)
        printf("Worst case execution time for task %d : %f\n",(num_tasks - 1),AN);

	  if(AN < task_set[num_tasks-1].period)
	    return UB_Test(num_tasks-1, task_set);
	  else
      {
        if(print_it)
          printf("Task set is not schedulable as task %d fails RT test\n",(num_tasks - 1));
        
        return 0;
      }
}

bool effective_utilization(int num_tasks, struct task *task_set)
{
    float ef_util = 0, UB_con = 0;
    int Hn = 1;
    for(int i = 0; i < num_tasks; i++)
    {
        if(task_set[i].period < task_set[num_tasks].deadline)
        {
            ef_util += task_set[i].WCET/task_set[i].deadline;
            Hn++;
        }
        else
            ef_util += task_set[i].WCET/task_set[num_tasks].deadline;
    }
    UB_con = Hn*(powf(2.0,(float)1/Hn)-1);
    if(ef_util < UB_con)
    {
        if(num_tasks == 1)
        {
            return 1;
        }
        else
        {
            return(effective_utilization(num_tasks-1,task_set));
        }
    }
    else
    {
        return 0;
    }
    
}


bool RT_Only_Test(int num_tasks, struct task *task_set)                        //Iterative Response time test
{
    float  A0 = 0, AN =0;
    for(int j = 0; j < num_tasks; j++)
    {
        A0 += task_set[j].WCET;
    }
    while(1)
    {
        AN = task_set[num_tasks-1].WCET;
        for(int j = 0; j < num_tasks - 1 ; j++)
        {
            AN += ceil(A0/task_set[j].period)*task_set[j].WCET;
        }
        
        if(AN > task_set[num_tasks-1].deadline)
        {
            if(print_it)
            printf("Task set is not schedulable from RT test as task %d fails RT test\n",(num_tasks - 1));
            
            return 0;
        }
        else if(A0 == AN)
            break;
        else
            A0=AN;
    }
    
    if(print_it)
        printf("Worst case execution time for task %d : %f\n",(num_tasks - 1),AN);
    
    if(AN < task_set[num_tasks-1].deadline)
    {
        if(num_tasks == 1)
        {
            {
                if(print_it)
                printf("Task set is schedulable from RT test\n");
                
                return 1;
            }
        }
        else
        return RT_Only_Test(num_tasks-1, task_set);
    }
    else
    {
        if(print_it)
        printf("Task set is not schedulable as task %d fails RT test\n",(num_tasks - 1));
        
        return 0;
    }
}

void Utilization (int num_tasks, float util, float* UT)                       //function to generate utilization values for synthetic task generation
{
    float Sum = util;
    float Next_Sum;
    for(int i = 0; i < num_tasks - 1; i++)
    {
        Next_Sum = Sum * (powf(drand48(),((double)(1.0)/(double)(num_tasks-i+1))));
        UT[i] = Sum - Next_Sum;
        Sum = Next_Sum;
    }
    UT[num_tasks - 1] = Sum;
}

void Synthetic_Task_Set(int num_tasks, int type)                              //Synthetic task set generation
{
    int i, TS, count_EDF = 0, count_RM = 0, count_DM = 0;
    float util;
    FILE *fp;
    float UT[num_tasks];
    struct task task_set[num_tasks];
    srand(time(NULL));
    if(num_tasks == 10 && type == 1)
        fp = fopen("type1.txt", "w+");                                        //Files for storing % tasks schedulable with respective utilization
    else if(num_tasks == 25 && type == 1)
        fp = fopen("type2.txt", "w+");
    else if(num_tasks == 10 && type == 0)
        fp = fopen("type3.txt", "w+");
    else if(num_tasks == 25 && type == 0)
        fp = fopen("type4.txt", "w+");
    
    for(util = 0.05; util <= 0.99; util+=0.1)                                //Generating Random tasks
    {
        count_EDF = count_RM = count_DM = 0;
        for(TS = 1; TS <= 5000; TS++)
        {
            Utilization(num_tasks,util, UT);
            for(i = 0; i < num_tasks; i++)
            {
                if(i < num_tasks/3)
                {
                    task_set[i].period = (rand() % 9000) + 1000;
                }
                else if(i < (num_tasks*2)/3)
                {
                    task_set[i].period = (rand() % 90000) + 10000;
                }
                else
                {
                    task_set[i].period = (rand() % 900000) + 100000;
                }
                
                task_set[i].WCET = task_set[i].period * UT[i];
                if(type)
                    task_set[i].deadline = rand()%(int)(task_set[i].period - task_set[i].WCET) + task_set[i].WCET;
                else
                    task_set[i].deadline = rand()%(int)((task_set[i].period - task_set[i].WCET)/2) + (task_set[i].period + task_set[i].WCET)/2;
            }
            if(EDF_test(num_tasks, task_set))
                count_EDF++;
            if(RM_Test(num_tasks, task_set))
                count_RM++;
            if(DM_Test(num_tasks, task_set))
                count_DM++;
        }
        if(num_tasks == 10 && type == 1)
            fprintf(fp, "%f \t %d \t %d \t %d \n", util, count_EDF/50, count_RM/50, count_DM/50);
        else if(num_tasks == 25 && type == 1)
            fprintf(fp, "%f \t %d \t %d \t %d \n", util, count_EDF/50, count_RM/50, count_DM/50);
        else if(num_tasks == 10 && type == 0)
            fprintf(fp, "%f \t %d \t %d \t %d \n", util, count_EDF/50, count_RM/50, count_DM/50);
        else if(num_tasks == 25 && type == 0)
            fprintf(fp, "%f \t %d \t %d \t %d \n", util, count_EDF/50, count_RM/50, count_DM/50);
    }
}


int main()
{
    int i, j;
    char file[100]="values.txt";                               // file to read input values from
    FILE * fp = fopen(file, "r");
    char str[200];
    char *line;
    char *ptr;
    int num_tasks;
    fgets(str, 200, fp);                                       // read 200 characters
    print_it = 1;
    int Total_Tasks_Sets = atoi(str);

    printf("Total tasks sets from input file: %d \n",Total_Tasks_Sets);
    

    for(i = 0; i < Total_Tasks_Sets; i++)
	{
       printf("Testing for Task Set %d\n", (i+1));
	   fgets(str, 200, fp);
	   num_tasks = atoi(str);
       printf("Num Tasks :%d\n", num_tasks);
 	   struct task task_set[num_tasks];
	   for(j = 0; j < num_tasks; j++)
		{
 	          fgets(str, 200, fp);
                  line = strdup(str);
		  ptr = strsep(&line, " ");
		  task_set[j].WCET = atoi(ptr);
		  ptr = strsep(&line, " ");
		  task_set[j].deadline = atoi(ptr);
		  ptr = strsep(&line, " ");
		  task_set[j].period = atoi(ptr);
		}
        
        printf("Checking for EDF \n");

        EDF_test(num_tasks, task_set);

        printf("Checking for RM \n");
        
        RM_Test(num_tasks, task_set);
        
        printf("Calculating DM \n");
        
        DM_Test(num_tasks, task_set);
        
        }
    
    print_it = 0;
    
    Synthetic_Task_Set(10,1);                                 // Synthetic tasks set generation function calls for different plots
    Synthetic_Task_Set(25,1);
    Synthetic_Task_Set(10,0);
    Synthetic_Task_Set(25,0);
}

    




  
			
    	  
			
	   
