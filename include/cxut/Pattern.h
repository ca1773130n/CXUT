#pragma once
#include <string>

#define SAFE_DELETE(x) { if (x) delete[] x; }

namespace cxut {
    class IDumpable {
        public:
            virtual void dump(std::string& buffer, size_t depth = 0) = 0;
    };

	class Object : public IDumpable {
		public:
            Object(std::string name) {
                mName = name;
            }
			virtual ~Object() {}

			std::string& name(void) {
				return mName;
			}

			virtual void dump(std::string& buffer, size_t depth = 0) {
                for (int i = 0; i < depth; ++i)
                    buffer.append(" ");
                buffer.append("[");
                buffer.append(mName);
                buffer.append("]\n");
            }

		protected:
			std::string mName;
	};

	template <typename T>
		class ISingleton {
			public:
				static T *getInstance(void) {
					if (!mInstance)
						mInstance = new T();
					return mInstance;
				}

				static void destroyInstance(void) {
					SAFE_DELETE(mInstance);
				}

			protected:
				ISingleton() {}
				virtual ~ISingleton() {}

			private:
				static T *mInstance;
		};

	template <typename T>
		T * ISingleton<T>::mInstance = nullptr;
}
