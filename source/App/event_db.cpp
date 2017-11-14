#include "event_db.h"


/////////////////////////////////////////////////////////////////////////
// Name : Initialization
// Function : ZetavuDB init
/////////////////////////////////////////////////////////////////////////
bool event_db::Initialization(const char *fname)
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
