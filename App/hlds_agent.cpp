#include<iostream>
#include "hlds_agent.h"

using namespace std;
hlds_agent::hlds_agent()
{
}


hlds_agent::~hlds_agent()
{
}

void hlds_agent::SetThreadCount(int index)
{
    m_index = index;
}

int hlds_agent::Proc()
{
    return 0;
}

