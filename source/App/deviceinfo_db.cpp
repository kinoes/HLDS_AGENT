#include "deviceinfo_db.h"

/////////////////////////////////////////////////////////////////////////
// Name : Initialization
// Function : ZetavuDB init
/////////////////////////////////////////////////////////////////////////
bool deviceinfo_db::Initialization(const char *fname)
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
