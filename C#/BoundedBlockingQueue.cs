namespace ConcurrentCollections
{
    internal class BoundedBlockingQueue<T>
    {
        private readonly int capacity;

        private readonly T[] buffer;

        private int writeIdx;
        private int readIdx;
        private int count;

        private readonly object lockObj = new object();


        public BoundedBlockingQueue(int capacity)
        {
            this.capacity = capacity;
            this.writeIdx = 0;
            this.readIdx = 0;
            this.buffer = new T[capacity];
            this.count = 0;
        }

        public void Enqueue(T item)
        {
            lock (lockObj)
            {
                while (count == capacity)
                {
                    Monitor.Wait(lockObj);
                }

                count++;

                buffer[writeIdx] = item;

                writeIdx = (writeIdx + 1) % capacity;

                if (count - 1 == 0)
                {
                    Monitor.PulseAll(lockObj);
                }
            }
        }

        public T Dequeue()
        {
            lock (lockObj)
            {
                while (count == 0)
                {
                    Monitor.Wait(lockObj);
                }

                count--;

                if (count == 0)
                {
                    Monitor.PulseAll(lockObj);
                }

                T val = buffer[readIdx];

                readIdx = (readIdx + 1) % capacity;

                return val;
            }
        }
    }
}
