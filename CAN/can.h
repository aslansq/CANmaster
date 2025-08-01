#ifndef CAN_H
#define CAN_H

#include <QThread>
#include <QObject>
#include <QMutex>
#include <QQueue>
#include <QMutexLocker>
#include <cstdint>

template <typename T>
class ThreadSafeQueue {
public:
	void enqueue(const T& value) {
		QMutexLocker locker(&mutex);
		queue.enqueue(value);
	}

	T dequeue() {
		QMutexLocker locker(&mutex);
		if (queue.isEmpty()) {
			throw std::runtime_error("Queue is empty");
		}
		return queue.dequeue();
	}

	bool waitAndDequeue(T &value, int timeoutMs) {
		mutex.lock();
		QElapsedTimer timer;
		timer.start();
		while(queue.isEmpty()) {
			if (timer.elapsed() > timeoutMs) {
				mutex.unlock();
				return false; // Timeout
			}
			mutex.unlock();
			QThread::msleep(1); // Sleep to avoid busy waiting
			mutex.lock();
		}
		value = queue.dequeue();
		mutex.unlock();
		return true;
	}

	bool isEmpty() const {
		QMutexLocker locker(&mutex);
		return queue.isEmpty();
	}

	void clear() {
		QMutexLocker locker(&mutex);
		queue.clear();
	}

private:
	QQueue<T> queue;
	mutable QMutex mutex;
};


typedef struct
{
	uint32_t id;
	bool isExtd;
	uint8_t dataLength;
	uint8_t data[64];
	uint64_t timestamp;
} CanMsg;

enum class CanEvent
{
	Connected,
	Disconnected,
	MessageReceived
};


Q_DECLARE_METATYPE(CanEvent)

struct CanStdConfig {
public:
	QString interface;
	int channel;
	int baud;
};

struct CanFdPhaseConfig {
public:
	int baud;
	double samplePoint;
	int tseg1;
	int tseg2;
	int sjw;
};

struct CanFdConfig {
	QString interface;
	int channel;
	CanFdPhaseConfig arbit;
	CanFdPhaseConfig data;
};

class Can : public QObject
{
	Q_OBJECT
public:
	Can(QObject *parent);
	virtual ~Can() {}

	virtual void connect(std::variant<CanStdConfig, CanFdConfig> config) = 0;
	virtual void disconnect(void) = 0;
	//! @brief read from device and put it into rxQueue
	virtual void rx(void) = 0;
	//! @brief check txQueue, if not empty send it to can
	virtual void tx(void) = 0;

	bool isConnected(void) const;

	ThreadSafeQueue<CanMsg> rxQueue;
	ThreadSafeQueue<CanMsg> txQueue;
signals:
	void eventOccured(CanEvent event);
protected:
	void stopRxThread(void);
	void startRxThread(void);
	void stopTxThread(void);
	void startTxThread(void);
	QThread *rxThread;
	QThread *txThread;
};

#endif // CAN_H
