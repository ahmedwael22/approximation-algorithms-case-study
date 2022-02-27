#ifndef Approximation_h
#define Approximation_h

#include <iostream>
#include <vector>
#include <cmath>
#include <numeric>
#include <fstream>
using namespace std;

#include "Lab.h"

struct Approximation
{
    //Primary Attributes
    int L;                                                                                      //Number of Labs
    int C;                                                                                      //Numer of cleaning visits
    int D;                                                                                      //Number of days
    vector<vector<int>>  P;                                                                     //Period each student needs in Lab (has N implictly)
    
    //Runtime Attributes
    int H_max;                                                                                  //Maximum number of hours (24*D)
    
    //Lab attributes
    float Lab_Variance;
    float Lab_Mean;
    float Lab_StdDev;

    
    //Results
    vector <int> C_schedule;                                                                    //Schedule of Cleaning Visits
    int          timeUsed;                                                                      //Lab time used
    
    //Best Results
    vector <int> best_C_schedule;
    int besttimeUsed;
    int bestPolicy;
    
    //Utility Functions
    int nextCritical (vector <Lab>& Lab_curr, int H_curr);                                      //Returns next critical time
    void dumpRecords();
    
    //Main  Methods
    void runSimulation(int H_curr, vector<int> c_curr, int timeUsed_curr, vector <Lab> Lab_curr);//Run the simulation
    
    //Constructor and Destructor
    Approximation (int L, int C, int D, vector<vector<int>> P);
    Approximation (int L, int C, int D, vector<vector<int>> P, int Policy);
    ~Approximation();
    
    //Policies
    int Policy;
    string PolicyName;
    string PolicyDictionary (int Policy);

    //Main Policy Function
    bool policyMet (int H_curr, vector<Lab> Lab_curr, vector <int> C_curr);
    
    //Policies based on Occupancy
    bool policyMetHalf(int H_curr, vector<Lab> Lab_curr);
    bool policyMetHalfDynamic (int H_curr, vector<int> C_curr, vector<Lab> Lab_curr);
    bool policyMetHalfDynamic2 (int H_curr, vector<int> C_curr, vector<Lab> Lab_curr);
    bool policyMetMax(int H_curr, vector<Lab> Lab_curr);

    //Policies based on Constant
    bool policyMetConstant(int H_curr, vector<int> C_curr, vector<Lab> Lab_curr);
    
    //Based on Lost/Gain Analysis
    bool policyMetQueue(int H_curr, vector<Lab> Lab_curr);
    bool policyMetPL(int H_curr, vector<Lab> Lab_curr);
    
    //Control Policies (for comparsion)
    bool policyMetRand(int H_curr, vector<Lab> Lab_curr);
    bool policyMetGreedy();
    
    //Statistical Functions
    void setupPruner();
    void calcStats (vector <Lab>& Lab_curr);
    void setupSlack(vector <Lab>& Lab_curr);
    void updateWeight(vector <Lab>& Lab_curr);
    void updateSlack (int H_curr, vector <Lab>& Lab_curr);
    int  updateLabs (int H_curr, vector <Lab>& Lab_curr, vector<int> C_curr);                                         //Adds new people to each empty lab, returns the timeUsed

};

//Utility Methods
int Approximation::nextCritical (vector <Lab>& Lab_curr, int H_curr)
{
    int min = INT_MAX;
    
    for (int i = 0; i<Lab_curr.size(); i++)
        if (Lab_curr[i].EndTime<min && Lab_curr[i].EndTime>H_curr)
            min = Lab_curr[i].EndTime;

    return (min==INT_MAX)? (H_curr):(min);
}

int Approximation::updateLabs (int H_curr, vector <Lab>& Lab_curr, vector<int> C_curr)
{
    int timeUsed = 0;
    
    for (int i = 0; i<Lab_curr.size(); i++) //go over all labs
    {
        if (Lab_curr[i].EndTime<=H_curr && Lab_curr[i].Queue < P[i].size()) //If its free, disinfect and get the next persion
        {
            //Place the next person inside the lab by modifying the end time to fit him
            Lab_curr[i].EndTime = H_curr + P[i][Lab_curr[i].Queue];
                        
            //Add the time he's spending in the lab to the timeUsed accumlator
            if (Lab_curr[i].EndTime<H_max) timeUsed += P[i][Lab_curr[i].Queue];
            else timeUsed += H_max-H_curr;
            
            //Move the turn to the next person
            Lab_curr[i].Queue++;
        }
        
        else if ((P[i].size()-Lab_curr[i].Queue)>(C_curr.size()-C))//At start, the numbber of people is equal to C
        {
            
            Lab_curr[i].Slack += P[i].back();
            P[i].pop_back();
        }
    }
    return timeUsed;
    
}

void Approximation::updateSlack (int H_curr, vector <Lab>& Lab_curr)
{
    for (int i = 0; i<Lab_curr.size(); i++)
    {
        if (H_curr>Lab_curr[i].EndTime)             //If the lab  was free during the time jump, remove slack
        {
            Lab_curr[i].Slack -= H_curr - ((Lab_curr[i].LastUpdate>Lab_curr[i].EndTime)? (Lab_curr[i].LastUpdate):(Lab_curr[i].EndTime));
            Lab_curr[i].LastUpdate = H_curr;
        }
    }
}
//Main  Methods
void Approximation::runSimulation(int H_curr, vector<int> C_curr, int timeUsed_curr, vector <Lab> Lab_curr)
{
    
    setupPruner();
    calcStats (Lab_curr);
    setupSlack(Lab_curr);
    
    while (H_curr < H_max && C_curr.size() < C) //We're done, return
    {
        updateSlack (H_curr, Lab_curr);
        updateWeight (Lab_curr);

        
        if (policyMet (H_curr, Lab_curr, C_curr))
        {
            timeUsed_curr += updateLabs (H_curr, Lab_curr, C_curr);
            if (H_curr != 0) C_curr.push_back(H_curr);
        }
        
        //Move to the next critical point
        H_curr = nextCritical (Lab_curr, H_curr);
    }

    //Update the results
    C_schedule = C_curr;
    timeUsed = timeUsed_curr;
}

//Constructor and Destructor
Approximation::Approximation (int L, int C, int D, vector<vector<int>> P)
{
    //Setting up the paramters
    this-> L = L;
    this-> C = C;
    this-> D = D;
    this-> P = P;
    H_max = 24 * D;
    
    timeUsed = 0;
    C_schedule.clear();
    
    besttimeUsed = 0;
    best_C_schedule.clear();
    
    
    //Create placeholder parameters
    vector<Lab> startingLabs (L);
    vector<int> startingC;
    
    //Run the simulation using all policies tell you find the best
    for (int i = 1; i<=9; i++)
    {
        Policy = i;
        runSimulation(0,startingC, 0, startingLabs);
        if(timeUsed>=besttimeUsed)
        {
            besttimeUsed = timeUsed;
            best_C_schedule = C_schedule;
            bestPolicy = Policy;
        }

    }
    
    Policy = 10;
}

Approximation::Approximation (int L, int C, int D, vector<vector<int>> P, int Policy)
{
    //Policy Setting
    this->Policy = Policy;
    
    //Setting up the paramters
    this-> L = L;
    this-> C = C;
    this-> D = D;
    this-> P = P;
    H_max = 24 * D;
    
    timeUsed = 0;
    C_schedule.clear();
        
    //Running the simulation
    vector<Lab> startingLabs (L);
    vector<int> startingC;
    runSimulation(0,startingC, 0, startingLabs);
}

Approximation::~Approximation()
{
    if (C<=0)
    {
        cout<<"Please enter a number of visits greater than 0. Program Terminated."<<endl;
        return;
    }

    //Printing the results
    cout<<"Approx. Disinfection Schedule for "<< ((Policy==10)? ("Best Policy"):(to_string(Policy))) <<": "<<PolicyDictionary((Policy==10)? bestPolicy: Policy)<<endl;
    for (int i = 0; i<((Policy==10)? (best_C_schedule.size()):(C_schedule.size())); i++)
        cout<<i+1<<") "<<((Policy==10)? (best_C_schedule[i]):(C_schedule[i]))<<endl;
    
    cout<<"Time Used: "<<((Policy==10)? (besttimeUsed):(timeUsed))<<endl<<endl;
    
    dumpRecords();
}

//Policy Umbrella
bool Approximation::policyMet (int H_curr, vector<Lab> Lab_curr, vector <int> C_curr)
{
    switch (Policy)
    {
        case 1:
        {
            return policyMetHalf(H_curr, Lab_curr);
            break;
        }
        case 2:
        {
            return policyMetMax(H_curr, Lab_curr);
            break;
        }
        case 3:
        {
            return policyMetConstant(H_curr,C_curr, Lab_curr);
            break;
        }
        case 4:
        {
            return policyMetQueue(H_curr, Lab_curr);
            break;
        }
        case 5:
        {
            return policyMetPL(H_curr, Lab_curr);
            break;
        }
        case 6:
        {
            return policyMetRand(H_curr, Lab_curr);
            break;
        }
        case 7:
        {
            return policyMetGreedy();
            break;
        }
        case 8:
        {
            return policyMetHalfDynamic (H_curr, C_curr, Lab_curr);
            break;
        }
        case 9:
        {
            return policyMetHalfDynamic2 (H_curr, C_curr, Lab_curr);
            break;
        }
            
        default:
        {
            return policyMetPL(H_curr, Lab_curr);
        }
    }
}

string Approximation::PolicyDictionary (int Policy)
{
    switch (Policy)
    {
        case 1:
        {
            return "Wait till half the Labs are free";
            break;
        }
        case 2:
        {
            return "Wait till all Labs are free";
            break;
        }
        case 3:
        {
            return "Constant Seperation";
            break;
        }
        case 4:
        {
            return "Queue Analysis";
            break;
        }
        case 5:
        {
            return "Profit/Loss Analysis";
            break;
        }
        case 6:
        {
            return "Random Decisions";
            break;
        }
        case 7:
        {
            return "Greedy Decisions";
            break;
        }
        case 8:
        {
            return "Dynamic Policy - Half Priority";
            break;
        }
        case 9:
        {
            return "Dynamic Policy - Half Priority v2.0";
            break;
        }
            
        default:
        {
            return "";
        }
    }
}
//Policy Functions
bool Approximation::policyMetHalf(int H_curr, vector<Lab> Lab_curr)
{
    int counter = 0;
    
    for (auto i : Lab_curr)
        counter+= (i.EndTime<=H_curr);
    
    return (counter >= ((Lab_curr.size()+1)/2));
}

bool Approximation::policyMetMax(int H_curr, vector<Lab> Lab_curr)
{
    return (nextCritical(Lab_curr, H_curr) == H_curr);
}

bool Approximation::policyMetConstant (int H_curr, vector <int> C_curr, vector <Lab> Lab_curr)
{
    return true;
    return ((H_curr <= (H_max/C)) || (nextCritical(Lab_curr, H_curr) == H_curr) || (H_curr - C_curr.back())>= (H_max/(C+1))) ;
}

bool Approximation::policyMetQueue(int H_curr, vector<Lab> Lab_curr)
{
    int queueTaken = 0;
    int queueLost = 0;
    
    for (int i = 0; i<Lab_curr.size(); i++)
    {
        if (H_curr>=Lab_curr[i].EndTime) queueTaken += P[i][Lab_curr[i].Queue];          //This is ready to be disinfected
        else if (Lab_curr[i].Queue<P[i].size()-1) queueLost += P[i][Lab_curr[i].Queue]; //This is NOT ready to be disinfected
    }
    return (queueTaken>queueLost);
}

bool Approximation::policyMetPL(int H_curr, vector<Lab> Lab_curr)
{
    if (nextCritical(Lab_curr, H_curr) == H_curr) return true;

    int queueTaken = 0;
    int queueLost = 0;
    
    for (int i = 0; i<Lab_curr.size(); i++)
    {
        if (H_curr>=Lab_curr[i].EndTime) queueTaken ++;
        else if (Lab_curr[i].Queue<P[i].size()-1) queueLost  += P[i][Lab_curr[i].Queue]; //This is NOT ready to be disinfected
    }
    
    queueTaken *= nextCritical(Lab_curr, H_curr) - H_curr;
    
    return (queueTaken>queueLost);
}

bool Approximation::policyMetGreedy()
{
    return true;
}

bool Approximation::policyMetRand(int H_curr, vector<Lab> Lab_curr)
{
    srand(time(NULL));
    if (nextCritical(Lab_curr, H_curr) == H_curr) return true;
    else return rand()%2;
}

bool Approximation::policyMetHalfDynamic (int H_curr, vector<int> C_curr, vector<Lab> Lab_curr)
{
    //Checking the priorities
    float priorityTaken = 0;
    float priorityLeft = 0;
    
    //Checking the free labs
    int labTaken = 0;
    int labLeft = 0;

    //Checking the left tasks
    int maxQueueLeft = 0;
    
    for (int i = 0; i<L; i++)
    {
        if (Lab_curr[i].EndTime<=H_curr)
        {
            priorityTaken+= (Lab_curr[i].Priority);
            labTaken++;
        }

        else
        {
            priorityLeft+= (Lab_curr[i].Priority);
            labLeft++;
        }
        if (maxQueueLeft < (P[i].size() - Lab_curr[i].Queue+1)) maxQueueLeft = (P[i].size() -  Lab_curr[i].Queue+1);
    }
        
    //return (L*(priorityTaken)>=((priorityLeft * C_curr.size()/C)));
    return ( (labTaken>=labLeft) || (priorityTaken>=priorityLeft) || (maxQueueLeft < C-C_curr.size())) ;

}

bool Approximation::policyMetHalfDynamic2 (int H_curr, vector<int> C_curr, vector<Lab> Lab_curr)
{
    //Update the statistics
    calcStats (Lab_curr);

    //Checking the priorities
    float priorityTaken = 0;
    float priorityLeft = 0;
    
    //Checking the free labs
    int labTaken = 0;
    int labLeft = 0;

    //Checking the left tasks
    int maxQueueLeft = 0;
    
    for (int i = 0; i<L; i++)
    {
        if (Lab_curr[i].EndTime<=H_curr)
        {
            priorityTaken+= (Lab_curr[i].Priority);
            labTaken++;
        }

        else
        {
            priorityLeft+= (Lab_curr[i].Priority);
            labLeft++;
        }
        if (maxQueueLeft < (P[i].size() - Lab_curr[i].Queue+1)) maxQueueLeft = (P[i].size() -  Lab_curr[i].Queue+1);
    }
        
    //return (L*(priorityTaken)>=((priorityLeft * C_curr.size()/C)));
    return ( (labTaken>=labLeft) || (priorityTaken>=priorityLeft) || (maxQueueLeft < C-C_curr.size())) ;

}



void Approximation::setupPruner()
{
    //Prune periods if scheduled visits are greater than C+1
    for (int i = 0; i<P.size(); i++)
        while (P[i].size()>C+1)
            P[i].pop_back();

    
    //Prune periods if their sum exceeds 24*D
    for (int i = 0; i<P.size(); i++)
    {
        int Sum = 0;
        for (int j = 0; j<P[i].size(); j++)
        {
            Sum+= P[i][j];
            if (Sum>H_max)
            {
                P[i].resize(j+1);
                P[i][j] -= (Sum - H_max);
            }
        }
    }
}
void Approximation::calcStats (vector <Lab>& Lab_curr)
{
    float tempSum = 0;
    
    //Calcualating the mean for each lab
        for (int i = 0; i<L; i++)
        {
            Lab_curr[i].Mean = std::accumulate(P[i].begin()+Lab_curr[i].Queue, P[i].end(), 0.0) / float (P[i].size());
            tempSum += Lab_curr[i].Mean;
        }
    
    //Calculate the Mean of Means
        Lab_Mean = tempSum/ float (L);
    
    //Calculate the Variance
        tempSum = 0;
        for (int i = 0; i<L; i++)
            tempSum += pow ((Lab_curr[i].Mean - Lab_Mean),2);
        Lab_Variance = tempSum / float(L);
    
    //Calculate the STD_DEV
        Lab_StdDev = sqrt (Lab_Variance);
    
    //Calculating the Mean Deviance
        for (int i = 0; i<L; i++)
            //Lab_curr[i].MeanDeviance = Lab_Mean - Lab_curr[i].Mean;
            Lab_curr[i].MeanDeviance = float((Lab_curr[i].Mean-Lab_Mean))/float(Lab_StdDev);



}

void Approximation::setupSlack(vector <Lab>& Lab_curr)
{
    for (int i = 0; i<L; i++)
        Lab_curr[i].Slack = 24*D - std::accumulate(P[i].begin(), P[i].end(), 0.0);
}

void Approximation::updateWeight(vector <Lab>& Lab_curr)
{
    //Greater Weight -> More priority
    //Weight of finished > Weight of unfinished -> Disinfection
    //To be more prioritized
        // Least abs(MeanDeviance); or least abs (Z-score)
        // Least slack
    
    float totalWeight = 0;
    for (int i = 0; i<Lab_curr.size(); i++)
    {
        Lab_curr[i].Weight = ((!Lab_curr[i].MeanDeviance)?(1):(1/abs(Lab_curr[i].MeanDeviance))) * ((!Lab_curr[i].Slack)?(1):(1/Lab_curr[i].Slack));
        totalWeight += Lab_curr[i].Weight;
    }
    
    for (int i = 0; i<Lab_curr.size(); i++)
    {
        Lab_curr[i].Priority = Lab_curr[i].Weight/totalWeight;
    }

    
}

void Approximation::dumpRecords()
{
    fstream dataDump;
    dataDump.open( "dump.csv");
    dataDump.seekg (0, ios::end);
    dataDump << L <<","<< C <<","<< D <<","<< H_max <<","<< Policy <<","<< ((Policy==10)? (besttimeUsed):(timeUsed)) <<endl;
    dataDump.close();
}
#endif 
