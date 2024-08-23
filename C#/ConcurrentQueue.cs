namespace ConcurrentCollections
{
    internal class ConcurrentQueue<T>
    {
        private class Node
        {
            public T Value;
            public Node Next;

            public Node(T value)
            {
                this.Value = value;
            }
        }

        private volatile Node Head;

        private volatile Node Tail;


        public void Push(T value) { }

        public bool TryPop(out T value) { }

        public bool TryPeek(out T value) { }
    }
}
