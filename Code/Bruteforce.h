#include <iostream>
#include <vector>
#include <fstream>
using namespace std;
#ifndef Bruteforce_h
#define Bruteforce_h
#include "Lab.h"
struct Bruteforce
{
    //Primary Attributes
    int L;                                                                                      //Number of Labs
    int C;                                                                                      //Numer of cleaning visits
    int D;                                                                                      //Number of days
    //vector<int>        N;                                                                     //Number of students per lab
    vector<vector<int>>  P;                                                                     //Period each student needs in Lab
    
    //Runtime Attributes
    int H_max;                                                                                  //Maximum number of hours (24*D)
    
    //Results
    vector <int> C_schedule;                                                                    //Schedule of Cleaning Visits
    int timeUsed;                                                                               //Lab time used
    
    //Utility Methods
    int runUpdate (int H_curr, vector <Lab>& Lab_curr);                                         //Adds new people to each empty lab, returns the timeUsed
    bool runPromising (int H_curr, int timeUsed_curr);
    int nextCritical (vector <Lab>& Lab_curr, int H_curr);                                      //Returns next critical time
    void dumpRecords();
    void setupPruner();

    
    //Main  Methods
    void runSimulation(int H_curr, vector<int> c_curr, int timeUsed_curr, vector <Lab> Lab_curr);//Run the recursive simulation
    
    //Constructor and Destructor
    Bruteforce (int L, int C, int D, vector<vector<int>> P);
    ~Bruteforce();
};

//Utility Methods
int Bruteforce::nextCritical (vector <Lab>& Lab_curr, int H_curr)
{
    int min = INT_MAX;
    for (int i = 0; i<Lab_curr.size(); i++)
    {
        if (Lab_curr[i].EndTime<min && Lab_curr[i].EndTime>H_curr) min = Lab_curr[i].EndTime;
    }
    return (min==INT_MAX)? (H_curr):(min);
}

int Bruteforce::runUpdate (int H_curr, vector <Lab>& Lab_curr)
{
    int timeUsed = 0;
    
    for (int i = 0; i<Lab_curr.size(); i++) //go over all labs
    {
        if (Lab_curr[i].EndTime<=H_curr && Lab_curr[i].Queue < P[i].size()) //If its free, disinfect and get the next persion
        {
            //Place the next person inside the lab by modifying the end time to fit him
            Lab_curr[i].EndTime = H_curr + P[i][Lab_curr[i].Queue];
            
            //Add the time he's spending in the lab to the timeUsed accumlator
            if (Lab_curr[i].EndTime<H_max)
                    timeUsed += P[i][Lab_curr[i].Queue];

            else
                    timeUsed += H_max-H_curr;
            
            //Move the turn to the next person
            Lab_curr[i].Queue++;
        }
    }
    return timeUsed;
    
}

//Main  Methods
void Bruteforce::runSimulation(int H_curr, vector<int> C_curr, int timeUsed_curr, vector <Lab> Lab_curr)
{
    if (H_curr == H_max || C_curr.size() == C || !runPromising (H_curr, timeUsed_curr)) //We're done, return
    {
        if (timeUsed_curr>timeUsed)            //Check if we had a nice solution then keep it
        {
            C_schedule = C_curr;
            timeUsed = timeUsed_curr;
        }
        return;
    }
    
    //Else, If there happens to be another critical hour after that - 0
    if (nextCritical (Lab_curr, H_curr) != H_curr)
        runSimulation(nextCritical (Lab_curr, H_curr), C_curr, timeUsed_curr, Lab_curr);
    
    //Try to take it - 1
    if (H_curr != 0) C_curr.push_back(H_curr);
    timeUsed_curr += runUpdate (H_curr, Lab_curr);
    runSimulation(nextCritical (Lab_curr, H_curr), C_curr, timeUsed_curr, Lab_curr);
}

//Constructor and Destructor
Bruteforce::Bruteforce (int L, int C, int D, vector<vector<int>> P)
{
    //setupPruner();
    //Setting up the paramters
    this-> L = L;
    this-> C = C;
    this-> D = D;
    this-> P = P;
    H_max = 24 * D;
    
    timeUsed = 0;
    
    //Running the simulation
    vector<Lab> startingLabs (L);
    vector<int> startingC;
    runSimulation(0,startingC, 0, startingLabs);


}


Bruteforce::~Bruteforce()
{
    if (C<=0)
    {
        cout<<"Please enter a number of visits greater than 0. Program Terminated."<<endl;
        return;
    }

    //Printing the results
    cout<<"Bruteforce Disinfection Schedule: "<<endl;
    for (int i = 0; i<C_schedule.size(); i++)
        cout<<i+1<<") "<<C_schedule[i]<<endl;
    
    cout<<"Time Used: "<<timeUsed<<endl<<endl;
    dumpRecords();
}

bool Bruteforce::runPromising (int H_curr, int timeUsed_curr)
{
    if ((((H_max - H_curr)*L)+timeUsed_curr) < timeUsed)
        return false;
    else return true;
}


void Bruteforce::dumpRecords()
{
    fstream dataDump;
    dataDump.open( "dump.csv");
    dataDump.seekg (0, ios::end);
    dataDump << L <<","<< C <<","<< D <<","<< H_max <<","<< "Bruteforce" <<","<< timeUsed<<endl;
    dataDump.close();
}

void Bruteforce::setupPruner()
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
#endif /* Bruteforce_h */
