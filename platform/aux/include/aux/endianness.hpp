#ifndef AUX_ENDIANNESS_H_
#define AUX_ENDIANNESS_H_

#include <cstdint>

namespace aux
{

class Endianness
{
    public:
        enum class EndiannessType
        {
            LITTLE,
            BIG
        };

        Endianness()
        {
            int i = 1;
            char *p = (char *)&i;
            system_endianness_ = ((p[0] == 1) ? EndiannessType::LITTLE : EndiannessType::BIG);
        };

        inline uint16_t swap16(uint16_t val) { return (((val >> 8) & 0x00FF) | ((val << 8) & 0xFF00)); };
        inline uint32_t swap32(uint32_t val) { return ((swap16(val & 0xFFFF) << 16) | (swap16(val >> 16))); };
        inline uint64_t swap64(uint64_t val) { return ((static_cast<uint64_t>(swap32(val & 0xFFFFFFFFULL)) << 32) | (swap32(val >> 32))); };

        inline uint16_t conv16(uint16_t val) { return ((system_endianness_ == EndiannessType::LITTLE) ? swap16(val) : val); };
        inline uint32_t conv32(uint32_t val) { return ((system_endianness_ == EndiannessType::LITTLE) ? swap32(val) : val); };
        inline uint64_t conv64(uint64_t val) { return ((system_endianness_ == EndiannessType::LITTLE) ? swap64(val) : val); };

        inline uint16_t hton16(uint16_t val) { return conv16(val); };
        inline uint16_t ntoh16(uint16_t val) { return conv16(val); };

        inline uint32_t hton32(uint32_t val) { return conv32(val); };
        inline uint32_t ntoh32(uint32_t val) { return conv32(val); };

        inline uint64_t hton64(uint64_t val) { return conv64(val); };
        inline uint64_t ntoh64(uint64_t val) { return conv64(val); };

        inline uint64_t hton_double(double val)
        {
            DoubleToUint64 u;
            u.d = val;
            return conv64(u.ui64);
        };
        inline double ntoh_double(uint64_t val)
        {
            uint64_t a = conv64(val);
            DoubleToUint64 u;
            u.ui64 = a;
            return u.d;
        };

    protected:
        union DoubleToUint64
        {
            double d;
            uint64_t ui64;
        };

    private:
        EndiannessType system_endianness_;

};

}
#endif //AUX_ENDIANNESS_HPP_


