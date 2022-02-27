#include "Bruteforce.h"
#include "Approximation.h"

#include <iostream>
#include <vector>
using namespace std;

void RandomInstanceGenerator (int& L, int& C, int&D, vector<vector<int>>& P);
void InstancePrinter         (int L, int C, int D, vector<vector<int>> P);

int main()
{
    fstream dataDump;
    dataDump.open( "dump.csv");
    dataDump << "L," << "C," << "D," << "H_max," << ",Policy," << "timeUsed"<<endl;
    dataDump.close();
    
    for (int i = 1; i<150; i++)
    {
        
         int L = 10;
         int C = i;       //1 to 20
         int D = 10;       //1 to 365
         vector<vector<int>>  P;
        
        srand(time(NULL));
        RandomInstanceGenerator (L, C, D, P);
        InstancePrinter         (L, C, D, P);

        Approximation TestInstance (L, C, D, P);
        
        Bruteforce    ControlInstance (L, C, D, P);
    }

}



void RandomInstanceGenerator (int& L, int& C, int&D, vector<vector<int>>& P)
{
    //srand(time(NULL));
    //L = 1 + rand()%25;      //1 to 100
    //srand(time(NULL));
   // C = 1 + rand()%10;       //1 to 20
    //srand(time(NULL));
   // D = 1 + rand()%10;       //1 to 365
    
    P.resize (L);
    
    for (int i = 0; i<L; i++)
    {
        int sum = 0;
        while (sum <D*24 && P[i].size()<C+1)
        {
            int instance = 1+rand()%24*D;
            sum += instance;
            P[i].push_back(instance);
        }
    }
    
}

void InstancePrinter (int L, int C, int D, vector<vector<int>> P)
{
    
    cout<<"Number of Labs: "<<L<<endl;
    cout<<"Number of Cleaning Visits: "<<C<<endl;
    cout<<"Number of Days: "<<D<<endl<<endl;

    
    for (int i = 0; i<P.size(); i++)
    {
        cout<<"Lab "<<i+1<<" Queue:"<<endl;
        
        for (int j = 0; j<P[i].size(); j++)
            cout<<P[i][j]<<" ";
        
        cout<<endl;
    }
    cout<<endl;

}
