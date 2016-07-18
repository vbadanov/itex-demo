
/*=============================================================================
 * Abstract Factory Pattern - Factory Implementation class
 *=============================================================================
 * USAGE:
 *
 * #include "class_factory.h"
 * BEGIN_CLASS_FACTORY(ConcreteClassFactory, ConcreteClassInterface, ClassIDType)
 *  ADD_CLASS(ID1, ConcreteClass1)
 *  ADD_CLASS(ID2, ConcreteClass2)
 *  ADD_CLASS(ID3, ConcreteClass3)
 * END_CLASS_FACTORY()
 *
 *=============================================================================
 * EXAMPLE:
 *
 * #include "class_factory.h"
 * BEGIN_CLASS_FACTORY(ServiceFactory, IService, string)
 *  ADD_CLASS(string("DATA_STORAGE_SERVICE"), DataStorageService)
 *  ADD_CLASS(string("MATH_SERVICE"), MathService)
 *  ADD_CLASS(string("HISTORY_SERVICE"), HistoryService)
 * END_CLASS_FACTORY()
 *
 * IClassFactory<IService, string> *serviceFactory = new ServiceFactory();
 *
 * IService *serviceDataStorage = NULL;
 * serviceFactory->GetClassInstance(string("DATA_STORAGE_SERVICE"), &serviceDataStorage);
 *
 * IService *serviceMath = NULL;
 * serviceFactory->GetClassInstance(string("MATH_SERVICE"), &serviceMath);
 *
 * IService *serviceDataStorage = NULL;
 * serviceFactory->GetClassInstance(string("HISTORY_SERVICE"), &serviceHistory);
 *
 *===========================================================================*/
#ifndef CLASS_FACTORY_H_
#define CLASS_FACTORY_H_

#include <aux/i-class-factory.hpp>

namespace aux
{

/*==========================================================================================================*/
#define BEGIN_CLASS_FACTORY(ConcreteClassFactory, ConcreteClassInterface, ClassIDType)                       \
                                                                                                             \
class ConcreteClassFactory : public IClassFactory<ConcreteClassInterface, ClassIDType>                       \
{                                                                                                            \
    private:                                                                                                 \
        std::vector<ClassIDType> _classIDs;                                                                       \
                                                                                                             \
        enum ClassDataType                                                                                   \
        {                                                                                                    \
            CLASS_ID_LIST = 0,                                                                               \
            CLASS_INSTANCE = 1                                                                               \
        };                                                                                                   \
                                                                                                             \
    public:                                                                                                  \
        virtual int GetAvailableClassIDs(std::vector<ClassIDType> &classIDs)                                      \
        {                                                                                                    \
            int retcode = GetData(CLASS_ID_LIST, NULL, NULL);                                                \
            classIDs = _classIDs;                                                                            \
            return retcode;                                                                                  \
        };                                                                                                   \
                                                                                                             \
        virtual int GetClassInstance(ClassIDType classID, ConcreteClassInterface **instance)                 \
        {                                                                                                    \
            *instance = NULL;                                                                                \
            return GetData(CLASS_INSTANCE, &classID, instance);                                              \
        };                                                                                                   \
                                                                                                             \
        int GetData(ClassDataType data_type, ClassIDType *classID, ConcreteClassInterface **instance)        \
        {                                                                                                    \
            int retcode = 0;                                                                                 \

/*==========================================================================================================*/
#define ADD_CLASS(ClassID, ConcreteClass)                                                                    \
            switch(data_type)                                                                                \
            {                                                                                                \
                default:                                                                                     \
                case CLASS_ID_LIST:                                                                          \
                {                                                                                            \
                    _classIDs.push_back(ClassID);                                                            \
                    retcode = 0;                                                                             \
                    break;                                                                                   \
                };                                                                                           \
                case CLASS_INSTANCE:                                                                         \
                {                                                                                            \
                    if(*classID == ClassID)                                                                  \
                    {                                                                                        \
                        *instance = new ConcreteClass();                                                     \
                        retcode = 0;                                                                         \
                    }                                                                                        \
                    break;                                                                                   \
                };                                                                                           \
            };                                                                                               \

/*==========================================================================================================*/
#define END_CLASS_FACTORY()                                                                                  \
            return retcode;                                                                                  \
        };                                                                                                   \
};                                                                                                           \

}

#endif /*CLASS_FACTORY_H_*/

