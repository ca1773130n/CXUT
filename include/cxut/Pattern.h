#pragma once
#include <queue>
#include <string>

#define DELETE(x) { if (x) delete x; x = nullptr; }

namespace cxut {
	class IDumpable {
	public:
		virtual void dump(std::string& buffer, size_t depth = 0) = 0;
	};

	class Object : public IDumpable {
	public:
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
		Object() : mName("") {}
		Object(std::string name) {
			mName = name;
		}

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
			DELETE(mInstance);
		}

	protected:
		ISingleton() {}
		virtual ~ISingleton() {}

	private:
		static T *mInstance;
	};

	class ObservableData : public Object {
	public:
		ObservableData(int sensorType, int sensorID) :
			mSensorType(sensorType), mSensorID(sensorID) {}

		int mSensorType;
        int mSensorID;
	};

	class IObserver {
	public:
		virtual bool push(ObservableData *data) = 0;
	};

	template <typename T>
	class CircularQueue : public Object {
	public:
		explicit CircularQueue(size_t maxQueueSize) : mHead(0), mQueueSize(0) {
			setMaxSize(maxQueueSize);
		}

		~CircularQueue() {
			for (auto& e : mQueue)
				DELETE(e);
		}

		size_t getSize(void) {
			return mQueueSize;
		}

		size_t getMaxSize(void) {
			return mMaxQueueSize;
		}

		void setMaxSize(size_t maxQueueSize) {
			mMaxQueueSize = maxQueueSize;
			mQueue.resize(mMaxQueueSize);
		}

		void push(const T element) {
            mQueue[mHead] = element;
            advanceHead();
            mQueueSize++;
		}

		const T getLast(void) const {
			if (mHead == 0)
				return mQueue[mMaxQueueSize - 1];
			return mQueue[mHead - 1];
		}

		const T getNext(void) {
			advanceHead();
			return getLast();
		}

		virtual void dump(std::string& buffer) {}

	protected:
		void advanceHead(void) {
			mHead++;
			mHead %= mMaxQueueSize;
		}

		std::vector<T> mQueue;
		size_t mQueueSize;
		size_t mMaxQueueSize;
		size_t mHead;
	};

	template <typename T>
	class ObjectPool : public Object {
	public:
		ObjectPool(size_t initSize, size_t maxSize) : mPoolSize(0), mSizeLimit(maxSize) {
			populate(initSize);
		}

		void putInstance(T *instance) {
			mQueue.push(instance);
		}

		T *getInstance(void) {
			if (mQueue.empty()) {
				if (mPoolSize <= mSizeLimit - mPoolSize) populate(mPoolSize);
				else return nullptr;
			}

			T *instance = mQueue.front();
			mQueue.pop();
			return instance;
		}

		virtual void dump(std::string& buffer) {}

	protected:
		void populate(size_t num) {
			for (size_t i = 0; i < num; ++i)
				putInstance(new T);
			mPoolSize += num;
		}

		size_t mPoolSize;
		size_t mSizeLimit;
		std::queue<T *> mQueue;
	};

	template <typename T>
	T * ISingleton<T>::mInstance = nullptr;
}
