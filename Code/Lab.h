#ifndef Lab_h
#define Lab_h

struct Lab
{
    //Lab Operation Attributtes
    int Queue;
    int EndTime;
    
    //Static Statistics
    float Mean;
    float MeanDeviance;
    
    //Dynamic Statistics
    int Slack;
    int LastUpdate;
    
    //Priority
    float Weight;
    float Priority;
    
    Lab ()
    {
        Queue = 0;
        EndTime = 0;
        LastUpdate = 0;
    }
};

#endif /* Lab_h */
