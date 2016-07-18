
#include <iostream>
#include <string>
#include "aux/class-factory.hpp"

using namespace std;

/*===========================================================================*/
class IService
{
    public:
        virtual int GetNumber(void) = 0;
};
/*===========================================================================*/
class DataStorageService : public IService
{
    public:
        virtual int GetNumber(void) { return 111; }
};
/*===========================================================================*/
class MathService : public IService
{
    public:
        virtual int GetNumber(void) { return 222; }
};
/*===========================================================================*/
class HistoryService : public IService
{
    public:
        virtual int GetNumber(void) { return 333; }
};
/*===========================================================================*/

BEGIN_CLASS_FACTORY(ServiceFactory, IService, string)
 ADD_CLASS(string("DATA_STORAGE_SERVICE"), DataStorageService)
 ADD_CLASS(string("MATH_SERVICE"), MathService)
 ADD_CLASS(string("HISTORY_SERVICE"), HistoryService)
END_CLASS_FACTORY()

/*===========================================================================*/
using namespace std;

int main()
{
    cout << "Abstract Class Factory Test" << endl;

    cout << "Creating Abstract Class Factory..." << endl;
    IClassFactory<IService, string> *serviceFactory = new ServiceFactory();
    cout << "Abstract Class Factory Created" << endl;

    cout << "List of classes in the Abstract Class Factory:" << endl;
    vector<string> classIDs;
    serviceFactory->GetAvailableClassIDs(classIDs);
    cout << "Total Number: " << classIDs.size() << endl;
    for(int i = 0; i < classIDs.size(); i++)
    {
        cout << i+1 << ". " << classIDs[i] << endl;
    }

    cout << "Creating DataStorageService..." << endl;
    IService *serviceDataStorage = NULL;
    serviceFactory->GetClassInstance(string("DATA_STORAGE_SERVICE"), &serviceDataStorage);
    cout << "DataStorageService Created" << endl;

    cout << "Creating MathService..." << endl;
    IService *serviceMath = NULL;
    serviceFactory->GetClassInstance(string("MATH_SERVICE"), &serviceMath);
    cout << "MathService Created" << endl;

    cout << "Creating HistoryService..." << endl;
    IService *serviceHistory = NULL;
    serviceFactory->GetClassInstance(string("HISTORY_SERVICE"), &serviceHistory);
    cout << "HistoryService Created" << endl;

    cout << "DataStorageService result = " << serviceDataStorage->GetNumber() << endl;
    cout << "MathService result = " << serviceMath->GetNumber() << endl;
    cout << "HistoryService result = " << serviceHistory->GetNumber() << endl;
 return 0;
};
