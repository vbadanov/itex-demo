
#ifndef PLATFORM_AUX_INCLUDE_AUX_ARRAY3D_HPP_
#define PLATFORM_AUX_INCLUDE_AUX_ARRAY3D_HPP_

#include <stdexcept>
#include <algorithm>
#include <type_traits>
#include <aux/utils.hpp>

namespace aux
{

//===============================================================================
struct Vector3
{
	int64_t x;
	int64_t y;
	int64_t z;
};

template<class T>
class Array3d
{
	public:
		//===============================================================================
		Array3d(Vector3 size, T default_value, T* data = nullptr)
			: size_(size), data_(nullptr)
		{
			size_t array_size = size_.x * size_.y * size_.z;
			if(array_size != 0)
			{
				if(std::is_arithmetic<T>::value)
				{
					data_ = aux::allocate_buffer<T>(array_size);
				}
				else
				{
					data_ = new T[array_size];
				}
			}

			if(data_ == nullptr)
			{
				return;
			}

			if(data != nullptr)
			{
				std::copy_n(data, array_size, data_);
			}
			else
			{
				std::fill_n(data_, array_size, default_value);
			}
		}

		//===============================================================================
		Array3d(const Array3d& array)
			: size_(array.size_), data_(nullptr)
		{
			size_t array_size = size_.x * size_.y * size_.z;
			if(array_size != 0)
			{
				if(std::is_arithmetic<T>::value)
				{
					data_ = aux::allocate_buffer<T>(array_size);
				}
				else
				{
					data_ = new T[array_size];
				}
			}

			if(data_ != nullptr)
			{
				std::copy_n(array.data_, array_size, data_);
			}
		}

		//===============================================================================
		Array3d(Array3d&& array)
			: size_(array.size_), data_(array.data_)
		{
			array.size_ = {0, 0, 0};
			array.data_ = nullptr;
		}

		//===============================================================================
		~Array3d()
		{
			if(data_ != nullptr && size_.x * size_.y * size_.z > 0)
			{
				if(std::is_arithmetic<T>::value)
				{
					free(data_);
				}
				else
				{
					delete[] data_;
				}
				data_ = nullptr;
				size_ = {0, 0, 0};
			}
		}

		//===============================================================================
		Array3d& operator=(const Array3d& array)
		{
			Array3d tmp(array);
			*this = std::move(tmp);
			return *this;
		}

		//===============================================================================
		Array3d& operator=(Array3d&& array)
		{
			if(data_ != nullptr && size_.x * size_.y * size_.z > 0)
			{
				if(std::is_fundamental<T>::value)
				{
					free(data_);
				}
				else
				{
					delete[] data_;
				}
			}
			size_ = array.size_;
			array.size_ = {0, 0, 0};

			data_ = array.data_;
			array.data_ = nullptr;

			return *this;
		}

		//===============================================================================
		static inline Vector3 fix_index(const Vector3& index, const Vector3& size)
		{
			register int64_t idx_i = (index.x >= 0 ? index.x : size.x + index.x);
			idx_i = (idx_i >= size.x ? idx_i - size.x : idx_i);

			register int64_t idx_j = (index.y >= 0 ? index.y : size.y + index.y);
			idx_j = (idx_j >= size.y ? idx_j - size.y : idx_j);

			register int64_t idx_k = (index.z >= 0 ? index.z : size.z + index.z);
			idx_k = (idx_k >= size.z ? idx_k - size.z : idx_k);

			if(idx_i < 0 || idx_i >= size.x || idx_j < 0 || idx_j >= size.y || idx_k < 0 || idx_k >= size.z)
			{
				std::cout << "Array3d - invalid index: {" << index.x << ", " << index.y << ", " << index.z << "}/{" << idx_i << ", " << idx_j << ", " << idx_k << "} min: {0, 0, 0} max: {" << size.x-1 << ", " << size.y-1 << ", " << size.z-1 << "}" << std::endl;
				throw std::runtime_error("Array3d - invalid index");
			}

			return Vector3 {idx_i, idx_j, idx_k};
		}
		//===============================================================================
		inline T& operator()(int64_t i, int64_t j, int64_t k)
		{
			Vector3 fixed_idx = fix_index({i, j, k}, size_);
			return data_[fixed_idx.x + size_.x * (fixed_idx.y + size_.y * fixed_idx.z)];
		}

		//===============================================================================
		inline Vector3& size()
		{
			return size_;
		}

		//===============================================================================
		inline T* data()
		{
			return data_;
		}

		//===============================================================================
		inline size_t data_size()
		{
			return size_.x * size_.y * size_.z;
		}

	private:
		Vector3 size_;
		T* data_;

};

}

#endif /* PLATFORM_AUX_INCLUDE_AUX_ARRAY3D_HPP_ */
