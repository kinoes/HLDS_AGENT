#include "agent_status_db.h"
/////////////////////////////////////////////////////////////////////////
// Name : Initialization
// Function : ZetavuDB init
/////////////////////////////////////////////////////////////////////////
bool agent_status_db::Initialization(const char *fname)
{
    try{
    }
    catch (CppSQLite3Exception& e)
    {
        return false;
    }
    return true;
}
/***********************************************************************/
