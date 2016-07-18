
/*=============================================================================
 * Abstract Factory Pattern - Factory interface class
 *===========================================================================*/
#ifndef I_CLASS_FACTORY_H_
#define I_CLASS_FACTORY_H_

#include <vector>

namespace aux
{

template<class ConcreteClassInterface, class ClassIDType> class IClassFactory
{
    public:
        virtual int GetAvailableClassIDs(std::vector<ClassIDType> &classIDs) = 0;
        virtual int GetClassInstance(ClassIDType classID, ConcreteClassInterface **instance) = 0;
};

}
#endif /*I_CLASS_FACTORY_H_*/
